librm notes...

This is the ee rpc code to access the module RMMAN in my PS2, I don't know if this is 
compatible across different versions of the PS2. For info mine is a 30003R or so :)

RMMAN is in rom1 not rom0, I have not worked out how you get the normal ROMDRV to access
rom1 (as it should do) so I just extracted it from a BIOS dump I made. You also need
the rom1 SIO2MAN and PADMAN. If the rom0 SIO2MAN has been loaded you _MUST_ reboot the IOP
and load rom1:SIO2MAN fresh else it will not work. The rom1 PADMAN looks to use the XPADMAN
RPC interface so use the approrpiate library from PS2SDK.

To use it's the same as using the PAD library.

Call RMMan_Init() to initialise the RPC interface.
Call RMMan_Open(port, slot, data) to open the port with the remote in it. Data should
be a 64byte aligned data structure of 256 bytes.
Call RMMan_Read(port, slot) to read out the current status.

The remote only has a concept of one key press at a time, if you press more than one
then then neither is registered.

Check the status value equals either RM_READY or RM_KEYPRESSED to check it's state.
If status == RM_KEYPRESSED then button is set to one of the RM_ key defines in librm.h
