# SimpleFOC-Serial-Driver
Advanced dual BLDC driver based on the RP2350 and DRV8323 ICs, designed to be controlled from a trajectory controller over RS-485, RS-422 or TTL UART. Features a compact high power design with two stacked PCBs. While the top PCB focuses on motor driving with two DRV8323 motor drivers, it also includes an additional FET and brake resistors to dissipate energy during braking and a thermistor to monitor drive temperature. The bottom PCB serves as the controller and interface board, featuring a RP2354B Microcontroller, THVD1424 communications chip for RS-485 or RS-422 communication, input voltage and current sensing, powerful buck converter to power any 3.3V consumers that may be added to motor (or other) shields later, and all peripherals required for USB programming.

The controller board is designed as a generic base unit to allow use with different actor shields. It can currently be used with MKS Dual FOC boards (tested with v3.2) and the custom driver board from this repository.

<img width="1310" height="805" alt="Screenshot 2026-09-19 211833" src="https://github.com/user-attachments/assets/1225732d-5c95-4351-a9f9-31afb10a5321" />

Work in Progress, will be updated as more information and tests become available.
