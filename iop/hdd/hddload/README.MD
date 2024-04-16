# HDDLOAD
The HDDLOAD module is used for loading a bootstrap program from the `__mbr` partition of the attached HDD unit. The program is expected to be an ELF file, headerless, with load address `0x00100000` and appropriately-flagged KELF. (`kelftool encrypt mbr ELF KELF`)

Some PlayStation 2 consoles like the `SCPH-10000`, `SCPH-15000` and `SCPH-18000` do not support their HDD units, the Sony HDD Utility Disc may be used to install a "System Driver Update" that adds the ability to boot from the HDD.

While not intended by Sony, it is possible to use this system to boot a HDD update on the `SCPH-700xx`.

This update will boot a HDDLOAD module, which works similarly to the one found in the boot ROM (`rom0:HDDLOAD` on expansion bay models).

This package contains the source code a clone of the HDDLOAD module, based on a module from the 1.60 ROM.

# Dependencies

It depends on `dev9.irx` and `atad.irx` tp function.


## Workflow
- Call either `BootHDDLoadFile()` or `BootHDDLoadBuffer()` depending if the module will be loaded from a file or buffer. This will attempt to load `hddload.irx`, on the given memory card. If an update can be loaded, the update will be loaded to `0x00100000`.
This will take some time as the HDD may be spun up, so you could consider doing this in another thread. Otherwise, the user will be greeted with a black screen.  Sony renders the "Sony Computer Entertainment" boot screen as this is being done.
- Call `DetermineHDDLoadStatus()`.
- Call `GetHddSupportEnabledStatus()` and `GetHddUpdateStatus()` to determine whether HDD booting was enabled and if an update was successfully booted.
- If both return 1, then boot the executable can be executed from `0x00100000`.


## Configuration
The IOP side will access the configuration storage area which is set up by EELOAD, which resides at `0x000003c0`. This is loaded from the MECHACON's NVRAM storage.

- Bits:
```
 1 = Disable ATAD. By default, this is not set.
 2 = Disable HDD support. PS2 consoles come with this bit set.  
```
So if the HDD browser was never installed on the PS2, HDD support is disabled. Which results in HDDLOAD switching off the HDD immediately after the program is loaded.


## ATAD check
As described above, the `ATAD` module from the boot ROM is a combination of `DEV9` and `ATAD` from the Sony PS2 SDK. It will also check whether `ATAD` is enabled.  
This bit is not set by default (`ATAD` is enabled by default). This is what the `ATAD` module checks:
```c
static int CheckATADEnabled(void){
    return(!((*(*(vu8 **)0x000003c0))&1));
}
```

## Enabling HDD support
As shown in the FMCB Installer, the following can be done from the EE to enable HDD booting on the PS2. This should be done during installation:

```c
static int EnableHDDBooting(void){
	int OpResult, result;
	unsigned char OSDConfigBuffer[15];

	do {
		sceCdOpenConfig(0, 0, 1, &OpResult);
	} while(OpResult&9);

	do {
		result=sceCdReadConfig(OSDConfigBuffer, &OpResult);
	} while(OpResult&9 || result==0);

	do {
		result=sceCdCloseConfig(&OpResult);
	} while(OpResult&9 || result==0);

	if ((OSDConfigBuffer[0]&3)!=2) {	//If ATAD support and HDD booting are not already activated.
		OSDConfigBuffer[0]=(OSDConfigBuffer[0]&~3)|2;

		do {
			sceCdOpenConfig(0, 1, 1, &OpResult);
		} while(OpResult&9);

		do {
			result = sceCdWriteConfig(OSDConfigBuffer, &OpResult);
		} while(OpResult&9 || result==0);

		do {
			result = sceCdCloseConfig(&OpResult);
		} while(OpResult&9 || result==0);

		result = 0;
	} else result = 1;

	return result;
}
```
