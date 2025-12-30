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
 * Definitions for memory-mapped I/O for System 147.
 */

#ifndef __S147_MMIO_HWPORT__
#define __S147_MMIO_HWPORT__

typedef struct s147_dev9_mem_mmio_
{
	vu8 m_unk00;
	vu8 m_led;
	vu8 m_security_unlock_unlock;
	vu8 m_unk03;
	vu8 m_rtc_flag;
	vu8 m_watchdog_flag2;
	vu8 m_unk06;
	vu8 m_sram_write_flag;
	vu8 m_pad08;
	vu8 m_pad09;
	vu8 m_pad0A;
	vu8 m_pad0B;
	vu8 m_security_unlock_set1;
	vu8 m_security_unlock_set2;
} s147_dev9_mem_mmio_t;

#if !defined(USE_S147_DEV9_MEM_MMIO) && defined(_IOP)
// cppcheck-suppress-macro constVariablePointer
#define USE_S147_DEV9_MEM_MMIO() s147_dev9_mem_mmio_t *const s147_dev9_mem_mmio = (s147_dev9_mem_mmio_t *)0xB0000000
#endif
#if !defined(USE_S147_DEV9_MEM_MMIO)
#define USE_S147_DEV9_MEM_MMIO()
#endif

typedef struct s147nand_dev9_io_mmio_
{
	vu8 m_nand_waitflag;          // 0 (R/B)
	vu8 m_nand_cmd_enable;        // 1 (CE+WE)
	vu8 m_nand_cmd_sel;           // 10 (CE+WE+CLE)
	vu8 m_nand_cmd_offs;          // 11 (CE+WE+ALE)
	vu8 m_nand_write_cmd_unlock;  // 100
	vu8 m_pad05;
	vu8 m_pad06;
	vu8 m_pad07;
	vu8 m_nand_outbyte;           // 1000 (CE+RE)
} s147nand_dev9_io_mmio_t;

#if !defined(USE_S147MAMD_DEV9_IO_MMIO) && defined(_IOP)
// cppcheck-suppress-macro constVariablePointer
#define USE_S147MAMD_DEV9_IO_MMIO() s147nand_dev9_io_mmio_t *const s147nand_dev9_io_mmio = (s147nand_dev9_io_mmio_t *)0xB4000000
#endif
#if !defined(USE_S147MAMD_DEV9_IO_MMIO)
#define USE_S147MAMD_DEV9_IO_MMIO()
#endif

typedef struct s147link_dev9_mem_mmio_
{
	vu8 m_pad00;
	vu8 m_unk01;
	vu8 m_pad02;
	vu8 m_unk03;
	vu8 m_pad04;
	vu8 m_node_unk05;
	vu8 m_pad06;
	vu8 m_unk07;
	vu8 m_pad08;
	vu8 m_unk09;
	vu8 m_pad0A;
	vu8 m_pad0B;
	vu8 m_pad0C;
	vu8 m_unk0D;
	vu8 m_pad0E;
	vu8 m_pad0F;
	vu8 m_pad10;
	vu8 m_pad11;
	vu8 m_stsH_unk12;
	vu8 m_stsL_unk13;
	vu8 m_unk14;
	vu8 m_unk15;
	vu8 m_pad16;
	vu8 m_unk17;
	vu8 m_pad18;
	vu8 m_pad19;
	vu8 m_pad1A;
	vu8 m_pad1B;
	vu8 m_unk1C;
	vu8 m_unk1D;
	vu8 m_rxfc_hi_unk1E;
	vu8 m_rxfc_lo_unk1F;
	vu8 m_pad20;
	vu8 m_unk21;
	vu8 m_unk22;
	vu8 m_unk23;
	vu8 m_unk24;
	vu8 m_unk25;
	vu8 m_pad26;
	vu8 m_pad27;
	vu8 m_unk28;
	vu8 m_unk29;
	vu8 m_pad2A;
	vu8 m_maxnode_unk2B;
	vu8 m_pad2C;
	vu8 m_mynode_unk2D;
	vu8 m_pad2E;
	vu8 m_unk2F;
	vu8 m_pad30;
	vu8 m_unk31;
	vu8 m_pad32;
	vu8 m_pad33;
	vu8 m_watchdog_flag_unk34;
} s147link_dev9_mem_mmio_t;

#if !defined(USE_S147LINK_DEV9_MEM_MMIO) && defined(_IOP)
// cppcheck-suppress-macro constVariablePointer
#define USE_S147LINK_DEV9_MEM_MMIO() s147link_dev9_mem_mmio_t *const s147link_dev9_mem_mmio = (s147link_dev9_mem_mmio_t *)0xB0800000
#endif
#if !defined(USE_S147LINK_DEV9_MEM_MMIO)
#define USE_S147LINK_DEV9_MEM_MMIO()
#endif

#endif /* __S147_MMIO_HWPORT__ */
