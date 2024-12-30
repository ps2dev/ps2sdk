# Simple Thread Monitor

This module provides an interface on `tty9:` (accessible from dsicons) to monitor threads on the IOP.

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `thmon` -> The recommended version.

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with, optionally, a single integer for added stack size.
