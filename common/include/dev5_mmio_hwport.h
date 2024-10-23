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
	vu8 m_dev5_reg_004;
	vu8 m_dev5_reg_005;
	vu8 m_dev5_reg_006;
	vu8 m_dev5_reg_007;
	vu8 m_dev5_reg_008;
	vu8 m_dev5_reg_009;
	vu8 m_dev5_reg_00A;
	vu8 m_dev5_reg_00B;
	vu8 m_dev5_reg_00C;
	vu8 m_dev5_reg_00D;
	vu8 m_dev5_reg_00E;
	vu8 m_dev5_reg_00F;
	vu8 m_dev5_reg_010;
	vu8 m_dev5_reg_011;
	vu8 m_dev5_reg_012;
	vu8 m_dev5_reg_013;
	vu8 m_dev5_reg_014;
	vu8 m_dev5_reg_015;
	vu8 m_dev5_reg_016;
	vu8 m_dev5_reg_017;
	vu8 m_dev5_reg_018;
	vu8 m_dev5_reg_019;
	vu8 m_dev5_reg_01A;
	vu8 m_dev5_reg_01B;
	vu8 m_dev5_reg_01C;
	vu8 m_dev5_reg_01D;
	vu8 m_dev5_reg_01E;
	vu8 m_dev5_reg_01F;
	vu8 m_dev5_reg_020;
	vu8 m_dev5_reg_021;
	vu8 m_dev5_reg_022;
	vu8 m_dev5_reg_023;
	vu8 m_dev5_reg_024;
	vu8 m_dev5_reg_025;
	vu8 m_dev5_reg_026;
	vu8 m_dev5_reg_027;
	vu8 m_dev5_reg_028;
	vu8 m_dev5_reg_029;
	vu8 m_dev5_reg_02A;
	vu8 m_dev5_reg_02B;
	vu8 m_dev5_reg_02C;
	vu8 m_dev5_reg_02D;
	vu8 m_dev5_reg_02E;
	vu8 m_dev5_reg_02F;
	vu8 m_dev5_reg_030;
	vu8 m_dev5_reg_031;
	vu8 m_dev5_reg_032;
	vu8 m_dev5_reg_033;
	vu8 m_dev5_reg_034;
	vu8 m_dev5_reg_035;
	vu8 m_dev5_reg_036;
	vu8 m_dev5_reg_037;
	vu8 m_dev5_reg_038;
	vu8 m_dev5_reg_039;
	vu8 m_dev5_reg_03A;
} dev5_mmio_hwport_t;

#if !defined(USE_DEV5_MMIO_HWPORT) && defined(_IOP)
// cppcheck-suppress-macro constVariablePointer
#define USE_DEV5_MMIO_HWPORT() dev5_mmio_hwport_t *const dev5_mmio_hwport = (dev5_mmio_hwport_t *)0xBF402000
#endif
#if !defined(USE_DEV5_MMIO_HWPORT)
#define USE_DEV5_MMIO_HWPORT()
#endif

#endif /* __DEV5_MMIO_HWPORT__ */
