/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Definitions for memory-mapped I/O for SIF.
 */

#ifndef __SIF_MMIO_HWPORT__
#define __SIF_MMIO_HWPORT__

typedef struct sif_mmio_hwport_ /* base -> 0xBD000000 */
{
	vu32 mscom;
	u32 unused04[3];
	vu32 smcom;
	u32 unused14[3];
	vu32 msflag;
	u32 unused24[3];
	vu32 smflag;
	u32 unused34[3];
	vu32 controlreg;
	u32 unused44[7];
	vu32 unk60;
	u32 unused64[3];
} sif_mmio_hwport_t;

#if !defined(USE_SIF_MMIO_HWPORT) && defined(_EE)
// cppcheck-suppress-macro constVariablePointer
#define USE_SIF_MMIO_HWPORT() \
	sif_mmio_hwport_t *const sif_mmio_hwport = (sif_mmio_hwport_t *)0xB000F200
#endif
#if !defined(USE_SIF_MMIO_HWPORT) && defined(_IOP)
// cppcheck-suppress-macro constVariablePointer
#define USE_SIF_MMIO_HWPORT() \
	sif_mmio_hwport_t *const sif_mmio_hwport = (sif_mmio_hwport_t *)0xBD000000
#endif
#if !defined(USE_SIF_MMIO_HWPORT)
#define USE_SIF_MMIO_HWPORT()
#endif

#endif /* __SIF_MMIO_HWPORT__ */
