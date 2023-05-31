HDD (APA) defragmenter (HDSK)	- 01/01/2016
--------------------------------------------

The HDD defragmenter (HDSK) module is an IOP module for reducing external fragmentation of a HDD unit.

Syntax:
	hdsk

After successful initialization, HDSK will create a hdsk device, which will have the following I/O functions:
	devctl
		Used for controlling the HDSK module.

		Codes:
			HDSK_DEVCTL_GET_FREE		(0) = Gets an estimated time remaining for the checking operation to complete.
			HDSK_DEVCTL_GET_HDD_STAT	(1) = Gets (struct hdskStat) the amount of free space and the amount of data (in sectors) that has to be moved.
			HDSK_DEVCTL_START		(2) = Starts the defragmentation process.
			HDSK_DEVCTL_WAIT		(3) = Waits for the defragmentation operation to end.
			HDSK_DEVCTL_POLL		(4) = Polls for whether the defragmentation operation has ended (1 = in progress, 0 = done).
			HDSK_DEVCTL_GET_STATUS		(5) = Gets runtime status (0 = OK, non-zero = error).
			HDSK_DEVCTL_STOP		(6) = Aborts the defragmentation operation. Termination must be confirmed with either codes #2 or #3.
			HDSK_DEVCTL_GET_PROGRESS	(7) = Gets the progress of the defragmentation operation (returns the amount of sectors moved so far).

The different HDSK versions:
	HDD Utility Disc v1.00 (Japan):	HDSK.IRX v1.04
	HDD Utility Disc v1.10 (USA):	HDSK.IRX v1.04

	The HDSK module from the US Utility Discs are exactly the same as its counterpart from the Japanese Utility Discs,
	other than being built with a newer SDK.
