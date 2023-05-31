HardDisk ChecKer (HDCK)	- 24/05/2015
------------------------------------

The HardDisk ChecKer (HDCK) module is IOP module for ensuring the integrity of the APA scheme, which SONY uses to organize space on the PlayStation 2 HDD unit.

Syntax:
	hdck

After successful initialization, HDCK will create a hdck device, which will have the following I/O functions:
	devctl
		Used for starting off the checking process of the HDCK module.

HDCK will do the following as checks:
	1. Check that the specified HDD unit exists.
	2. Attempt to read the __mbr partition.
	3. Check partition links for integrity issues, both forward and backwards:
		a) Forward direction:
			i) If the partition has an invalid previous partition address, correct it.
			ii) Check that the final partition address is the last partition in the chain.
		b) If errors were found in a(i), then scan the disk in reverse:
			i) If the partition has an invalid next partition address, correct it.
			ii) Check that the first partition address is the first partition in the chain.

		Errors in the links will be fixed on the spot and are not considered errors.
		Only I/O errors will be treated as errors for the module to fix.
	4. If I/O errors are found, determine if the partition is a sub-partition of another partition. Attempt to recover it.
	5. If the I/O error still persists (either because it cannot be fixed or if the partition is not a sub-partition of another partition),
		delete the affected partitions:
		a) If the partition(s) is/are not the last partition(s), mark them as empty.
		b) Otherwise, delete them.
	6. If the number of bad partitions exceeds 8, abort the scanning process. No further error handling takes place.
	7. The checks on the partition links ends here. Erase the error sector record.
	8. Delete all free/empty partitions.
	9. Check the relationship between all main and sub partitions:
		a) Generate lists of main and sub partitions.
		b) For each sub-partition: check through the list of sub-partitions, to see if a the sub-partition belongs to a main partition.
			i) If the sub-partition does not belong to anything (orphaned, aka is a "missing sub"), the main partition's sub-partition list is truncated.
			ii) All further sub-partitions after the orphan, are truncated.
		c) For each sub-partition: check through the list of main partitions, to see if the sub-partition belongs to a main partition.
			i) All orphaned sub-partitions are deleted.

Known bugs/issues/limitations:
	1. if partitions are crosslinked, HDCK will get stuck in an infinite loop as it checks the links between partitions.

The different FSCK versions:
	HDD Utility Disc v1.00 (Japan):	HDCK.IRX v1.04
	PSBBN v0.32, FSCK v1.10:	HDCK.IRX v2.02

	There doesn't seem to be a difference in code between the different versions. The version number is different.
	And the code has some slight differences in generation, as if it was built with a different compiler and different library versions.
