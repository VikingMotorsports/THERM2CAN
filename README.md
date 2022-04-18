# THERM2CAN
This repository contains the implementation for an STM32 MCU to collect data from 36 temperature sensors and then transmit these over the Controller Area Network (CAN) communication protocol.
These mimic the off-the-shelf *Thermistor Expansion Modules* from Ewert Energy. 
A technical description of the development process is available [here](https://pdxscholar.library.pdx.edu/honorstheses/1150/). 

# Author(s)
- Samuel Parker ([Email](samuel.j.parker@outlook.com))

# This project falls under the [GNU Public License 3.0](./LICENSE)

# Components
- Board: STM32 Blue Pill
- MCU: STM32F103C8T6
- Debugger: ST Link V2
- Thermistors: 10K, B-value of 3380
- 6x 8-to-1 multiplexers (MUXs)
- Orion BMS 2

# Software Tools

- ST's Hardware Abstraction Library (HAL)
- Use ac6 System Workbench to compile and debug the program (NOTE: you can also do this from the command-line with OpenOCD and GDB)

# Software Description

main() contains the following stages:
- Configuration
- Temperature Collection 
- CAN Transmission

## Configuration

Configuration functions for various peripherals (e.g. clocks, Analog-Digital Converters, CAN Controller, etc.) are located in [./src/config.c](./src/config.c) and called in main(), see the technical description for an explanation of their configurations. 

## Temperature Collection

The MCU selects the output channel for each MUX and the Analog-Digital Converter (ADC). It then collects a voltage measurement from the ADC, which it uses to calculate the resistance of the thermistor and solve for the temperature. 
It stores the following data in variables as it scans:
- A running sum
- the highest scanned temperature
- the lowest scanned temperature
- the number of faulty thermistors

## CAN Transmission
The data is then organized in a CAN message per the Orion BMS 2's [CAN message specifications](https://www.orionbms.com/downloads/misc/thermistor_module_canbus.pdf) and transmitted. 

# Notes: 
- Blue Pills are not an authentic ST item, so if you have issues flashing the boards from System Workbench, it may be an issue with the board itself
- To configure the characteristics of each module (# of thermistors, module number, thermistors per mux, number of muxs) change the `#defines` in [./inc/main.h](./inc/main.h)

