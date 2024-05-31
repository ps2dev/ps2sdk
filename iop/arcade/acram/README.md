# Arcade Extended RAM Interface

This module allows access to the extended volatile memory \
on the arcade interface \
for Bandai Namco System 246/256 systems.  
On System 246A and 246B, the extended memory is provided \
by a RAM32 or RAM64 PCB, but on newer hardware, \
RAM64 comes onboard as standard.  

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `acram` -> The recommended version.

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with no arguments.
