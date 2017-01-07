Playstation 2 port of the LWIP v1.4.1 protocol stack - 2012/12/24
-----------------------------------------------------------------

NOTE: This isn't meant to be just a "drop-in" replacement for any older versions of ports of the LWIP protocol stack. The exports table has been changed, and hence any modules built with the old include files meant for the old versions of the LWIP ports will be unable to link up with this module.
The inner workings of the SMAP driver has been changed too. Look at the included SMAP driver for details.

!!! Do not use the smap drivers that were meant for other LWIP ports with this module!!! It will probably fail to work properly as some of the structures that are used internally might have a different layout. !!!
!!! YOU HAVE TO REBUILD ALL MODULES THAT WILL INTERACT WITH THIS MODULE !!!

So why the change in the SMAP driver? I've changed the design so that the LWIP protocol stack doesn't have to be modified.

This network protocol stack has been written to work with the NETMAN Network Manager.

Note that because of the design of NETMAN, the module load order should be like this instead:
1. NETMAN.IRX
2. Network adaptor driver (SMAP.IRX from this PS2SDK, or something compatible which uses NETMAN).
3. LWIP network protocol stack. IP address configuration gets passed to this module instead! The device name registered is "sm".

How to install this into your PS2SDK:
-------------------------------------
1. Download the correct version of LWIP from the LWIP homepage and extract the downloaded archive into the tcpip folder of your PS2SDKSRC folder (E.g. ps2sdksrc/common/tcpip/).
2. Place a copy of ps2ip141.h (Found within the include folder in this package) into the folder containing the compiler header files for the IOP (E.g. ps2sdk/iop/include).
2. Place a copy of tcpip141.h (Found within the include folder in this package) into the folder containing the common compiler header files (E.g. ps2sdk/common/include).
3. Build this module.

Good luck!
-SP193