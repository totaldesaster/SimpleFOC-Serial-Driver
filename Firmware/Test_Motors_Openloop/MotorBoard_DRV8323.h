#include <Arduino.h>
#include <SimpleFOC.h>

#define SPISETTINGS SPISettings(500000, MSBFIRST, SPI_MODE3)

struct Vector {
  float x;
  float y;
  float z;
};

// Class for motor board hardware
class MotorBoard {
  public:
    BLDCDriver3PWM drv0;                                              // SimpleFOC Driver Object
    BLDCDriver3PWM drv1;                                              // SimpleFOC Driver Object
    LowsideCurrentSense cs0;                                          // SimpleFOC Current Sense Object
    LowsideCurrentSense cs1;                                          // SimpleFOC Current Sense Object
    SPIClass& spi;                                                    // SPI Bus
    MotorBoard();                                                     // Constructor
    void begin(float supply, float limit);                            // Initialisation
    uint8_t readTemperature();                                        // LIS2DW12: Read board temperature
    Vector readAcceleration();                                        // LIS2DW12: Read acceleration vector

  private:
    // Functions
    uint8_t spiTransfer(uint8_t tx);                                  // Bit-Banging SPI
    uint8_t readAccelRegister(uint8_t reg);                           // LIS2DW12: SPI READ on accelerometer chip
    void readAccelRegs(uint8_t reg, uint8_t* buffer, size_t length);  // LIS2DW12: SPI READ burst on accelerometer chip
    void writeAccelRegister(uint8_t reg, uint8_t value);              // LIS2DW12: SPI WRITE on accelerometer chip
    bool accelSetup();                                                // LIS2DW12: Setup
    // Pins
    static constexpr int SPI_SDO       = 14;                          // SPI1 data out
    static constexpr int SPI_SDI       = 24;                          // SPI1 data in
    static constexpr int SPI_SCK       = 26;                          // SPI1 clock
    static constexpr int SPI_CS_DRV0   = 19;                          // SPI Chip Select: Driver 0
    static constexpr int SPI_CS_DRV1   = 22;                          // SPI Chip Select: Driver 1
    static constexpr int SPI_CS_ACCEL  = 43;                          // SPI Chip Select: Accelerometer
    static constexpr int BRAKE_CHOP    = 16;                          // Brake Chopper Mosfet
    static constexpr int DRIVE_FAULT0  = 21;                          // Driver 0 Fault Indication
    static constexpr int DRIVE_FAULT1  = 17;                          // Driver 1 Fault Indication
    static constexpr int DRIVE_CALIB   = 33;                          // Driver Calibration Command
    // Configuration constants
    static constexpr uint8_t REG_OUT_T    = 0x26;                     // LIS2DW12: Temperature Output (8 Bit)
    static constexpr uint8_t REG_WHO_AM_I = 0x0F;                     // LIS2DW12: "Who am I" Value
    static constexpr uint8_t REG_CTRL1    = 0x20;                     // LIS2DW12: Control Register 1 (Data Rate & Mode)
    static constexpr uint8_t REG_CTRL2    = 0x21;                     // LIS2DW12: Control Register 2 (Access Settings)
    static constexpr uint8_t REG_CTRL6    = 0x25;                     // LIS2DW12: Control Register 6 (Bandwidth & Filtering)
    static constexpr uint8_t REG_OUT_X_L  = 0x28;                     // LIS2DW12: Acceleration Output (X Low)
    static constexpr uint8_t REG_OUT_X_H  = 0x29;                     // LIS2DW12: Acceleration Output (X High)
    static constexpr uint8_t REG_OUT_Y_L  = 0x2A;                     // LIS2DW12: Acceleration Output (Y Low)
    static constexpr uint8_t REG_OUT_Y_H  = 0x2B;                     // LIS2DW12: Acceleration Output (Y High)
    static constexpr uint8_t REG_OUT_Z_L  = 0x2C;                     // LIS2DW12: Acceleration Output (Z Low)
    static constexpr uint8_t REG_OUT_Z_H  = 0x2D;                     // LIS2DW12: Acceleration Output (Z High)
    static constexpr uint8_t WHO_AM_I_VALUE = 0x44;                   // LIS2DW12: Expected value when reading who am i
};