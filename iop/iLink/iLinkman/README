iLinkman v0.98H README file - 2012/02/29
============================================

iLinkman is a software driver for the Playstation 2's iLink port.

Yes, DMA support has been added. Performance isn't really up to my original expectations, however.
The i.Link hardware doesn't seem to have any mechanisms to handle byte-swapping, and the IOP is left to swap the endianness of the data on it's own. D:

On the up side, I got roughly around 7MB/s during my tests.
Not to mention that the iLink might even outperform the PS2's ethernet hardware, as the iLink hardware seems to be easier and simpler to configure and prepare for transmitting/receiving data.
Performance should still be better than the Sony iLink driver though, as less memcpy() calls are required to transfer data.

Hardware tested on:
===================
Consoles:
	*SCPH-39006 (With an unknown modchip installed, possibly a M-Chip M2 chip)
	*SCPH-10000

IEEE1394 enclosure:
	*SmartDrive IEEE1394 + USB 2.0 combo HDD enclosure.
		Chipset: Oxford Semiconductor OXUF934SSA-LQAG

Additional notes:
=================
*IEEE1394 enclosures with a Prolific PL-3507, Initio INIC-1430 and a certain Genesys chip do not work with iLinkman.

Credits:
========
Special thanks go to:
	*EEUG, for providing a partially completed IEEE1394/iLink driver for the PS2.
	*Mark K., for volunteering to be a beta tester for the early iLinkman versions.
	Silverbull, for contributing information regarding the DMAC belonging to the i.Link hardware.

A short note for developers:
============================
Most of the functions are similar to the functions available in Sony's ilink.irx module, and they are not multithread-safe.
That means that care should be taken to prevent multiple threads from calling any function from the library at the same time.
Check the header file "iLinkman.h" to see a list of exported functions.
I'll try to create a documentation for the functions soon. Meanwhile, feel free to ask me questions.

I had several resources to aid me when I was building iLinkman.
The most important ones were the LSI technical manuals titled "1394 Node Controller Core" and "1394 Physical Layer (PHY) Core".

They should be available on the Internet.
After all, I could still find them through a Google search (As of May 2011).

There are three *critical* issues with the iLink hardware of some consoles:
	1. When the console is the root node, it seems like it cannot operate at higher speeds than S100. @_@
		Such behaviour can be observed with the Sony iLink driver too - when the console is the root node, the speed will always be S100 (And it seems to rarely be able to operate at full speed; S400).
		When made to go faster, it seems like packets won't get sent (Or received by the receiver) at all. @_@
		The ideal solution to the issue will probably be to manually detect the maximum supported speed, but I think that the maximum speed will then always be S100 whenever the console is the root node lol.
		(So it such a complicated system required? Just locking the speed to S100 mode whenever the console is the root node is a simpler solution). :X @_@

		Well, if somebody can prove me wrong about this chip bug (And that there IS a correct way to control the iLink hardware), I'll be glad to be corrected.
		I had spent a LONG time trying to fix this, and have came to this conclusion after monitoring the behaviour of the Sony iLink driver.
	2. Enabling the UTD interrupt (INTR1 register) seems to cause something to crash on the IOP. It's because the IOP is too slow and the iLink hardware killed the IOP by spamming interrupts.
		->Note: The UTD interrupt works fine if the IOP doesn't remain within the interrupt context for long periods of time. During my tests, the Multi-Threaded variant of iLinkman uses the UTD interrupt just fine, as the FIFOs are handled by individual threads (And not within the interrupt context).
	3. Sometimes, the receival of packets in the DBUF FIFOs will go unannounced. D:
		** UPDATE ** This problem does exist, but the LSI documents seem to recommend another method of receiving the SELF-ID packets.
		By waiting for the SubActGap interrupt event to occur after detecting a bus reset, SELF-ID packets can be read from the DBUF FIFO without the problem of missing any of them.

		** UPDATE 2 ** The problem here is related to the issue with the usage of the UTD interrupt event: The IOP is too slow and might miss some interrupt assertions. D:
		The problem begins when the IOP is running an interrupt handler and the i.Link hardware asserts the BUS RESET interrupt event. The IOP misses that interrupt assertion and does not know about SELF-ID data in the UBUF/DBUF.
		While the UBUF/DBUF contains SELF-ID packets, the i.Link hardware won't assert further interrupts regarding bus resets at all. D:
