#include <Arduino.h>
#include <SimpleFOC.h>

// Class for DRRV8323 SPI driver
class DRV8323S_SPI {
  public:
    DRV8323S_SPI(uint8_t sclk,uint8_t sdi,uint8_t sdo,uint8_t cs);
    void begin();
    uint16_t writeRegister(uint8_t reg, uint16_t data);
    uint16_t readRegister(uint8_t reg);

  private:
    void select();
    void deselect();
    uint16_t transfer16(uint16_t tx);
    uint8_t _sclk;
    uint8_t _sdi;
    uint8_t _sdo;
    uint8_t _cs;
};

// Class for motor board hardware
class MotorBoard {
  public:
    MotorBoard();                                                     // Constructor
    BLDCDriver3PWM drv0;                                              // SimpleFOC Driver Object
    BLDCDriver3PWM drv1;                                              // SimpleFOC Driver Object
    LowsideCurrentSense cs0;                                          // SimpleFOC Current Sense Object
    LowsideCurrentSense cs1;                                          // SimpleFOC Current Sense Object
    DRV8323S_SPI drvspi0;                                             // Serial interface to DRV8323S
    DRV8323S_SPI drvspi1;                                             // Serial interface to DRV8323S
    void begin(float supply, float limit);                            // Initialisation
    void setBrakeDuty(uint8_t duty, float supplyVolts);               // Brake Chopper duty cycle
    void calibrateCSA();                                              // Calibrate Current Sense Amplifiers
    void enableDrives();                                              // Enable motor driver
    void disableDrives();                                             // Disable motor driver
    void clearFaultsNormal();                                         // Clear faults on the motor drivers and put them in normal operation
    void clearFaultsCoast();                                          // Clear faults on the motor drivers and put them in coasting
    uint64_t getFaults();                                             // Get fault data from driver ICs

  private:
    // Pins
    static constexpr int SPI_CS_ACCEL   = 43;                         // SPI Chip Select: Accelerometer
    static constexpr int DRV_ENA0       = 20;                         // Drive 0 Enable
    static constexpr int DRV_ENA1       = 18;                         // Drive 1 Enable
    static constexpr int DRV_CAL        = 33;                         // Drives current sense calibrate command
    static constexpr int DRV_FAULT1     = 17;                         // Drive 1 Fault Indication
    static constexpr int DRV_FAULT0     = 21;                         // Drive 0 Fault Indication
    static constexpr int BRAKECHOP      = 16;                         // Brake Chopper MOSFET
    // Configuration constants
    static constexpr float BRAKE_R      = 2.240f;                     // Brake Resistor resistance (Ohms)
    static constexpr float MAX_POWER    = 5.000f;                     // Brake Power limit (Watts)
};


#define PINCONF_DRV0                    39, 38, 37                    // DRV8323S Drv1 PWM Pins: A, B, C
#define PINCONF_DRV1                    36, 35, 34                    // DRV8323S Drv1 PWM Pins: A, B, C
#define PINCONF_SPI0                    26, 14, 24, 19                // DRV8323S Drv0 SPI Pins: SCLK, MOSI, MISO, CS
#define PINCONF_SPI1                    26, 14, 24, 22                // DRV8323S Drv1 SPI Pins: SCLK, MOSI, MISO, CS

#define PINCONF_CS0                     0.01f, 10.0f, 44, 45          // Current Sense 0: 0.01 Ohm Resistor, 10V/V Gain, Pins 44/45 (Phase C virtual)
#define PINCONF_CS1                     0.01f, 10.0f, 41, 42          // Current Sense 0: 0.01 Ohm Resistor, 10V/V Gain, Pins 41/42 (Phase C virtual)
