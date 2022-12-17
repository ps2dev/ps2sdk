PlayStation FileSystem (PFS) FileSystem ChecKer (FSCK)	- 27/05/2015
--------------------------------------------------------------------

The FileSystem ChecKer (FSCK) module is an IOP module for ensuring the integrity of PlayStation FileSystem (PFS) partitions.

Syntax:
	fsck <options>
		Options:
			-n <buffers>	- specifies the number of buffers that FSCK will use, which affects performance.

After successful initialization, FSCK will create a fsck device, which will have the following I/O functions:
	open:
		Opens the partition that will be checked, with a path like "fsck:hdd0:__system".
		The mode parameter has the following bit definition:	VVVV00PW
			W = Write enabled (0 = read-only).
			P = Prompt user before performing each fix.
			V = Verbosity level (0-15).
			0 = unused. Must be set to 0.

		When this function is invoked FSCK will attempt to:
			1. Get information on the partition with getstat().
			2. Identify the block device (only hdd: is supported).
			3. Open the partition/block device.
			4. Check super block.
			5. Initialize bitmap.
			6. Check all zones.
	close
		Used to close the partition that will be checked.
	ioctl2
		Used for controlling the FSCK module.

		Codes:
			FSCK_IOCTL2_CMD_GET_ESTIMATE	(0) = Gets an estimated time remaining for the checking operation to complete.
			FSCK_IOCTL2_CMD_START		(1) = Starts the checking operation.
			FSCK_IOCTL2_CMD_WAIT		(2) = Waits for the checking operation to end.
			FSCK_IOCTL2_CMD_POLL		(3) = Polls for whether the checking operation has ended (1 = in progress, 0 = done).
			FSCK_IOCTL2_CMD_GET_STATUS	(4) = Gets runtime data (struct fsckStatus, 28 bytes).
			FSCK_IOCTL2_CMD_STOP		(5) = Aborts the checking operation. Termination must be confirmed with either codes #2 or #3.

FSCK will check the following:
	1. (For FSCK v1.10 only) extended attribute information.
	2. Root directory	- integrity of inodes.
	3. Check all files	- integrity of directory entries and inodes.
	4. Compare the bitmap (checks against the bitmap that it generates, as it checks through all files).
	5. Clears the PFS error status bit (1).
	6. If errors were fixed, bit 2 of the super block is set.

Known bugs:
	While a directory entry is checked through, a NULL pointer appears to be dereferenced when an inode cannot be read (wrong method of handling the error?).

The different FSCK versions:
	HDD Utility Disc v1.00 (Japan), FSCK v1.00:	FSCK.IRX v1.04
	HDD Utility Disc v1.00 (USA), FSCK v1.00:	FSCK.IRX v1.04
	HDD Utility Disc v1.10 (USA), FSCK v1.10:	FSCK.IRX v1.04
	PSBBN v0.32, FSCK v1.10:			FSCK.IRX v2.01

	The FSCK module from the US Utility Discs are exactly the same. However, compared to the original Japanese version (despite the similar version number):
		1. The partition's extended attribute area is now checked (read) and will be zero-filled if it cannot be read properly.
		2. Bitmap mismatched errors are no longer recorded as errors, although opting to not fix them will still cause FSCK to terminate.
		3. The __mbr partition is now used for storing the generated bitmap, instead of __tmp.
		4. The PFS library that FSCK is compiled with, appears to be newer (the pfsGetTime function has a newer design).

	There doesn't seem to be a difference in code between the PSBBN FSCK v1.10 module (v2.01) and the modules that come from the US Utility Discs.
	The version number is different. And the code has some slight differences in generation, as if it was built with a different compiler.
