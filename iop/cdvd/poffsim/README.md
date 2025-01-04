# Power off simulation

This module simulates power off callbacks, equivalent to the `POWEROFF` module on DTL-T.  
This is useful for situations when debugging on systems that do not support
power off callbacks, such as SCPH-10000.

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `poffsim` -> The recommended version.

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with no arguments.
