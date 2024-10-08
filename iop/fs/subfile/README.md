# Subfile

IOP module for exposing a portion of a file (useful for loading an executable
embedded in a APA partition).

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `subfile` -> The recommended version.

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with no arguments.
