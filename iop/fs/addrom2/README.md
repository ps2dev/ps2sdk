# Additional ROM driver

IOP module for handling the filesystem contained in the ROM chip attached to DEV1 at an offset, usually containing the Chinese font.  

## How to use this module in your program

In order to use this module in your program, you must integrate the module into
an IOPRP image, then load the image with `UDNL`.\
Using `LoadModule` or `LoadModuleBuffer` directly will not work, because the
module is already loaded at IOP boot.
