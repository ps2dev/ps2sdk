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
 * Definitions for memory-mapped I/O for DEV5.
 */

#ifndef __DEV5_MMIO_HWPORT__
#define __DEV5_MMIO_HWPORT__

typedef struct dev5_mmio_hwport_ /* base -> 0xBF402000 */
{
	vu8 m_dev5_reg_000;
	vu8 m_dev5_reg_001;
	vu8 m_dev5_reg_002;
	vu8 m_dev5_reg_003;
	vu8 m_dev5_reg_004; /**< CDVDreg_NCOMMAND; Current N command (R/W) */
	vu8 m_dev5_reg_005; /**< CDVDreg_READY / CDVDreg_NDATAIN; N command status (R) / N command param (W) */
	vu8 m_dev5_reg_006; /**< CDVDreg_ERROR / CDVDreg_HOWTO; CDVD error (R) */
	vu8 m_dev5_reg_007; /**< CDVDreg_ABORT; BREAK */
	vu8 m_dev5_reg_008; /**< CDVDreg_PWOFF; CDVD I_STAT (R=Status, W=Acknowledge) */
	vu8 m_dev5_reg_009; /* Accessed */
	vu8 m_dev5_reg_00A; /**< CDVDreg_STATUS; CDVD drive status (R) */
	vu8 m_dev5_reg_00B; /**< CDVDreg_STICKY_STATUS; Sticky drive status (R) */
	vu8 m_dev5_reg_00C; /* Accessed */
	vu8 m_dev5_reg_00D; /* Accessed */
	vu8 m_dev5_reg_00E; /* Accessed */
	vu8 m_dev5_reg_00F; /**< CDVDreg_TYPE; CDVD disk type (R) */
	vu8 m_dev5_reg_010;
	vu8 m_dev5_reg_011;
	vu8 m_dev5_reg_012;
	vu8 m_dev5_reg_013; /* Accessed */
	vu8 m_dev5_reg_014;
	vu8 m_dev5_reg_015; /* Accessed */
	vu8 m_dev5_reg_016; /**< CDVDreg_SCOMMAND; Current S command (R/W) */
	vu8 m_dev5_reg_017; /**< CDVDreg_SDATAIN; S command status (R), S command params (W) */
	vu8 m_dev5_reg_018; /**< CDVDreg_SDATAOUT; S command result (R) */
	vu8 m_dev5_reg_019;
	vu8 m_dev5_reg_01A;
	vu8 m_dev5_reg_01B;
	vu8 m_dev5_reg_01C;
	vu8 m_dev5_reg_01D;
	vu8 m_dev5_reg_01E;
	vu8 m_dev5_reg_01F;
	vu8 m_dev5_reg_020; /* Accessed; Key block 0 data 0 */
	vu8 m_dev5_reg_021; /* Accessed; Key block 0 data 1 */
	vu8 m_dev5_reg_022; /* Accessed; Key block 0 data 2 */
	vu8 m_dev5_reg_023; /* Accessed; Key block 0 data 3 */
	vu8 m_dev5_reg_024; /* Accessed; Key block 0 data 4; used as the XOR key when enabled in CDVDreg_DEC */
	vu8 m_dev5_reg_025;
	vu8 m_dev5_reg_026;
	vu8 m_dev5_reg_027;
	vu8 m_dev5_reg_028; /* Accessed; Key block 1 data 5 */
	vu8 m_dev5_reg_029; /* Accessed; Key block 1 data 6 */
	vu8 m_dev5_reg_02A; /* Accessed; Key block 1 data 7 */
	vu8 m_dev5_reg_02B; /* Accessed; Key block 1 data 8 */
	vu8 m_dev5_reg_02C; /* Accessed; Key block 1 data 9 */
	vu8 m_dev5_reg_02D;
	vu8 m_dev5_reg_02E;
	vu8 m_dev5_reg_02F;
	vu8 m_dev5_reg_030; /* Accessed; Key block 2 data A */
	vu8 m_dev5_reg_031; /* Accessed; Key block 2 data B */
	vu8 m_dev5_reg_032; /* Accessed; Key block 2 data C */
	vu8 m_dev5_reg_033; /* Accessed; Key block 2 data D */
	vu8 m_dev5_reg_034; /* Accessed; Key block 2 data E */
	vu8 m_dev5_reg_035;
	vu8 m_dev5_reg_036;
	vu8 m_dev5_reg_037;
	vu8 m_dev5_reg_038; /**< CDVDreg_KEYSTATE */
	vu8 m_dev5_reg_039; /**< CDVDreg_KEYXOR */
	vu8 m_dev5_reg_03A; /**< CDVDreg_DEC */
} dev5_mmio_hwport_t;

#if !defined(USE_DEV5_MMIO_HWPORT) && defined(_IOP)
// cppcheck-suppress-macro constVariablePointer
#define USE_DEV5_MMIO_HWPORT() dev5_mmio_hwport_t *const dev5_mmio_hwport = (dev5_mmio_hwport_t *)0xBF402000
#endif
#if !defined(USE_DEV5_MMIO_HWPORT)
#define USE_DEV5_MMIO_HWPORT()
#endif

#endif /* __DEV5_MMIO_HWPORT__ */
