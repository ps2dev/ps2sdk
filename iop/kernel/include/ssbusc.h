/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * SSBUSC service control function definitions.
 */

#ifndef __SSBUSC_H__
#define __SSBUSC_H__

#include <irx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*	Extract of the SSBUSC control registers by wisi.
	For more information, refer to the SSBUSC documentation by Wisi.

	Delay / Configuration channel registers:
	All bit-fields are r/w unless otherwise noted.

	 3 3 2 2 2  2 2  2 2  1 1 1 1 1 1 1 0 0 0  0 0  0
	 1 0 9 8 7  4 3  1 0  6 5 4 3 2 1 0 9 8 7  4 3  0
	--------------------------------------------------
	|W|W|D|A| D  |    | D  |E|I|A|A|S|F|H|R| R  | W  |
	|A|D|M|D| M  | -  | E  |X|O|I|T|T|L|O|E| D  | R  |
	|I|M|A|E| A  |    | C  |D|I|N|Y|R|O|L|C| D  | D  |
	|T|A|F|R| T  |    | R  |L|S|C|P|B|T|D|V| L  | L  |
	--------------------------------------------------

	3:0	WRDL	Write delay: /SWR active low period: 0-15 [cycles+1]
	7:4	RDDL	Read delay: /SRD active low period: 0-15 [cycles+1]
	8	RECV	Recovery period: 0= off / 1= use commonDelay.3:0
	9	HOLD	Hold period: 0= off / 1= use commonDelay.7:4
	10	FLOT	Floating period: 0= off / 1= use commonDelay.11:8
	11	STRB	Pre-Strobe period: 0= off / 1= use commonDelay.15:12
	12	ATYP	Access type: 0= 8bit / 1= 16bit. Also affects DMA bus width, when not using Wide DMA mode.
	13	AINC	Increment address when accessing a register shorter than the access instruction: 0= off / 1= increment.
			Example: when reading a word from a byte-wide bus, this option sets whether the address on each consecutive 8-bit access would increment or not.
			If disabled the byte at the base address of the word would be read 4 times, otherwise the bytes at +0, +1, +2, +3 will be read in that order.
			This is useful (to be disabled) for devices like ATA interfaces, where some ATA registers are written twice for extended LBA
			(which can be done with a single word-access).
	14	IOIS16	I/O is 16-bit: /IOIS16 signal: 0= don't use it / 1= regard it.
			Supported by only(?) DEV9 (but which Dev9M or Dev9I(more likely) or both?).
			Most likely overrides the Access Type (8/16 -bit) setting (tested, but doesn't seem to have any effect).
			/IOIS16 should be an input to the IOP, but is marked as output.
			A peripheral signals by this signal that the register accessed is 16-bit (when low). See Dev9C document for more info. 

			From PCMCIA specs (volume 2 / r7.0, february 1999).
			-----
			The IOIS16# output signal is asserted when the address at the socket corresponds to an I/O address to which the card responds,
			and the I/O port addressed is capable of 16-bit access.
			When this signal is not asserted during a 16-bit I/O access,
			the system will generate 8-bit references to the even and odd byte of the 16-bit port being accessed.
			-----
	15	EXDL	Extended delay: MSb of /SRD active low period. Not found to have any effect (regardless of reg. 1020).
	20:16	DECR	Decoding range = 2^n bytes. Values: 0 - 27 = 1B - 128MB.
	23:21	r/o	Unused, 0.
	27:24	DMAT	DMA timing: /SRD /SWR active.low period: 0-14 [cycles+1]. The inactive period is always 1cycle. 
			0=1cycle, 1=2cycles, ..., 14=15cy
			15(0xF)= '0.5cycles' Each word still remains on the bus for 1cy, but there are no inactive pauses between the words.
				- 'fastest' DMA mode: /SRD/SWR is kept active low over the whole transfer and the SSCLK clocks the data words. 
			On some devices (PS2 mode only?) the /SRD active low period cannot be 1 cycle, thus with DMAT = 0 or 1, the resulting period is 2cy.
			It is unknown if this is caused by some setting. The /SWR period is unaffected. 
	28	ADER	r/o Address error flag (when =1). When the an address error within the memory range of this channel occurs this bit becomes set.
			Writing 1 clears it. Unknown what Address Error means. This doesn't become set by unaligned access.
			It might be for the DMAC accessing RAM past its end when doing DMA through this SSBUSC channel, but it doesn't activate on that either. 
	29	DMAF	DMA select timing source flag: 0= use PIO timing / 1= use DMA timing bit-field 28:24 for DMA.
	30	WDMA	Wide DMA mode. When = 1 this overrides the Access Type (8/16 bit) setting, and causes 32 bits to be transferred in one go.
			The low 16bits are mapped to the SD15:SD0 data lines, and the high 16bits - to the low address lines SA15:SA0, which become inputs,
			if the transfer is done 'to RAM'. 
			This is why most devices get all the low 16 address bits, even though their address space is much smaller than 64kbyte.
			The SPU2 doesn't get all 16 low address lines, so this is disabled for it, while it is enabled for the CDVD and Dev9M, Dev9I.
 	31	WAIT	/WAIT signal. When = 1 enables the IOP waiting on the /WAIT (input to IOP) signal, when accessing the range of the given device. 
			Slow devices can make the IOP wait (indefinitely) before transferring data, by asserting the /WAIT line low while a PIO access is being done
			(after the device detected assertion of /CS and decoded the address). 
			On read, once /WAIT is deasserted, IOP loads the data (it was made to wait for) from the device/bus (on read) and continues execution. 
			On write the write operation completes even if /WAIT is asserted low and the IOP can continue executing. This is true for any device.
			However if /WAIT is kept asserted low and another write (or a read) access is done (to any device - even such with /WAIT hardwired to disabled state),
			then the IOP will stall before (or after?) transferring any data, until /WAIT is deasserted.
			This can be used as a hack to use /WAIT even of devices that have it disabled (like the BOOT ROM for example):
			write a device that has it enabled (while /WAIT is low) then access (r/w) the BOOT ROM and the IOP will stall until /WAIT is deasserted.
			/WAIT should be driven most likely by OC/OD drivers (or a tri-state output enabled on /CS). The output of the Dev9C seems to be just such.
 
	There are two main modes of transfer: PIO and DMA.
	The timing parameters of the PIO mode can be set in detail, while for the DMA mode a symmetric clock is assumed only with period configurable.
	PIO timing configuration can be used for DMA transfers too.

	For DMA on read (->IOP) both for 0 and 1 DMA periods 1 cycle is added, which results in 2 cycle transfer - reason unknown.
	On write, 0->1cycle, 1->2cycles (reason unknown).
  
	Besides the /SRD /SWR active.low periods that can be set individually for each channel,
	the following parameters exist, that can only be enabled for the individual channels,
	but the value is common for all. Their values are set by the Common Delay register.

	1020	Common delay register r/w:
		3:0	Recovery period 0 - 15 cycles.
		7:4	Hold period	0 - 15 cycles.
		11:8	Floating release delay 0 - 15 cycles.
		15:12	Strobe active-going edge delay 0 - 15 cycles.
		16	r/w ? 0 by default.
		17	r/w ? 0 by default.
		31:18	r/o, = 0 unused.

	Recovery period: Elongates with commonDelay.3:0 cycles the period after the data bus is released by the IOP (write) (after the added hold period, if any)
	or after /SRD goes inact.high, to the next active /SRD/SWR pulse or /CS going inactive transition,
	to give the device more time to release the bus on read or give more bus turnaround time to the next access on write.

	Hold period: The period after /SWR goes inact.high when the data bus keeps its data valid, necessary for signal propagation and correct latching.
	The /SWR active.low inter-pulse period will be elongated with commonDelay.7:4 cycles in which after /SWR goes inactive.high the data output from the IOP would still be kept valid.
	This setting has no effect on /SRD read timing.

	Floating period: Elongate the /CS (active low) and the inter-/SRD pulses periods after /SRD goes inactive to give the device more time to leave the bus
	floating after driving it (for slow devices that release the bus slowly).
	Give the device commonDelay.11:8 cycles more after /SRD goes inactive to the next /SRD pulse or the end of the cycle (/CS -> inact.high),
	to leave the bus floating. When set to 0, the period /SRD inact.high-going edge to /CS inact.high -going edge is 1/2 cycle and is 1cy between /SRD pulses act.low pulses.
	This setting has no effect on /SWR write timing.

	Pre-Strobe period: Delay from the start of data output (and /CS going active.low) to the active.low -going edge of /SRD or /SWR:
	Shifts the active-going edge of /SWR the commonDelay.15:12 cycles later than it would usually come,
	so data can be read on the active-going falling edge.
	If the active period of the act.low pulse is less than this strobe delay, then the inactive periods between pulses are elongated to account for that.
	The active pulse is shrunk to a minimum of 1cy in this case. /CS act.low-going edge to /SRD or /SWR act-low going edge is of this number of cycles (otherwise = 0).
	While delayReg./SRD/SWR_act.low_period <= strobePeriod, no change in the total period is done.
	Pulse active.low period = max((delayReg.act.low_period - strobePer), 1). - at least 1 cy. This has the same effect on both /SRD and /SWR.	*/

/**	This enum lists all devices controlled by the SSBUSC.
	Note that the PS2 has some physical device assignments, which are not directly related to the SSBUSC's devices.
	Usually, these devices are related to the assignments of interrupt, chip-select and DMA control line sets.
	For example, DEV9 is known as such because it is assigned INT9, CS9, DACK9 and DREQ9.
	However, DEV9 is controlled as SSBUSC devices 10-12.

	There can be coincidences, like how DEV5 (CD/DVD DSP) also has a SSBUSC device ID of 5.	*/

enum SSBUSC_DEV {
	/** Seems to affect the memory map entirely. */
	SSBUSC_DEV0 = 0,
	/** DVD ROM chip (contains rom1 and erom), DEV1 */
	SSBUSC_DEV_DVDROM,
	/** BOOT ROM chip (contains rom0), DEV2 */
	SSBUSC_DEV_BOOTROM,
	SSBUSC_DEV3,
	/** SPU, DEV4 */
	SSBUSC_DEV_SPU,
	/** CD/DVD hardware, DEV5 */
	SSBUSC_DEV_CDVD,
	SSBUSC_DEV6,
	SSBUSC_DEV7,
	SSBUSC_DEV8,
	/** SPU2 */
	SSBUSC_DEV_SPU2,
	//Controls for the DEV9 expansion interface.
	SSBUSC_DEV_DEV9I,	//DEV9 I/O Window
	SSBUSC_DEV_DEV9M,	//DEV9 Memory Window
	SSBUSC_DEV_DEV9C	//DEV9 Controller
};

/* SSBUSC Register Map, by Wisi

ID  - PS2 Function - PS Function - Address     - Delay (Configuration)
---------------------------------------------------------------------------------------
0     - Exp1       - Exp1        - 0xBF801000 - 0xBF801008
1     - DVD ROM    - Exp3        - 0xBF801400 - 0xBF80100C
2     - Boot ROM   - Boot ROM    - -          - 0xBF801010
3     - ???        - ???         - -
4     - SPU        - SPU         - 0xBF801404 - 0xBF801014
5     - CD/DVD     - CD-ROM      - 0xBF801408 - 0xBF801018
6     - ???        - ???         - -          - -
7     - ???        - ???         - -          - -
8     - Exp2       - Exp2        - 0xBF801004 - 0xBF80101C
9     - SPU2       - N/A         - 0xBF80140C - 0xBF801414
10    - DEV9I      - N/A         - -          - 0xBF801418
11    - DEV9M      - N/A         - 0xBF801410 - 0xBF80141C
12    - DEV9C      - N/A         - -          - 0xBF801420

Common Delay register: 0xBF801020	*/

//These functions control access configuration (i.e. window size and access cycles) and memory map of the devices.
int SetDelay(int device, unsigned int value);
int GetDelay(int device);
int SetBaseAddress(int device, unsigned int value);
int GetBaseAddress(int device);

//Helper functions for getting/setting various fields within the common delay register.
int SetRecoveryTime(unsigned int value);
int GetRecoveryTime(void);
int SetHoldTime(unsigned int value);
int GetHoldTime(void);
int SetFloatTime(unsigned int value);
int GetFloatTime(void);
int SetStrobeTime(unsigned int value);
int GetStrobeTime(void);

//Direct access to the common delay register.
int SetCommonDelay(unsigned int value);
int GetCommonDelay(void);

#define ssbusc_IMPORTS_start DECLARE_IMPORT_TABLE(ssbusc, 1, 1)
#define ssbusc_IMPORTS_end END_IMPORT_TABLE

#define I_SetDelay DECLARE_IMPORT(4, SetDelay)
#define I_GetDelay DECLARE_IMPORT(5, GetDelay)
#define I_SetBaseAddress DECLARE_IMPORT(6, SetBaseAddress)
#define I_GetBaseAddress DECLARE_IMPORT(7, GetBaseAddress)
#define I_SetRecoveryTime DECLARE_IMPORT(8, SetRecoveryTime)
#define I_GetRecoveryTime DECLARE_IMPORT(9, GetRecoveryTime)
#define I_SetHoldTime DECLARE_IMPORT(10, SetHoldTime)
#define I_GetHoldTime DECLARE_IMPORT(11, GetHoldTime)
#define I_SetFloatTime DECLARE_IMPORT(12, SetFloatTime)
#define I_GetFloatTime DECLARE_IMPORT(13, GetFloatTime)
#define I_SetStrobeTime DECLARE_IMPORT(14, SetStrobeTime)
#define I_GetStrobeTime DECLARE_IMPORT(15, GetStrobeTime)
#define I_SetCommonDelay DECLARE_IMPORT(16, SetCommonDelay)
#define I_GetCommonDelay DECLARE_IMPORT(17, GetCommonDelay)

#ifdef __cplusplus
}
#endif

#endif /* __SSBUSC_H__ */
