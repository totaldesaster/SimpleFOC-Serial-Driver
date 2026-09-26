#include "ControlBoard_RP2354.h"
#include "MotorBoard_DRV8323.h"

ControlBoard cb;
MotorBoard mb;
BLDCMotor m0 = BLDCMotor(7);
BLDCMotor m1 = BLDCMotor(7);


void setup() {
  cb.begin();                                                           // Initialize control board
  mb.begin(24,24);                                                      // Initialize motor board
  m0.linkDriver(&mb.drv0);
  m1.linkDriver(&mb.drv1);
  m0.voltage_limit = 24;
  m1.voltage_limit = 24;
  m0.controller = MotionControlType::velocity_openloop;
  m1.controller = MotionControlType::velocity_openloop;
  m0.init();
  m1.init();
  mb.enableDrives();
}

void loop() {
  m0.move(100);
  m0.loopFOC();
  m1.move(100);
  m1.loopFOC();
}