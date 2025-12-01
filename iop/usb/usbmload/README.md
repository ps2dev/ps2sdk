# USB Automatic Module Loader

This module provides functions to allow IOP modules to be automatically loaded
and unloaded when a USB device is plugged based on its characteristics.

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `usbmload` -> The recommended version.

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with no arguments.
