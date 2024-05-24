# Arcade Memory Interface RPC

This module exposes the interfaces provided by acjv, acmem, acram, and acsram \
to the EE using SIF RPC.  

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `acmeme` -> The recommended version.

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with no arguments.
