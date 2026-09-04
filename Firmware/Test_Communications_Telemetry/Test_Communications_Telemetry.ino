#include "ControlBoard_RP2354.h"
#include <FreeRTOS.h>
#include <task.h>

// Serial Bus & Control Board test sketch
// Transmit telemetry data over RS-422 Full Duplex. Forward received data to serial monitor.
// Designed for two boards talking to eachother, to test onboard sensors and communication.

ControlBoard cb;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void comRxTask(void *parameter) {
  for (;;) {                                                            // Let Task run forever
    cb.setComLED(cb.comchipUART.available());                           // Set LED to indicate data available
    while (cb.comchipUART.available()) {                                // While data is available on comchip UART:
      Serial.write(cb.comchipUART.read()); }                            // - Read from comchip UART, write to USB Serial
    vTaskDelay(pdMS_TO_TICKS(1));                                       // Short delay to avoid excessive CPU load
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void telemetryTask(void *parameter) {
  TickType_t lastWakeTime = xTaskGetTickCount();                        // last time the task was executed
  for (;;) {                                                            // Let Task run forever
    cb.comchipUART.printf("U=%.2f V, I=%.2f A, T=%.1f\r\n",             // Transmit telemetry
      cb.readVoltage(),
      cb.readCurrent(),
      cb.readTemperature());
    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(100));                 // Schedule next wakeup event
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
    cb.begin();                                                         // Initialize controller board
    cb.comchipConfig(FD_TXRX);                                          // Configure comms chip to Full Duplex
    xTaskCreate(telemetryTask,"Telemetry",2048,nullptr,1,nullptr);      // RTOS Telemetry Task
    xTaskCreate(comRxTask,"COM RX",2048,nullptr,2,nullptr);             // RTOS Receive & Forward Task
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));                                      // Loop does nothing
}