# SIO2 Manager

This module is an abstraction layer for the SIO2 interface used for memory card
and controller ports.  
This implementation of the module has backwards compatibility features to allow
different versions of e.g. `mcman`, `padman`, `mcxman`, `mtapman`, `rmman`,
`imodeman`, `sio2d`, `mc2_d`, `ds2u_d` to link and operate correctly.  

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `sio2man` -> The recommended version.
*   `sio2man-nano` -> For memory savings.
*   `sio2log` -> For debug and logging purposes.

## How to use this module in your program

In order to use this module in your program, use `LoadModule` or \
`LoadModuleBuffer` with no arguments.
