# SimpleFOC-Serial-Driver
Advanced dual BLDC driver modules, designed to be controlled from a trajectory controller over a serial connection. Features a compact high power design with two stacked PCBs. While the top PCB focuses on motor driving, the bottom PCB serves as the controller and interface board.

<img width="1310" height="805" alt="Screenshot 2026-09-19 211833" src="https://github.com/user-attachments/assets/1225732d-5c95-4351-a9f9-31afb10a5321" />

Work in Progress, will be updated as more information and tests become available!

# Base Board (RP2350)
<img width="2341" height="1238" alt="BaseBoard" src="https://github.com/user-attachments/assets/0dc1f6b3-7d13-436a-bd18-050768eb4cc9" />
The base board acts as a carrier for power boards and contains the microcontroller, communications and programming interfaces, and power input circuits. It shares power with top-mounted driver boards over the standoffs in the mounting holes. It's equipped with:  

- RP2354B, Dual Cortex-M33 or RISC-V processors at 150MHz
- THVD1424, communication over RS-485 or RS-422 at up to 10MHz
- Input voltage and current monitoring
- Two Encoder connections with I2C interfaces
- External connections to power, RS-422/485 and TTL UART

# Dual BLDC driver board
<img width="1455" height="1098" alt="DualShield" src="https://github.com/user-attachments/assets/28a8eaa5-c8a7-4537-8d60-8e08f9f9b501" />
Featuring two BLDC drivers and an auxiliary circuit to dissipate braking energy, the Dual BLDC board is able to drive low to medium power BLDCs at up to 7 amps. Peak currents may be far higher (not yet tested).  

- Two DRV8323S (SPI configuration) or DRV8323H (hard-coded) BLDC drivers
- Medium-Power HSBB6066 MOSFETs
- Onboard brake resistor able to dissipate ~5 watts
- Onboard thermistor to measure the board's temperature

# High-Power BLDC driver board
<img width="1283" height="906" alt="PowerShield" src="https://github.com/user-attachments/assets/7e84ff21-ed99-44a0-9cd8-2b9d67f16a4e" />
Equipped with bigger MOSFETs and connection points for an external brake resistor and power supply, this board is able to drive a much higher power motor.  

- DRV8323S BLDC driver
- 160A rated Infineon OptiMOS MOSFETs, continuous power limited by thermals (not yet tested)$
- Auxiliary output for external brake resistor
- Onboard thermistor to measure the board's temperature
