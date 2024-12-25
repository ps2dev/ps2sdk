# EE Configuration Module

This module reads configuration data needed by EE, and also sets up DVE,
DECKARD region/video mode/MAC address information, and an unknown device.  

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `eeconf` -> The recommended version.

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with no arguments.
