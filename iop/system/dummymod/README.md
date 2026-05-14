# Dummy IOP Module

This module is a minimal module intended to be used where a module is used but
a no-op is desired, such as a module in an IOPRP image.

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `dummymod` -> The recommended version.

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with no arguments.
