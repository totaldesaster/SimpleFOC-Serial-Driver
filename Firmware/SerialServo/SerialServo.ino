#include "ControlBoard_RP2354.h"
#include "MotorBoard_DRV8323.h"
#include "SerialDriveMotor.h"

// Configuration & tuning for DFRobot FIT1035
ControlBoard cb;                                                        // Control Board hardware class
MotorBoard mb;                                                          // Motor Board hardware class
BLDCMotor m0 = BLDCMotor(7, 10.7, 110, 0.0053);
BLDCMotor m1 = BLDCMotor(7, 10.7, 110, 0.0053);
MagneticSensorI2C enc0 = MagneticSensorI2C( AS5600_I2C );
MagneticSensorI2C enc1 = MagneticSensorI2C( AS5600_I2C );
SerialDriveMotor axis0 (m0, enc0, mb.drv0, Wire);
SerialDriveMotor axis1 (m1, enc1, mb.drv1, Wire1);
repeating_timer_t focTimer;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTOS Tasks
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool focTimerCB(repeating_timer_t *timer) {                             // Timer-Controlled Hard Realtime motion control loop
  axis0.loop();                                                         // Run axis loop
  return true;                                                          // Return "keep repeating" to controlling timer
}

void processCommand(Stream& ser) {                                      // Command processing accepts serial stream to allow USB and UART
  String command;                                                       // String to store the command substring
  String response;                                                      // String to store the command response
  String received = ser.readStringUntil('\n');                          // Read data from serial
  received.trim();                                                      // Remove termination characters
  if (received.length() == 0) { return; }                               // Abort if length is 0 (e.g. only newline received)
  if (received.indexOf(' ') == -1) { command = received; }              // If there is no space, the entire string is the command
  else { command = received.substring(0, received.indexOf(' ')); }      // If there is a space, first substring is the command
  command.trim();                                                       // Trim command string
  command.toUpperCase();                                                // Convert command string to uppercase for robustness

  if (command == "ENABLE") { mb.enableDrives(); }                       // Motor Driver Enable command

  if (command == "DISABLE") { mb.disableDrives(); }                     // Motor Driver Disable (coast) command

  if (command == "T") {                                                 // Torque (current) command
    int p1 = received.indexOf(' ');                                     // Find position of first space
    int p2 = received.indexOf(' ', p1 + 1);                             // Find position of second space
    int axis = received.substring(p1 + 1, p2).toInt();                  // Substring 1 is axis number
    float current = received.substring(p2 + 1).toFloat();               // Substring 2 is torque target
    if (axis == 0) { axis0.setTrkTarget(current); }                     // Apply command to the relevant axis
    if (axis == 1) { axis0.setTrkTarget(current); }
  }

  if (command == "V") {                                                 // Velocity command
    int p1 = received.indexOf(' ');                                     // Find position of first space
    int p2 = received.indexOf(' ', p1 + 1);                             // Find position of second space
    int axis = received.substring(p1 + 1, p2).toInt();                  // Substring 1 is axis number
    float velocity = received.substring(p2 + 1).toFloat();              // Substring 2 is velocity target
    if (axis == 0) { axis0.setVelTarget(velocity); }                    // Apply command to the relevant axis
    if (axis == 1) { axis0.setVelTarget(velocity); }
  }

  if (command == "P") {                                                 // Position command
    int p1 = received.indexOf(' ');                                     // Find position of first space
    int p2 = received.indexOf(' ', p1 + 1);                             // Find position of second space
    int axis = received.substring(p1 + 1, p2).toInt();                  // Substring 1 is axis number
    float position = received.substring(p2 + 1).toFloat();              // Substring 2 is position target
    if (axis == 0) { axis0.setPosTarget(position); }                    // Apply command to the relevant axis
    if (axis == 1) { axis0.setPosTarget(position); }
  }

  if (command == "C") {                                                 // Cascaded position command
    int p1 = received.indexOf(' ');                                     // Find position of first space
    int p2 = received.indexOf(' ', p1 + 1);                             // Find position of second space
    int p3 = received.indexOf(' ', p2 + 1);                             // Find position of third space
    int p4 = received.indexOf(' ', p3 + 1);                             // Find position of fourth space
    int axis = received.substring(p1 + 1, p2).toInt();                  // Substring 1 is axis number
    float position = received.substring(p2 + 1).toFloat();              // Substring 2 is position target
    float vel_ff = received.substring(p3 + 1).toFloat();                // Substring 3 is velocity feed-forward
    float trk_ff = received.substring(p4 + 1).toFloat();                // Substring 4 is torque feed-forward
    if (axis == 0) { axis0.setCascadeTargets(position,vel_ff,trk_ff); } // Apply command to the relevant axis
    if (axis == 1) { axis0.setCascadeTargets(position,vel_ff,trk_ff); }
  }

    if (command == "B") {                                               // Feedback request (Kinematics)
    int p1 = received.indexOf(' ');                                     // Find position of first space
    uint8_t power = received.substring(p1 + 1).toInt();                 // Substring 1 is braking power
    mb.setBrakeDuty(power, cb.readVoltage());                           // Apply braking power
  }

  if (command == "F") {                                                 // Feedback request (Kinematics)
    int p1 = received.indexOf(' ');                                     // Find position of first space
    int axis = received.substring(p1 + 1).toInt();                      // Substring 1 is axis number
    if (axis == 0) { response = axis0.getKinematicData(); }             // Apply command to the relevant axis
    if (axis == 1) { response = axis1.getKinematicData(); }
  }

  if (command == "FMP") {                                               // Feedback Request (Motor Power)
    int p1 = received.indexOf(' ');                                     // Find position of first space
    int axis = received.substring(p1 + 1).toInt();                      // Substring 1 is axis number
    if (axis == 0) { response = axis0.getTelemetryData(); }             // Apply command to the relevant axis
    if (axis == 1) { response = axis0.getTelemetryData(); }
  }

  if (command == "FBP") {                                               // Feedback Request (Board Power)
    response += String(cb.readCurrent(), 2);                            // Get amperage
    response += "A ";                                                   // Add unit and space
    response += String(cb.readVoltage(), 2);                            // Get voltage
    response += "V ";                                                   // Add unit and space
    response += String(cb.readVoltage()*cb.readCurrent(), 2);           // Calculate power
    response += "W";                                                    // Add unit
  }

  if (response.length() > 0) { ser.println(response); }                 // Print the response if there is one
}

