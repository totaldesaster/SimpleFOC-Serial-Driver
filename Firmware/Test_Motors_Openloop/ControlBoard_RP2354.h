#include <Arduino.h>

// Configurations of serial chip
#define BUSIDLE           0,0,0,0                           // No Tx/Rx
#define TERM              0,0,0,1                           // No Tx/Rx, with 120 Ohm Termination
#define HD_TXRX           1,1,1,0                           // Half Duplex Tx & Rx
#define HD_RXONLY         0,1,1,0                           // Half Duplex Rx Only
#define HD_TXRX_TERM      1,1,1,1                           // Half Duplex Tx & Rx with 120 Ohm Termination
#define HD_RXONLY_TERM    0,1,1,1                           // Half Duplex Rx Only with 120 Ohm Termination
#define FD_TXRX           1,1,0,0                           // Full Duplex Tx & Rx
#define FD_RXONLY         0,1,0,0                           // Full Duplex Rx Only
#define FD_TXRX_TERM      1,1,0,1                           // Full Duplex Tx & Rx with 120 Ohm Termination
#define FD_RXONLY_TERM    0,1,0,1                           // Full Duplex Rx Only with 120 Ohm Termination

// Class for control board hardware
class ControlBoard {
  public:
    ControlBoard();                                         // Constructor
    void begin();                                           // Hardware initialisation
    float readCurrent();                                    // Analog Input: read supply current
    float readVoltage();                                    // Analog Input: read supply voltage
    float readTemperature();                                // Analog Input: read board thermistor
    uint8_t readConfsw();                                   // Digital Inputs: read config select pins
    void setComLED(int state);                              // Digital Output: "COM" LED
    void setDrvLED(int state);                              // Digital Output: "DRV" LED
    void set5VEna(int state);                               // Digital Output: 5V Regulator Enable
    SerialUART& ttlUART;                                    // Serial Port: TTL UART on screw terminals
    SerialUART& comchipUART;                                // Serial Port: Differential UART on THVD1424
    void comchipConfig(int txen,int rxen,int term,int hf);  // Serial Port: Set configuration for THVD1424
    static constexpr int ENCODER_0A   = 2;                  // Enc0 Phase A / I2C1 SDA
    static constexpr int ENCODER_0B   = 3;                  // Enc0 Phase B / I2C1 SCL
    static constexpr int ENCODER_0I   = 7;                  // Enc0 Phase I
    static constexpr int ENCODER_1A   = 6;                  // Enc1 Phase A / I2C0 SDA
    static constexpr int ENCODER_1B   = 5;                  // Enc1 Phase B / I2C0 SCL
    static constexpr int ENCODER_1I   = 4;                  // Enc1 Phase I

  private:
    static constexpr int EXTCOM_TX    = 8;                  // UART0 TTL Tx
    static constexpr int EXTCOM_RX    = 9;                  // UART0 TTL Rx
    static constexpr int COMBUS_TX    = 0;                  // UART1 THVD1424 Tx
    static constexpr int COMBUS_RX    = 1;                  // UART1 THVD1424 Rx
    static constexpr int COMBUS_TXEN  = 11;                 // THVD1424 Tx Enable
    static constexpr int COMBUS_RXEN  = 12;                 // THVD1424 Rx Enable
    static constexpr int COMBUS_TERM  = 10;                 // THVD1424 Termination Resistor Enable
    static constexpr int COMBUS_HF    = 13;                 // THVD1424 Full / Half Duplex Switch
    static constexpr int BOARDTEMP    = 40;                 // Board Temperature Thermistor
    static constexpr int INPUT_CURR   = 46;                 // ACS711 Hall Current Sensor
    static constexpr int INPUT_VOLT   = 47;                 // Voltage Divider
    static constexpr int LED_DRV      = 23;                 // "DRV" LED
    static constexpr int LED_COM      = 25;                 // "COM" LED
    static constexpr int VREG_ENA     = 27;                 // 5 Volt Regulator Enable
    static constexpr int CONFSW[]     = {28,29,30,31};      // Config pins
};