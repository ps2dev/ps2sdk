IEEE1394_disk v1.06E README file - 2012/02/29
=============================================

IEEE1394_disk is an SBP-2 over IEEE1394 device software driver.
It's designed to be used with iLinkman v0.98H and later.

It was based on USBHDFSD, and hence shares the features of USBHDFSD.
It supports the FAT12, FAT16 and FAT32 filesystems, and has experimental (Untested) write support.
Caution should be exercised when attempting to write data with this driver as anything can go wrong, possibly causing the existing data on your disk to be damaged.

Hardware tested on:
===================
Consoles:
	*SCPH-39006 (With an unknown modchip installed, possibly a M2 chip)
	*SCPH-10000

IEEE1394 enclosure:
	*SmartDrive IEEE1394 + USB 2.0 combo HDD enclosure.
		Chipset: Oxford Semiconductor OXUF934SSA-LQAG

Additional notes:
=================
*IEEE1394 enclosures with the Prolific PL-3507 chip do not work with IEEE1394_disk.
	I am not sure whether the Initio INIC-1430 and that certain Genesys chip which I had mentioned in the iLinkman README file are incompatible with IEEE1394_disk.

Credits:
========
USBHDFSD project developers.

Special thanks go to:
	*EEUG, for providing a partially completed SBP-2 device driver for the PS2.
	*Mark K., for volunteering to be a beta tester for the early IEEE1394_disk versions.

A short note for programmers:
=============================
In order to work on IEEE1394_disk properly, knowledge on the SBP-2 protocol is required.
The T10 Project 1155D document that is titled "Information technology - Serial Bus Protocol 2 (SBP-2)" (Revision 4; Dated May 19, 1998) was what I used, and was found through a Google search (In February 2011).
