# Arcade Extended RAM Interface

This module allows access to the extended volatile memory on the arcade interface for Bandai Namco System 246/256 systems.

Originally, the ACRAM came as external boards, both RAM32 PCB and RAM64 PCB

- on System246A (at least on DRIVING version) the system has two slots for ACRAM boards. (only both wangan midnight games need two RAM64PCBs)
- on System246B and 246C 64MB of ACRAM are mounted directly on the motherboard
- on System256 the ACRAM is no longer part of the system

just like the ATA interface, it depends on the altera FPGA programmed by acfpgald.irx

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `acram` -> The recommended version.
*   `acram_extended` -> Supports up to 128mb of ACRAM, only on system246 Rack A

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with no arguments.
