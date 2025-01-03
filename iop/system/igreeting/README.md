# IOP Greeting

This module will print the following information:  

* Reboot type (hard reset boot/soft reboot/update reboot/update reboot complete)  
* Board type (DTL-T only)  
* Board info (DTL-T only)  
* ROMGEN string  
* CPUID value
* CACH_CONFIG value  
* Memory size in MB  
* Whether in PS or IOP mode (IOP only)  
* Whether using Flash ROM or ROM (DTL-T only)  
* EXTINFO comment  
* DIP switches (DTL-T only)  
* EE boot parameter (DTL-T only)  
* IOP boot parameter (DTL-T only)  
* DMA_WIDE_CH value (DTL-T only)  

Additionally, it will enable SSBUS if DIP switch 5 is enabled (DTL-T only)

## Configurations

There are multiple configurations of this library, allowing the choice of
balancing between size, speed, and features.

*   `igreeting` -> The recommended version.
*   `igreeting-dtlt` -> For DTL-T systems.

## How to use this module in your program

In order to use this module in your program, you must integrate the module into
an IOPRP image, then load the image with `UDNL`.\
Using `LoadModule` or `LoadModuleBuffer` directly will not work, because the
module is already loaded at IOP boot.
