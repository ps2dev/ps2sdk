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
 * Interface for System 147 NAND flash.
 */

#ifndef _S147NAND_H
#define _S147NAND_H

#include <tamtypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct s147nand_info_
	{
		int m_page_size_noecc;
		int m_page_size_withecc;
		int m_pages_per_block;
		int m_block_size;
		int m_page_count;
	} s147nand_info_t;

	typedef struct s147nand_header_part_
	{
		int m_offset;
		int m_size;
	} s147nand_header_part_t;

	typedef struct s147nand_header_
	{
		char m_sig[16];
		int m_bootsector_ver_1;
		int m_bootsector_ver_2;
		s147nand_header_part_t m_nand_partition_info[8];
		char m_nand_seccode[2];
		char pad[2];
		int m_nand_vidmode[1];
		char m_nand_desc[32];
		int m_page_size_noecc;
		int m_page_size_withecc;
		int m_pages_per_block;
		int m_block_size;
		unsigned int m_acmem_delay_val;
		unsigned int m_acio_delay_val;
		s147nand_header_part_t m_nand_partition_8_info;
	} s147nand_header_t;

	typedef struct s147nand_direntry_
	{
		char m_name[16];
		u32 m_unk;
		u8 m_type;
		u8 m_pad[3];
		u32 m_size;
		u32 m_offset;
	} s147nand_direntry_t;

	typedef struct s147nand_dir_
	{
		char m_sig[8];
		u16 m_ver;
		char m_unk0[6];
		u32 m_entrycnt;
		u32 m_unk1;
		u32 m_unk2;
		u32 m_unk3;
		s147nand_direntry_t m_direntry[63];
	} s147nand_dir_t;

	extern int s147nand_4_dumpprintinfo(int part);
	extern int s147nand_5_outerinit(void);
	extern void s147nand_6_checkformat(void);
	extern int s147nand_7_multi_read_dma(void *ptr, int pageoffs, int pagecnt);
	extern int s147nand_8_multi_write_dma(void *ptr, int pageoffs, int pagecnt);
	extern int s147nand_9_get_nand_partition(int part);
	extern int s147nand_10_get_nand_partition_size(int part);
	extern int s147nand_11_erasetranslatepageoffs(int pageoffs);
	extern int s147nand_12_load_logaddrtable(void);
	extern int s147nand_13_translate_blockoffs(int blockoffs);
	extern int s147nand_14_translate_pageoffs(int pageoffs);
	extern int s147nand_15_nandinit(void);
	extern s147nand_info_t *s147nand_16_getnandinfo(void);
	extern int s147nand_17_get_sema(void);
	extern void s147nand_18_enable_nand_watchdog(void);
	extern int s147nand_19_logaddr_read(u16 *tbl, int pageoffs, int bytecnt);
	extern int s147nand_20_nand_read_dma(void *ptr, int pageoffs, int byteoffs, int bytecnt);
	extern int s147nand_21_nand_read_pio(void *ptr, int pageoffs, int byteoffs, int bytecnt);
	extern int s147nand_22_nand_write_dma(void *ptr, int pageoffs, int byteoffs, int bytecnt);
	extern int s147nand_23_nand_write_pio(void *ptr, int pageoffs, int byteoffs, int bytecnt);
	extern int s147nand_24_eraseoffset(int pageoffs);
	extern int s147nand_25_nand_blockerase(int pageoffs);
	extern int s147nand_26_nand_readid(void *ptr);
	extern int s147nand_27_blocks2pages(int blocks);
	extern int s147nand_28_pages2blocks(int pages);
	extern int s147nand_29_pages2blockround(int pages);
	extern int s147nand_30_bytes2pagesnoeccround(int bytes);

#define s147nand_IMPORTS_start DECLARE_IMPORT_TABLE(s147nand, 4, 2)
#define s147nand_IMPORTS_end END_IMPORT_TABLE

#define I_s147nand_4_dumpprintinfo DECLARE_IMPORT(4, s147nand_4_dumpprintinfo)
#define I_s147nand_5_outerinit DECLARE_IMPORT(5, s147nand_5_outerinit)
#define I_s147nand_6_checkformat DECLARE_IMPORT(6, s147nand_6_checkformat)
#define I_s147nand_7_multi_read_dma DECLARE_IMPORT(7, s147nand_7_multi_read_dma)
#define I_s147nand_8_multi_write_dma DECLARE_IMPORT(8, s147nand_8_multi_write_dma)
#define I_s147nand_9_get_nand_partition DECLARE_IMPORT(9, s147nand_9_get_nand_partition)
#define I_s147nand_10_get_nand_partition_size DECLARE_IMPORT(10, s147nand_10_get_nand_partition_size)
#define I_s147nand_11_erasetranslatepageoffs DECLARE_IMPORT(11, s147nand_11_erasetranslatepageoffs)
#define I_s147nand_12_load_logaddrtable DECLARE_IMPORT(12, s147nand_12_load_logaddrtable)
#define I_s147nand_13_translate_blockoffs DECLARE_IMPORT(13, s147nand_13_translate_blockoffs)
#define I_s147nand_14_translate_pageoffs DECLARE_IMPORT(14, s147nand_14_translate_pageoffs)
#define I_s147nand_15_nandinit DECLARE_IMPORT(15, s147nand_15_nandinit)
#define I_s147nand_16_getnandinfo DECLARE_IMPORT(16, s147nand_16_getnandinfo)
#define I_s147nand_17_get_sema DECLARE_IMPORT(17, s147nand_17_get_sema)
#define I_s147nand_18_enable_nand_watchdog DECLARE_IMPORT(18, s147nand_18_enable_nand_watchdog)
#define I_s147nand_19_logaddr_read DECLARE_IMPORT(19, s147nand_19_logaddr_read)
#define I_s147nand_20_nand_read_dma DECLARE_IMPORT(20, s147nand_20_nand_read_dma)
#define I_s147nand_21_nand_read_pio DECLARE_IMPORT(21, s147nand_21_nand_read_pio)
#define I_s147nand_22_nand_write_dma DECLARE_IMPORT(22, s147nand_22_nand_write_dma)
#define I_s147nand_23_nand_write_pio DECLARE_IMPORT(23, s147nand_23_nand_write_pio)
#define I_s147nand_24_eraseoffset DECLARE_IMPORT(24, s147nand_24_eraseoffset)
#define I_s147nand_25_nand_blockerase DECLARE_IMPORT(25, s147nand_25_nand_blockerase)
#define I_s147nand_26_nand_readid DECLARE_IMPORT(26, s147nand_26_nand_readid)
#define I_s147nand_27_blocks2pages DECLARE_IMPORT(27, s147nand_27_blocks2pages)
#define I_s147nand_28_pages2blocks DECLARE_IMPORT(28, s147nand_28_pages2blocks)
#define I_s147nand_29_pages2blockround DECLARE_IMPORT(29, s147nand_29_pages2blockround)
#define I_s147nand_30_bytes2pagesnoeccround DECLARE_IMPORT(30, s147nand_30_bytes2pagesnoeccround)

#ifdef __cplusplus
}
#endif

#endif
