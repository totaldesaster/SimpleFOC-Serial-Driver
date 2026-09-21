#include "ControlBoard_RP2354.h"
#include "MotorBoard_DRV8323.h"

// Configuration & tuning for DFRobot FIT1035
ControlBoard cb;
MotorBoard mb;
BLDCMotor m0 = BLDCMotor(7);
MagneticSensorI2C enc0 = MagneticSensorI2C( AS5600_I2C );

void setup() {
  cb.begin();                                                           // Initialize control board
  mb.begin(12,12);                                                      // Initialize motor board
  mb.drv0.init();                                                       // Initialize motor board driver
  enc0.init();                                                          // Initialize Encoder
  m0.linkDriver(&mb.drv0);                                              // Link driver to motor
  m0.linkSensor(&enc0);                                                 // Link encoder to motor
  m0.controller = MotionControlType::velocity;                          // Link controller to motor
  m0.LPF_velocity.Tf = 0.005;                                           // Controller Tuning: filter time constant
  m0.PID_velocity.P = 0.2f;                                             // Controller Tuning: proportional gain
  m0.PID_velocity.I = 10;                                               // Controller Tuning: integral gain
  m0.PID_velocity.D = 0;                                                // Controller Tuning: derivative gain
  m0.PID_velocity.output_ramp = 1000;                                   // Controller Tuning: jerk limit
  mb.enableDrives();                                                    // Start up drivers
  m0.init();                                                            // Init motor
  m0.initFOC();                                                         // Init control
}

void loop() {
  m0.move(50);
  m0.loopFOC();
}