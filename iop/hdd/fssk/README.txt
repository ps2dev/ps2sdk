PlayStation FileSystem (PFS) FileSystem trimmer (FSSK)	- 01/01/2016
--------------------------------------------------------------------

The FileSystem trimmer (FSSK) module is an IOP module for reducing internal fragmentation of PlayStation FileSystem (PFS) partitions.

Syntax:
	fssk <options>
		Options:
			-n <buffers>	- specifies the number of buffers that FSSK will use, which affects performance.

After successful initialization, FSSK will create a fssk device, which will have the following I/O functions:
	open:
		Opens the partition that will be checked, with a path like "fssk:hdd0:__system".
		The mode parameter has the following bit definition:	VVVV0000
			V = Verbosity level (0-15).
			0 = unused. Must be set to 0.
	close
		Used to close the partition that will be checked.
	ioctl2
		Used for controlling the FSSK module.

		Codes:
			FSSK_IOCTL2_CMD_GET_ESTIMATE	(0) = Gets an estimated time remaining for the checking operation to complete.
			FSSK_IOCTL2_CMD_START		(1) = Starts the checking operation.
			FSSK_IOCTL2_CMD_WAIT		(2) = Waits for the checking operation to end.
			FSSK_IOCTL2_CMD_POLL		(3) = Polls for whether the checking operation has ended (1 = in progress, 0 = done).
			FSSK_IOCTL2_CMD_GET_STATUS	(4) = Gets runtime data (struct fsskStatus, 28 bytes).
			FSSK_IOCTL2_CMD_STOP		(5) = Aborts the checking operation. Termination must be confirmed with either codes #2 or #3.
			FSSK_IOCTL2_CMD_SET_MINFREE	(6) = Sets the minimum free threshold in percentage. The default is 3, but the minimum that can be specified is 4.
			FSSK_IOCTL2_CMD_SIM		(7) = Runs a simulation. The partsDeleted status will indicate the number of partitions to deleted and the size of the partitions deleted will be returned.

The different FSSK versions:
	HDD Utility Disc v1.00 (Japan):	FSSK.IRX v1.04
	HDD Utility Disc v1.10 (USA):	FSSK.IRX v1.04

	The FSSK module from the US Utility Discs are exactly the same as its counterpart from the Japanese Utility Discs,
	other than being built with a newer SDK.
