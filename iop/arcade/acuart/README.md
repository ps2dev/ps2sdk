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
