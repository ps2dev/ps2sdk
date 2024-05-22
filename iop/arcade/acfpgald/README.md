# Arcade FPGA Bitstream Loader

This module loads the Altera APEX EP20K100EQC208-2X FPGA bitstream \
on the arcade interface \
for Bandai Namco System 246/256 systems.  

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `acfpgald` -> The recommended version.

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with no arguments.
