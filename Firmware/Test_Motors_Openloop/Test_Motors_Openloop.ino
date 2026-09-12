#include "ControlBoard_RP2354.h"
#include "MotorBoard_DRV8323.h"

ControlBoard cb;
MotorBoard mb;
BLDCMotor m0 = BLDCMotor(7);
BLDCMotor m1 = BLDCMotor(7);

void setup() {
  cb.begin();                                                           // Initialize control board
  mb.begin(24,24);                                                      // Initialize motor board (supply & limit voltages)
  m0.linkDriver(&mb.drv0);                                              // Link motors and drivers
  m1.linkDriver(&mb.drv1);
  m0.voltage_limit = 6;                                                 // Limit voltage sent to the motor
  m1.voltage_limit = 6;                   
  m0.controller = MotionControlType::velocity_openloop;                 // Assign open loop controllers to motors
  m1.controller = MotionControlType::velocity_openloop; 
  m0.init();                                                            // Init Motors
  m1.init();                              
}

void loop() {
  m0.move(20);                                                          // Move motor 0 at 20 rad/s
  m0.loopFOC();                                                         // Execute FOC loop for motor 0
  m1.move(20);                                                          // Move motor 1 at 20 rad/s
  m1.loopFOC();                                                         // Execute FOC loop for motor 1
}