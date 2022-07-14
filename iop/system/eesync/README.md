# EE Synchronization Module

This module will wait for SIF to be initialized.

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `eesync` -> The recommended version.
*   `eesync-nano` -> For memory savings.

## How to use this module in your program

In order to use this module in your program, you must integrate the module into
an IOPRP image, then load the image with `UDNL`.\
Using `LoadModule` or `LoadModuleBuffer` directly will not work, because the
module is already loaded at IOP boot.
