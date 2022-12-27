# ROM/Flash Driver

IOP module for handling the filesystem contained in the ROM chip attached to DEV2.  

The source code for this module is based upon the source code from PS2Ident v0.835.  

## How to use this module in your program

In order to use this module in your program, you must integrate the module into
an IOPRP image, then load the image with `UDNL`.\
Using `LoadModule` or `LoadModuleBuffer` directly will not work, because the
module is already loaded at IOP boot.
