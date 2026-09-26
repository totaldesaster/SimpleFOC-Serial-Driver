#include "ServoAxis.h"

ServoAxis::ServoAxis  (BLDCMotor &motor, MagneticSensorI2C &encoder, BLDCDriver3PWM &driver, TwoWire &wire)
                      : motor(motor), encoder(encoder), driver(driver), wire(wire) {}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Controller tuning function
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ServoAxis::tuneFilter(float tf, float ts) {
  motor.LPF_velocity.Tf = tf;                                         // Filter time constant
  motor.LPF_velocity.Ts = ts;                                         // Sampling time
}

void ServoAxis::tuneVelController(float p, float i, float d, float ramp) {
  motor.PID_velocity.P = p;                                           // Proportional gain
  motor.PID_velocity.I = i;                                           // Integral gain
  motor.PID_velocity.D = d;                                           // Derivative gain
  motor.PID_velocity.output_ramp = ramp;                              // Voltage ramp rate ~ jerk limit
}

void ServoAxis::tunePosController(float p, float i, float d, float ramp) {
  motor.P_angle.P = p;                                                // Proportional gain
  motor.P_angle.I = i;                                                // Integral gain
  motor.P_angle.D = d;                                                // Derivative gain
  motor.P_angle.output_ramp = ramp;                                   // Velocity ramp rate ~ acceleration limit
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Telemetry functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ServoAxis::updateTelemetry() {                                   // Internally update the motor's telemetry data
  float vq = motor.voltage.q;                                         // Motor Q (torque producing) voltage
  float vd = motor.voltage.d;                                         // Motor D (field producing) voltage
  telemetry.amps = motor.current.q;                                   // Motor Q current = total current (no lag compensation)
  telemetry.volts = sqrt(vd * vd + vq * vq);                          // Total commanded voltate according to Pytagoras theorem
  telemetry.watts = telemetry.amps * telemetry.volts;                 // Power according to Ohm's law
  telemetry.vel = motor.shaft_velocity;                               // Rotational velocity (rad/s)
  telemetry.angle = motor.shaft_angle;                                // Angle (rad)
}

String ServoAxis::getKinematicData() {                                // Get the motor's telemetry data regarding motion control
  String result;                                                      // String to store the result
  result += String(telemetry.vel, 2);                                 // Get velocity
  result += " ";                                                      // Add a space
  result += String(telemetry.angle, 2);                               // Get position
  return result;                                                      // Return the string
}

String ServoAxis::getTelemetryData() {                                // Get the motor's telemetry data regarding electrical power
  String result;                                                      // String to store the result
  result += String(telemetry.amps, 2);                                // Get commanded amperage
  result += "A ";                                                     // Add unit and space
  result += String(telemetry.volts, 2);                               // Get commanded voltage
  result += "V ";                                                     // Add unit and space
  result += String(telemetry.watts, 2);                               // Get calculated power
  result += "W";                                                      // Add unit
  return result;                                                      // Return the string
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ServoAxis::setTrkTarget(float trk) {                             // Torque command
  motor.updateMotionControlType(MotionControlType::torque);           // Switch to torque control mode
  motor.target = trk;                                                 // Update torque target
}

void ServoAxis::setVelTarget(float vel) {                             // Velocity command
  motor.updateMotionControlType(MotionControlType::velocity);         // Switch to velocity control mode
  motor.target = vel;                                                 // Update velocity target
}

void ServoAxis::setPosTarget(float pos) {                             // Position (no cascade) command
  motor.updateMotionControlType(MotionControlType::angle_nocascade);  // Switch to direct angle control mode
  motor.target = pos;                                                 // Update angle target
}

void ServoAxis::setCascadeTargets(float pos, float vel, float trk) {  // Position (cascaded) command
  motor.updateMotionControlType(MotionControlType::angle);            // Switch to cascaded angle control mode
  motor.target = pos;                                                 // Update angle target
  motor.feed_forward_velocity = vel;                                  // Set velocity feed-forward
  motor.feed_forward_current.q = trk;                                 // Set current (torque) feed-forward
}

void ServoAxis::setLimits(float current, float velocity) {
  motor.updateCurrentLimit(current);                                  // Motor current (torque) limit
  motor.updateVelocityLimit(velocity);                                // Motor velocity limit
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup & Loop functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ServoAxis::setup() {
  wire.begin();                                                       // Initialize I2C
  wire.setClock(400000);                                              // Increase I2C speed
  encoder.init(&wire);                                                // Initialize Encoder
  motor.linkDriver(&driver);                                          // Link driver to motor
  motor.linkSensor(&encoder);                                         // Link encoder to motor
  motor.controller = MotionControlType::torque;                     
  motor.torque_controller = TorqueControlType::estimated_current;     // Control torque by estimated motor current
  motor.init();                                                       // Init motor
  motor.initFOC();                                                    // Init control
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ServoAxis::loop() {                                              // To be called in loop as fast as possible
  motor.loopFOC();                                                    // Run torque control
  motor.move();                                                       // Run motion control
}