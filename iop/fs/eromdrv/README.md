# Encrypted ROM driver

IOP module for handling the encrypted filesystem contained in the ROM
chip attached to DEV1.

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `eromdrv` -> The recommended version.

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with no arguments.
