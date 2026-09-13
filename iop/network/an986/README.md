# AN986 USB Ethernet driver

This module allows interfacing with the ADMtek an986 USB Ethernet hardware.

## Documentation

Datasheet: <https://stuff.mit.edu/afs/sipb/contrib/doc/specs/ic/network/an986.pdf>

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

* `an986` -> The recommended version.
* `an986-uepcb` -> Supports UEPCB intended for System 246C/256/147/148.

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with no arguments.
