#ifndef SERIAL_DRIVE_MOTOR_H
#define SERIAL_DRIVE_MOTOR_H

#include <SimpleFOC.h>



class ServoAxis {
  public:
    ServoAxis(BLDCMotor &motor, MagneticSensorI2C &encoder, BLDCDriver3PWM &driver, TwoWire &wire);
    void tuneFilter(float tf, float ts);
    void tuneVelController(float p, float i, float d, float ramp);
    void tunePosController(float p, float i, float d, float ramp);
    void setMode(MotionControlType type);
    void setLimits(float current, float velocity);
    void updateTelemetry();
    void loop();
    void setup();
    String getKinematicData();
    String getTelemetryData();

    void setTrkTarget(float trk);
    void setVelTarget(float vel);
    void setOpenloopVel(float vel);
    void setPosTarget(float pos);
    void setCascadeTargets(float pos, float vel, float trk);

    struct {
      float volts;
      float amps;
      float watts;
      float vel;
      float angle;
    } telemetry;

  private:
    BLDCMotor &motor;
    MagneticSensorI2C &encoder;
    BLDCDriver3PWM &driver;
    TwoWire &wire;
    float posTarget;
    float velTarget;
    float trkTarget;
};

#endif