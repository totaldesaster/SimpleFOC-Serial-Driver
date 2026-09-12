#include "ControlBoard_RP2354.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ControlBoard_RP2354.cpp
// Functions for hardware on the main control board
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup: assign two UARTs to TTL & RS-485 in the constructor, set up pins and serial ports in begin()
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ControlBoard::ControlBoard() :  ttlUART(Serial2),                               // Assign UART1 to TTL UART
                                comchipUART(Serial1)                            // Assign UART0 to RS422/RS485 UART
{ }                                                                             // Constructor empty

void ControlBoard::begin() {                                                    // Board initializer functions
  for (int i = 0; i < 4; i++) { pinMode(CONFSW[i], INPUT); }                    // Configure conf sw inputs
  pinMode(LED_DRV, OUTPUT);                                                     // Configure drive LED control
  pinMode(LED_COM, OUTPUT);                                                     // Configure communication LED control
  pinMode(VREG_ENA, OUTPUT);                                                    // Configure voltage regulator control
  pinMode(COMBUS_TXEN, OUTPUT);                                                 // Configure transceiver Tx Enable
  pinMode(COMBUS_RXEN, OUTPUT);                                                 // Configure transceiver Rx Enable
  pinMode(COMBUS_TERM, OUTPUT);                                                 // Configure transceiver Termination Resistor
  pinMode(COMBUS_HF, OUTPUT);                                                   // Configure transceiver Half/Full Duplex Switch
  comchipConfig(BUSIDLE);                                                       // Initial config: Rx/Tx disabled
  analogReadResolution(12);                                                     // Configure ADC
  ttlUART.setRX(EXTCOM_RX);                                                     // Configure TTL UART Pins
  ttlUART.setTX(EXTCOM_TX);                                                     // Configure TTL UART Pins
  ttlUART.begin(115200);                                                        // Begin TTL UART
  comchipUART.setRX(COMBUS_RX);                                                 // Configure RS422/RS485 UART Pins
  comchipUART.setTX(COMBUS_TX);                                                 // Configure RS422/RS485 UART Pins
  comchipUART.begin(10000000);                                                   // Begin RS422/RS485 UART
  Serial.begin(115200);                                                         // Begin USB Serial
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sets the configuration of RS-485/RS-422 Transceiver Chip
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ControlBoard::comchipConfig(int txen, int rxen, int term, int halfd) {
  digitalWrite(COMBUS_TXEN, txen);                                              // Transmitter enable
  digitalWrite(COMBUS_RXEN, 1 - rxen);                                          // Receiver enable (inverted on hardware)
  digitalWrite(COMBUS_TERM, term);                                              // Termination resistor enable
  digitalWrite(COMBUS_HF, halfd);                                               // Half-Duplex enable
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read input pins and evaluate them into a usable data
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float ControlBoard::readCurrent() {                                             // Read supply current
  float voltage = analogRead(INPUT_CURR) * 3.3f / 4095.0f;                      // Get ADC voltage
  float current = (voltage - 1.65f) / 0.055f;                                   // Convert voltage to current
  return current;                                                               // Return the calculated current
}

float ControlBoard::readVoltage() {                                             // Read supply voltage
  float adcVoltage = analogRead(INPUT_VOLT) * 3.3f / 4095.0f;                   // Get ADC voltage
  float motorVoltage = adcVoltage * (10000.0f + 470.0f) / 470.0f;               // Convert to voltage above divider
  return motorVoltage;                                                          // Return the calculated voltage
}

float ControlBoard::readTemperature() {                                         // Read board temperature from thermistor
  float voltage = analogRead(BOARDTEMP) * 3.3f / 4095.0f;                       // Get ADC voltage
  float rNtc = 4700.0f * voltage / (3.3f - voltage);                            // Convert voltage to resistance
  float tempK = 1.0f / (1.0f / 298.15f + log(rNtc / 10000.0f) / 3570.0f);       // Convert resistance to Kelvin
  return tempK - 273.15f;                                                       // Convert Kelvin to Celsius
}

uint8_t ControlBoard::readConfsw() {                                            // Read configuration switches
  uint8_t conf = 0;                                                             // Define bit mask storage
  for (int i = 0; i < 4; i++) { if (!digitalRead(CONFSW[i])) { conf|=(1<<i); }} // Loop through pins and set bit mask
  return conf;                                                                  // Return address
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set output pins
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ControlBoard::setComLED(int state) { digitalWrite(LED_COM, state); }       // Set state of the communication LED
void ControlBoard::setDrvLED(int state) { digitalWrite(LED_DRV, state); }       // Set state of the drive LED
void ControlBoard::set5VEna(int state) { digitalWrite(VREG_ENA, state); }       // Set state of the 5v regulator