#librm.h notes

This is the EE RPC code to access the module RMMAN in my PS2.

RMMAN is in `rom1` not `rom0`, to access it you must load the `rom0:ADDDRV` module, then you can load it manually. If the `rom0:SIO2MAN` has been loaded you _MUST_ reboot the IOP and load `rom1:SIO2MAN` fresh else it will not work. The `rom1:PADMAN` looks to use the XPADMAN RPC interface so use the appropriate library from PS2SDK.

To use it is the same as using the PAD library.

Call `RMMan_Init()` to initialise the RPC interface.
Call `RMMan_Open(port, slot, data)` to open the port with the remote in it. Data should
be a 64byte aligned data structure of 256 bytes.
Call `RMMan_Read(port, slot)` to read out the current status.

Check the status value equals either `RM_READY` or `RM_KEYPRESSED` to check its state.
If `(status == RM_KEYPRESSED)` then the button is set to one of the RM keys defines in librm.h

The remote only has a concept of one keypress at a time, if you press more than one then neither is registered.

When you hold the button then this button value is returned, when you release the button `RM_RELEASED` will be returned.

When you press (or hold) any directional arrow button on the remote, 2 button values will be returned: first DVD section button, then PS2 section button, then the value that button was released. For example when you press `UP`, then `RMMan_Read()` will return
```
RM_DVD_UP
RM_PS2_UP
RM_RELEASED
```
Each 2 minutes with no action RMMAN will return `RM_IDLE` followed by `RM_RELEASED` value.

###Sony Remote Theory

PS2 remote is from **Sony20** remote family. That means that Remote sends a 20-bit length signal, which is then parsed and translated into hex. Input IR signal has some internal standards, in short, 20 bits are divided into 3 blocks (8-5-7 bits in each block).
For example, the IR signal for `RM_PS2_SQUARE` will be
```
11011010-11010-1011111
```
The first block `11011010` in decimal will be 28, this block is identical on all Sony remotes.
The second block `11010` in decimal will be 218, this block is responsible for a specific device. In our case 218 is the codename for PS2. PS2 also accepts block 26 for DVD side buttons. These 2 families are called: `28.218` and `28.26`, both are supported by PS2.
The third block is the actual button code (7 bits) in the range 0-127.

###PS2 IR code conversion

PS2 will accept any 20 bit IR code and will convert it into its own format. For example `RM_PS2_SQUARE` IR 20-bit code will be in hex:
```
DAD5F
```
PS2 will add trailing Zeroes and change endian:
DAD5F -> DAD5F000 -> 00 0F 5D DA
In short - 3 bytes can get only 2 values (0xDAD or 0x49D) for PS2 and DVD side.
2 bytes: button number (higher bit always zero)

###RMMAN revision history
Firstly RMMAN was introduced in SCPH-18000 with DVD version 2.00. To get it to work in older models, you should install a DVD player update.
The next revision was introduced when SCPH-5xxxx was released.
The next revision was introduced when SCPH-7xxxx (slim) was released.
Also in SCPH-5xxxx appeared new module RMMAN2
The next revision was introduced when SCPH-7xxxx (slim) was released.

Were released 2 remote revisions: SCPH-10170 and SCPH-10420. The first came with an external IR receiver, which can be plugged into the controller port. The latter one contains 2 additional buttons: EJECT and RESET and has no eternal IR reciever. These 2 buttons work only on SLIM models with a built-in IR receiver. The external IR receiver has no effect when plugged into Slim PS2.

###Undocumented features
PS2 will accept additional IR commands which are not presented on its remote. Some standard DVD commands from the `28.26` family are supported.
The most useful additional commands from the PS2 side (`28.218` family) will be digital `POWER_ON` and `POWER_OFF` commands. These commands by default will work only on SLIM models.

###SCMD
It seems that there are 2 SCMD commands for RMMAN:
`0x1E` - ReadSIRc - will return pressed button code and stat value:
as an example: `0014  DAD1 6000`, where 00 60 D1 DA - button code.
`0x1F` - IgnoreSIRc - seems that this will disable IR completely

SCMD command `0x1E` is presented on PS3 Backward Compatible. On PS3 it just replicates each controller button press.

###Programmable IR remotes
Lirc format will accept 20-bit code, you only need to reverse it and translate it into hex. Example: `RM_PS2_SQUARE`
```
11011010110101011111
in reverse
11111010101101011011
in hex
0x00000000000FAB5B
```
Other information about IR signal:
```
Frequency=40000
Time Base=600
One=2,-1
Zero=1,-1
Prefix=4,-1
Message Time=45000
```
##Credits
Information about Sony remotes taken from http://www.hifi-remote.com/sony/Sony_ps2.htm
