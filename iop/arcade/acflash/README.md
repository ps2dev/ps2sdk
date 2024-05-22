# Arcade Flash Memory driver

This module allows flashing the flash ROM on the arcade interface \
for Bandai Namco System 246/256 systems.  
If ROMDIR compliant data is flashed, \
it will be accessible with `acdev` using `rom1:`.  

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `acflash` -> The recommended version.

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with no arguments.
