# FILEIO server service module

This module provides file and heap interfaces to the EE.

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `fileio` -> The recommended version.

## How to use this module in your program

In order to use this module in your program, you must integrate the module into
an IOPRP image, then load the image with `UDNL`.\
Using `LoadModule` or `LoadModuleBuffer` directly will not work, because the
module is already loaded at IOP boot.
