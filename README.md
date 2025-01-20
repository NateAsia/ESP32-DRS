# DRS (Drag Reduction System) Controller

## Overview
ESP32-based DRS controller for FSAE race cars. Manages the DRS wing based on vehicle dynamics and driver inputs through CAN bus communication.

## Features
- Multiple DRS operating modes:
  - Manual: Direct driver control
  - Low: Conservative automatic activation
  - High: Aggressive automatic activation
  - Open: Always active
- Real-time vehicle dynamics monitoring
- CAN bus communication at 1Mbps
- Automatic activation based on lateral/longitudinal G-forces
- Safety features including brake override

## Hardware Requirements
- ESP32 microcontroller
- CAN transceiver
- Pin Configuration:
  - CAN TX: GPIO 8
  - CAN RX: GPIO 9
  - LED: GPIO 2

## CAN Communication
### Input Message (ID: 0x653)
- Byte 0: Mode and Button Status
  - Bits 0-1: DRS Mode
  - Bit 2: Manual Button
- Bytes 1-2: Lateral G-force
- Bytes 3-4: Longitudinal G-force
- Byte 5: Brake Status

### Output Message (ID: 0x654)
- Byte 0: Online Status
- Byte 1: DRS Control State

## DRS Modes
- Manual (0): Control via manual button only
- Low (1): Conservative automatic mode (1.0 G/G slope)
- High (2): Aggressive automatic mode (0.5 G/G slope)
- Open (3): Always activated

## Building and Flashing
1. Install Arduino IDE with ESP32 support
2. Open `drs_controller_code.ino`
3. Select ESP32 board
4. Compile and flash

## Project Structure
```
src/drs_controller_code/
├── drs_controller_code.ino  # Main program
├── CANHandler.h/cpp         # CAN communication
├── DRSController.h/cpp      # DRS logic
└── constants.h              # Configuration
```

## Dependencies
- FreeRTOS
- ESP-IDF TWAI driver
- Arduino core for ESP32

## Safety Features
- Automatic deactivation on brake application
- Minimum longitudinal G-force requirement (0.1G)
- CAN timeout protection
- Mode-specific activation thresholds
