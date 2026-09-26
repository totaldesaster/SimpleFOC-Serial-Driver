#include "MotorBoard_DRV8323.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DRV8323S SPI driver definition and setup
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DRV8323S_SPI::DRV8323S_SPI(uint8_t sclk,uint8_t sdi,uint8_t sdo,uint8_t cs) : _sclk(sclk),_sdi(sdi),_sdo(sdo),_cs(cs) { }

void DRV8323S_SPI::begin() {                                                  // SPI driver setup function
  pinMode(_sclk, OUTPUT);                                                     // Pin config: DRV8323 clock in, MCU clock out
  pinMode(_sdi, OUTPUT);                                                      // Pin config: DRV8323 data in, MCU data out
  pinMode(_sdo, INPUT_PULLUP);                                                // Pin config: DRV8323 data out, MCU data in
  pinMode(_cs, OUTPUT);                                                       // Pin config: Chip select (active low)
  digitalWrite(_sclk, LOW);                                                   // Pin state: low, DRV8323 requires SCLK LOW when nSCS changes
  digitalWrite(_sdi, LOW);                                                    // Pin state: low, indicates zero
  digitalWrite(_cs, HIGH);                                                    // Pin state: high, deselect chip (idle)
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// High-Level SPI Control
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint16_t DRV8323S_SPI::writeRegister(uint8_t reg, uint16_t data) {            // Write a register on the device
  uint16_t tx = ((uint16_t)(reg & 0x0F) << 11) | (data & 0x07FF);             // B15 = 0 (write), B14:11 = register address, B10:0 = register data
  select();                                                                   // Select chip (and ensure it's enabled)
  uint16_t rx = transfer16(tx);                                               // Send data, register response
  deselect();                                                                 // Deselect chip (and disable if it was previously disabled)
  return rx;                                                                  // Return received data
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint16_t DRV8323S_SPI::readRegister(uint8_t reg) {                            // Read a register on the device
  uint16_t tx = 0x8000 | ((uint16_t)(reg & 0x0F) << 11);                      // B15 = 1 (read), B14:11  = register address, B10:0 = don't care (device returns data)
  select();                                                                   // Select chip (and ensure it's enabled)
  uint16_t rx = transfer16(tx);                                               // Send data, register response
  deselect();                                                                 // Deselect chip (and disable if it was previously disabled)
  return rx;                                                                  // Return received data
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Low-Level SPI Control
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DRV8323S_SPI::select() {                                                 // Function to select the chip
  digitalWrite(_sclk, LOW);                                                   // SCLK must already be LOW.
  digitalWrite(_cs, LOW);                                                     // nSCS setup time >= 50 ns.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DRV8323S_SPI::deselect() {                                               // Function to deselect the chip
  digitalWrite(_sclk, LOW);                                                   // SCLK must be LOW when nSCS goes HIGH.
  digitalWrite(_cs, HIGH);                                                    // Datasheet requires nSCS HIGH for >= 400 ns between SPI words.      
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint16_t DRV8323S_SPI::transfer16(uint16_t tx) {                              // 16-Bit bit-banged SPI transfer function
  uint16_t rx = 0;                                                            // Received data
  for (int8_t bit = 15; bit >= 0; bit--) {                                    // Loop through the 16 bits:
    digitalWrite(_sdi, (tx >> bit) & 0x01);                                   // - Set SDI while SCLK is LOW. The DRV8323 captures SDI on the FALLING edge.
    digitalWrite(_sclk, HIGH);                                                // - Rising edge: DRV8323 propagates SDO here.
    if (digitalRead(_sdo)) { rx |= (uint16_t)1 << bit; }                      // - SDO is now valid. Capture data.
    digitalWrite(_sclk, LOW);                                                 // - Falling edge: DRV8323 captures SDI here.
  } return rx;                                                                // Return the received data.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Motor Board definition and setup
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MotorBoard::MotorBoard() : drv0(PINCONF_DRV0),
                           drv1(PINCONF_DRV1),
                           cs0(PINCONF_CS0),
                           cs1(PINCONF_CS1),
                           drvspi0(PINCONF_SPI0),
                           drvspi1(PINCONF_SPI1)
{}

void MotorBoard::begin(float supply, float limit) {                           // Motor board initialisation function
  drv0.init();                                                                // SimpleFOC: Driver 0
  drv1.init();                                                                // SimpleFOC: Driver 1
  cs0.init();                                                                 // SimpleFOC: Current Sense 0
  cs1.init();                                                                 // SimpleFOC: Current Sense 1
  drv0.voltage_power_supply = supply;                                         // SimpleFOC: Voltage, Power Supply
  drv1.voltage_power_supply = supply;                                         // SimpleFOC: Voltage, Power Supply
  drv0.voltage_limit = limit;                                                 // SimpleFOC: Voltage, Driver Hard Limit
  drv1.voltage_limit = limit;                                                 // SimpleFOC: Voltage, Driver Hard Limit
  pinMode(SPI_CS_ACCEL, INPUT_PULLUP);                                        // Pin: disable accelerometer SPI
  pinMode(DRV_ENA0, OUTPUT);                                                  // Pin: drive 0 enable
  pinMode(DRV_ENA1, OUTPUT);                                                  // Pin: drive 1 enable
  pinMode(DRV_CAL, OUTPUT);                                                   // Pin: drive offset calibration
  drvspi0.begin();                                                            // SPI: configure driver for drv0
  drvspi1.begin();                                                            // SPI: configure driver for drv1
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Brake Chopper control
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MotorBoard::setBrakeDuty(uint8_t duty, float supplyVolts) {              // Set normalized brake power (255 = max continuous power)
    float targetPower = (static_cast<float>(duty) / 255.0f) * MAX_POWER;      // Target power, proportional to requested duty
    float pwmFraction = targetPower * BRAKE_R / (supplyVolts * supplyVolts);  // Power = PWM * U^2 / R
    float pwm = pwmFraction * 255.0f;                                         // Find target PWM value
    pwm = constrain(pwm, 0.0f, 255.0f);                                       // Constrain to 0...255 range
    analogWrite(BRAKECHOP, static_cast<uint8_t>(pwm + 0.5f));                 // Write PWM
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Auxiliary Motor Driver Functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MotorBoard::calibrateCSA() {                                             // Current Sense Amplifier calibration function
  pinMode(DRV_CAL, OUTPUT);                                                   // Set CAL pin to output
  digitalWrite(DRV_CAL, HIGH);                                                // Send calibration command
  delay(1);                                                                   // Give the driver time to perform calibration
  digitalWrite(DRV_CAL, LOW);                                                 // Reset calibration command
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MotorBoard::enableDrives() {                                             // Motor drivers enable command
  digitalWrite(DRV_ENA1, HIGH);                                               // Enable Drive 1
  digitalWrite(DRV_ENA0, HIGH);                                               // Enable Drive 0
  delay(1);                                                                   // Allow drives to wake up (tWAKE = 1ms)
  drvspi0.writeRegister(0x02, 0b00000100000);                                 // SPI: Driver Control Register: set to 3x PWM Mode
  drvspi0.writeRegister(0x05, 0b00101010100);                                 // SPI: Overcurrent Protection Register: set reduced VDS overcurrent treshold
  drvspi0.writeRegister(0x06, 0b01001000000);                                 // SPI: Current Sense Amplifier Register: set reduced gain and overcurrent treshold
  drvspi1.writeRegister(0x02, 0b00000100000);                                 // SPI: Driver Control Register: set to 3x PWM Mode
  drvspi1.writeRegister(0x05, 0b00101010100);                                 // SPI: Overcurrent Protection Register: set reduced VDS overcurrent treshold
  drvspi1.writeRegister(0x06, 0b01001000000);                                 // SPI: Current Sense Amplifier Register: set reduced gain and overcurrent treshold
  calibrateCSA();                                                             // Calibrate current sense amplifiers
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MotorBoard::disableDrives() {                                            // Motor drivers disable command
  digitalWrite(DRV_ENA1, LOW);                                                // Disable Drive 1
  digitalWrite(DRV_ENA0, LOW);                                                // Disable Drive 0
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MotorBoard::clearFaultsNormal() {                                        // Motor Driver clear fault and resume normal operation command
  drvspi0.writeRegister(0x02, 0b00000100001);                                 // SPI: Driver Control Register: set clear fault bit
  drvspi1.writeRegister(0x02, 0b00000100001);                                 // SPI: Driver Control Register: set clear fault bit
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MotorBoard::clearFaultsCoast() {                                         // Motor Driver clear fault and coast command
  drvspi0.writeRegister(0x02, 0b00000100101);                                 // SPI: Driver Control Register: set clear fault bit
  drvspi1.writeRegister(0x02, 0b00000100101);                                 // SPI: Driver Control Register: set clear fault bit
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint64_t MotorBoard::getFaults() {                                            // Get faults from drivers. 0 Indicates no fault.
  uint16_t drv0_fault1; uint16_t drv0_fault2;                                 // Two fault registers on Driver 0
  uint16_t drv1_fault1; uint16_t drv1_fault2;                                 // Two fault registers on Driver 1
  if ( digitalRead(DRV_ENA0) && !digitalRead(DRV_FAULT0)) {                   // If driver 0 is enabled and reporting a fault:
    drv0_fault1 = drvspi0.readRegister(0x00);                                 // - Read fault register 1
    drv0_fault2 = drvspi0.readRegister(0x01); }                               // - Read fault register 2
  if ( digitalRead(DRV_ENA1) && !digitalRead(DRV_FAULT1)) {                   // If driver 1 is enabled and reporting a fault:
    drv1_fault1 = drvspi1.readRegister(0x00);                                 // - Read fault register 1
    drv1_fault2 = drvspi1.readRegister(0x01); }                               // - Read fault register 2
  return (static_cast<uint64_t>(drv0_fault1) << 48) |                         // Assemble four fault registers to 64-Bit data packet
         (static_cast<uint64_t>(drv0_fault2) << 32) |
         (static_cast<uint64_t>(drv1_fault1) << 16) |
         static_cast<uint64_t>(drv1_fault2);
}