void comRxTask(void *parameter) {                                       // COM RX: receive commands and call processing
  for (;;) {                                                            // Let Task run forever
    vTaskDelay(pdMS_TO_TICKS(10));                                      // Short delay to avoid excessive CPU load
    bool usbAvail = Serial.available();                                 // Check if data is available on USB Serial
    bool comAvail = cb.comchipUART.available();                         // Check if data is available on RS422/485
    bool ttlAvail = cb.ttlUART.available();                             // Check if data is available on TTL UART
    cb.setComLED(usbAvail || comAvail || ttlAvail);                     // Set LED to indicate data available
    if ( usbAvail ) processCommand(Serial);                             // If data is available on USB: pass SerialUSB object
    if ( comAvail ) processCommand(cb.comchipUART);                     // If data is available on 485: pass SerialUART object
    if ( ttlAvail ) processCommand(cb.ttlUART);                         // If data is available on TTL: pass SerialUART object
  }
}

void dataUpdateTask(void *parameter) {                                  // Data update: slower functions
  TickType_t lastWakeTime = xTaskGetTickCount();                        // last time the task was executed
  for (;;) {                                                            // Let Task run forever
    axis0.updateTelemetry();                                            // Update telemetry data for axis 0
    axis1.updateTelemetry();                                            // Update telemetry data for axis 1
    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(100));                 // Delay task until 100ms after last start
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Flow Control
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {  
  cb.begin();                                                           // Initialize control board
  mb.begin(cb.readVoltage(),cb.readVoltage());                          // Initialize motor board
  mb.enableDrives();                                                    // Start up drivers for configuration
  axis0.setup();                                                        // Run servo axis setup
  axis0.setLimits(0.5, 20);                                             // Servo axis: 0.5A and 20rad/s limit
  axis0.tuneFilter(0.005, NOT_SET);                                     // Servo axis: 5ms filter time, no fixed sample time
  axis0.tuneVelController(0.01,1,0,1000);                               // Servo axis: PID values and ramp rate for velocity controller
  axis0.tunePosController(20, 0, 0, NOT_SET);                           // Servo axis: PID values and ramp rate for position controller
  mb.disableDrives();                                                   // Disable drivers until called to run
  add_repeating_timer_us(-1000, focTimerCB, nullptr, &focTimer);        // Initialize task: FOC (timer controlled)
  xTaskCreate(comRxTask,"COM RX",2048,nullptr,2,nullptr);               // RTOS Receive & Forward Task
  xTaskCreate(dataUpdateTask,"Telemetry",2048,nullptr,3,nullptr);       // Low Speed Updates Task
}

void loop() {
  delay(1000);                                                          // Loop does nothing, tasks are controlled by FreeRTOS
}