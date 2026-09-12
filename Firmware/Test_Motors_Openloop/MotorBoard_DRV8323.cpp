#include "MotorBoard_DRV8323.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ControlBoard_RP2354.cpp
// Functions for hardware on the motor driver board
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MotorBoard::MotorBoard() :  drv0(36, 35, 34, 20),                               // 3-PWM Driver on GPIO36 / 35 / 34 (Enable on 20)
                            drv1(39, 38, 37, 18),                               // 3-PWM Driver on GPIO39 / 38 / 37 (Enable on 18)
                            cs0(0.01f, 40.0f, 44, 45),                          // Current sense on GPIO44 & 45 (Phase C virtual) with 0.01 Ohm resistor and 40 gain
                            cs1(0.01f, 40.0f, 41, 42),                          // Current sense on GPIO41 & 42 (Phase C virtual) with 0.01 Ohm resistor and 40 gain
                            spi(SPI1)                                           // SPI bus for diagnostics
{}                                                                              // Constructor is empty

void MotorBoard::begin(float supply, float limit) {                             // Initialisation function
  drv0.init();                                                                  // SimpleFOC: Driver 0
  drv1.init();                                                                  // SimpleFOC: Driver 1
  cs0.init();                                                                   // SimpleFOC: Current Sense 0
  cs1.init();                                                                   // SimpleFOC: Current Sense 1
  pinMode(SPI_SDO, OUTPUT);                                                     // SPI Tx
  pinMode(SPI_SDI, INPUT);                                                      // SPI Rx
  pinMode(SPI_SCK, OUTPUT);                                                     // SPI Clock
  digitalWrite(SPI_SCK, LOW);                                                   // Drive clock low initially
  digitalWrite(SPI_SDO, LOW);                                                   // Drive Tx low initially
  accelSetup();                                                                 // Slave Config: Accelerometer
  drv0.voltage_power_supply = supply;                                           // Voltage, Power Supply
  drv1.voltage_power_supply = supply;                                           // Voltage, Power Supply
  drv0.voltage_limit = limit;                                                   // Voltage, Driver Hard Limit
  drv1.voltage_limit = limit;                                                   // Voltage, Driver Hard Limit
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bit-Banging SPI function because somebody didn't read the datasheet properly ;)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t MotorBoard::spiTransfer(uint8_t tx) {                                   // Transmit one byte and read the returned data
  uint8_t rx = 0;                                                               // Received Data
  for (int i = 7; i >= 0; --i) {                                                // Bit-Banging Loop:
    digitalWrite(SPI_SDO, (tx >> i) & 1);                                       //    1) Set Tx signal for next transfer
    digitalWrite(SPI_SCK, HIGH);                                                //    2) Rising edge on clock to start sample
    if (digitalRead(SPI_SDI)) rx |= (1 << i);                                   //    3) Read Rx signal
    digitalWrite(SPI_SCK, LOW); }                                               //    4) Falling edge on clock to reset
  return rx;                                                                    // Return the sampled byte
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Accelerometer IC low-level control
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t MotorBoard::readAccelRegister(uint8_t reg) {                            // Read register of the LIS12DW12 Accelerometer Chip
    uint8_t value;                                                              // Variable to store the register value
    digitalWrite(SPI_CS_ACCEL, LOW);                                            // Drive chip select low
    spiTransfer(reg | 0b10000000);                                              // Transfer read command and register address
    value = spiTransfer(0b00000000);                                            // Empty transfer (accelerometer returns data)
    digitalWrite(SPI_CS_ACCEL, HIGH);                                           // Release chip select
    return value;                                                               // Return the received data from the accelerometer
}

void MotorBoard::readAccelRegs(uint8_t reg, uint8_t* buffer, size_t length) {   // Read multiple registers of the LIS12DW12 Accelerometer Chip
    digitalWrite(SPI_CS_ACCEL, LOW);                                            // Drive chip select low
    spiTransfer(reg | 0b10000000);                                              // Transfer read command and start register address
    for (size_t i = 0; i < length; i++) {                                       // For all elements in the length specified:
        buffer[i] = spiTransfer(0b00000000); }                                  // - Transfer empty data, read the return to buffer
}

void MotorBoard::writeAccelRegister(uint8_t reg, uint8_t value) {               // Write register of the LIS12DW12 Accelerometer Chip
    digitalWrite(SPI_CS_ACCEL, LOW);                                            // Drive chip select low
    spiTransfer(reg & 0b01111111);                                              // Transfer write command and register address
    spiTransfer(value);                                                         // Transfer data to be written
    digitalWrite(SPI_CS_ACCEL, HIGH);                                           // Release chip select
}

bool MotorBoard::accelSetup() {                                                 // Configure accelerometer IC
  uint8_t whoAmI = readAccelRegister(REG_WHO_AM_I);                             // Read who am I value to verify chip
  if (whoAmI != WHO_AM_I_VALUE) { return false; }                               // Abort if wrong chip
  writeAccelRegister(REG_CTRL1, 0b01010100);                                    // Config register 1: 100Hz, High Performance Mode
  writeAccelRegister(REG_CTRL2, 0b00001100);                                    // Config register 2: Block Data Update, Auto-Increment
  writeAccelRegister(REG_CTRL6, 0b10000100);                                    // Config register 6: 1/10 cutoff, 2G fullscale, low noise
  return true;                                                                  // Return config OK
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Accelerometer IC high-level control
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t MotorBoard::readTemperature() {                                         // Read temperature from the LIS12DW12 Accelerometer Chip
  int8_t raw = static_cast<int8_t>(readAccelRegister(REG_OUT_T));               // Get temperature register and cast to signed integer
  return 25 + raw;                                                              // Add zero offset (sensor 0 value is at 25C ambient)                            
}

Vector MotorBoard::readAcceleration(){                                          // Read acceleration from the LIS12DW12 Accelerometer Chip
    uint8_t data[6];                                                            // Storage for 6 registers (XYZ)
    readAccelRegs(REG_OUT_X_L, data, 6);                                        // Read Acceleration registers, 6 starting at X Low
    int16_t rawX = (static_cast<uint16_t>(data[1]) << 8) | data[0];             // Get values from sensor, merging two registers to 16 bit
    int16_t rawY = (static_cast<uint16_t>(data[3]) << 8) | data[2];             // Get values from sensor, merging two registers to 16 bit
    int16_t rawZ = (static_cast<uint16_t>(data[5]) << 8) | data[4];             // Get values from sensor, merging two registers to 16 bit
    rawX >>= 2;                                                                 // Shift two bits to the right (acceleration is 14 bit)
    rawY >>= 2;                                                                 // Shift two bits to the right (acceleration is 14 bit)
    rawZ >>= 2;                                                                 // Shift two bits to the right (acceleration is 14 bit)
    Vector result;                                                              // Struct to store acceleration data
    result.x = static_cast<float>(rawX) * 0.000244f;                            // Calculate acceleration, factoring in sensitivity
    result.y = static_cast<float>(rawY) * 0.000244f;                            // Calculate acceleration, factoring in sensitivity
    result.z = static_cast<float>(rawZ) * 0.000244f;                            // Calculate acceleration, factoring in sensitivity
    return result;                                                              // Return acceleration in G
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gate Driver IC low-level control
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////