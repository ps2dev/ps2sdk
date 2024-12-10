# Arcade UART Interface

This module allows access to the Maxim MAX232 UART hardware \
on the arcade interface \
for Bandai Namco System 246/256 systems.  

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `acuart` -> default version. same behavior than original module.
*   `acuart-tty` -> exactly the same than default. but will redirect stdout to the uart as soon as module boots

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with no arguments.

## Hardware Notes
The UART is easily accesible through the white JST 3 pin connector located on the frontal panel of the arcade board, just between JVS USB and the dip switches.
From left to right, the pins are: RX, TX, GND.

An USB to RS232 (usually with DB9 connector) is recommended
