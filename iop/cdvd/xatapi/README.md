# DEV5 SPEED module for ATAPI

This module allows access to the ATAPI CD/DVD drive connected to SPEED on DEV5.
This module also contains support code for switching between the IDE/ATAPI and
MechaCon/Dragon-connected CD/DVD drives.

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `xatapi` -> The recommended version.

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with no arguments.
