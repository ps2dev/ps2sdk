/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"

#include <iop_mmio_hwport.h>
#include <tamtypes.h>

struct RomImgData
{
	const void *ImageStart;
	const void *RomdirStart;
	const void *RomdirEnd;
};

struct RomDirEntry
{
	char name[10];
	u16 ExtInfoEntrySize;
	unsigned int size;
};

struct RomdirFileStat
{
	const struct RomDirEntry *romdirent;
	const void *data;
	const struct ExtInfoFieldEntry *extinfo;
};

struct ExtInfoFieldEntry
{
	// cppcheck-suppress unusedStructMember
	u16 value;
	u8 ExtLength;
	u8 type;
	u8 payload[];
};

#ifdef IGREETING_DTL_T
struct rominfo_item
{
	// cppcheck-suppress unusedStructMember
	const char *m_device_name;
	void *m_delay_register;
	int m_delay_register_value;
	void *m_base_address;
	int m_chunk_size;
	int m_config_offset_1;
	int m_config_offset_2;
	int m_flash_size;
	int m_manufacturer;
	int m_device_id;
	int m_write_width;
	int m_address_register;
	// cppcheck-suppress unusedStructMember
	int m_flash_offset;
	int m_flash_blocksize;
};

struct romflash_sigcheck_res
{
	void *m_config_addr_1;
	void *m_config_addr_2;
	void *m_base;
	void *m_base_end;
	int m_manufacturer_res;
	int m_device_id_res;
};
#endif

#define _mfc0(reg)                                                                                                     \
	({                                                                                                                   \
		u32 val;                                                                                                           \
		__asm__ volatile("mfc0 %0, " #reg : "=r"(val));                                                                    \
		val;                                                                                                               \
	})

#define mfc0(reg) _mfc0(reg)

#ifdef IGREETING_DTL_T
static void do_get_dip_switch_values_chr(char *str, int val, int count);
#endif
static struct RomImgData *GetIOPRPStat(const u32 *start_addr, const u32 *end_addr, struct RomImgData *rid);
static struct RomdirFileStat *
GetFileStatFromImage(const struct RomImgData *rid, const char *filename, struct RomdirFileStat *rdfs);
static const struct ExtInfoFieldEntry *
do_find_extinfo_entry(const struct RomdirFileStat *rdfs, unsigned int extinfo_type);
#ifdef IGREETING_DTL_T
static const struct rominfo_item *flash_probe(const struct rominfo_item *rii);
#endif

#ifdef IGREETING_DTL_T
static const char *boardinfo_uma_dsw202 =
	"\r\nUMA board DSW202\r\n  --- PS kernel --\r\n  Sw8  bit0   0^ use H1500,         1_ no use H1500 (CD-BOOT "
	"only)\r\n  Sw7  bit1   0^ display color bar  1_ no color bar\r\n  Sw6  bit2   0^ IOP Kernel         1_ PS Kernel "
	"(when PS mode)\r\n  Sw5  bit3   0^ check cd-rom       1_ ignore cdrom always\r\n  Sw4  bit4   0^ Dram 16M           "
	"1_ Dram 2M\r\n  Sw3  bit5   0^ disable            1_ enable EE ssbus access\r\n  --- IOP kernel --\r\n  Sw5  bit3   "
	"0^ Extr Wide DMA      1_ Extr Wide DMA disable\r\n  Sw4  bit4   0^ Dram 16M           1_ Dram 2M\r\n  Sw3  bit5   "
	"0^ disable            1_ enable EE ssbus access\r\n  --- EE --\r\n  Sw2  bit6   0^ --                 1_ --\r\n  "
	"Sw1  bit7   0^ EE normal boot     1_ EE/GS self test\r\n\r\n";
static const char *boardinfo_common =
	"  --- PS kernel --\r\n  D0   0^ use H1500,         1_ no use H1500 (CD-BOOT only)\r\n  D1   0^ display color bar  "
	"1_ no color bar\r\n  D2   0^ IOP Kernel         1_ PS Kernel (when PS mode)\r\n  D3   0^ check cd-rom       1_ "
	"ignore cdrom always\r\n  D4   0^ Dram 8M            1_ Dram 2M\r\n  D5   0^ disable            1_ enable EE ssbus "
	"access\r\n  --- IOP kernel --\r\n  D0   0^ NTSC mode          1_ PAL mode\r\n  D3   0^ Extr Wide DMA      1_ Extr "
	"Wide DMA disable\r\n  D4   0^ Dram 8M            1_ Dram 2M\r\n  D5   0^ disable            1_ enable EE ssbus "
	"access\r\n  --- EE --\r\n  D6   0^ --                 1_ --\r\n  D7   0^ EE normal boot     1_ EE/GS self "
	"test\r\n\r\n";
static const char *boardinfo_unknown = "\r\nUnknown board\r\n";
#endif
static const char *romgen_eq_str = " ROMGEN=";
#ifdef IGREETING_DTL_T
static const char *cpuid_eq_str = " CPUID=";
#else
static const char *cpuid_eq_str = ", IOP info (CPUID=";
#endif
static const char *cach_config_eq_str = ", CACH_CONFIG=";
#ifdef IGREETING_DTL_T
static struct rominfo_item default_list[] = {
	{"Gmain1-3, flash mode, Fujitsu 512K flash",
	 (void *)0xBF801010,
	 1256783,
	 (void *)0xBFC00000,
	 1,
	 1365,
	 682,
	 524288,
	 4,
	 164,
	 8,
	 0,
	 0,
	 0},
	{"Gmain4..., flash mode, Fujitsu 2M flash",
	 (void *)0xBF801010,
	 1387855,
	 (void *)0xBFC00000,
	 2,
	 2730,
	 1365,
	 2097152,
	 4,
	 196,
	 8,
	 0,
	 0,
	 0},
	{"Gmain4..., flash mode, AMD 2M flash",
	 (void *)0xBF801010,
	 1387855,
	 (void *)0xBFC00000,
	 2,
	 2730,
	 1365,
	 2097152,
	 1,
	 196,
	 8,
	 0,
	 0,
	 0},
	{"shimakawa sp, flash mode, Fujitsu 4M flash",
	 (void *)0xBF801010,
	 1453391,
	 (void *)0xBFC00000,
	 2,
	 2730,
	 1365,
	 4194304,
	 4,
	 95,
	 8,
	 0,
	 0,
	 0},
	{NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
#endif

int _start(int ac, char **av)
{
	int *boot_mode_4;
	int cop0_processor_mode;
	const char *iop_or_ps_mode_str;
	const struct ExtInfoFieldEntry *extinfo_entry;
	const char *extinfo_id_str;
	const char *comma_index;
	struct RomImgData rid;
	struct RomdirFileStat rdfs;
#ifdef IGREETING_DTL_T
	int *boot_mode_2;
	int *boot_mode_3;
	const char *board_type_str;
	const char *board_info_str;
	int boardtype_int;
	int board_has_wide_dma;
	const char *rom_or_flashrom_boot_str;
	u8 tmp_dma_wide_ch;
	char switch_values_str[16];
#endif
	USE_IOP_MMIO_HWPORT();

	(void)ac;
	(void)av;

	boot_mode_4 = QueryBootMode(4);
#ifndef IGREETING_DTL_T
	printf("\nPlayStation 2 ======== ");
#endif
	if ( boot_mode_4 )
	{
		switch ( *(u16 *)boot_mode_4 )
		{
#ifndef IGREETING_DTL_T
			case 0:
				printf("Hard reset boot");
				break;
#endif
			case 1:
				printf("Soft reboot");
				break;
			case 2:
				printf("Update rebooting..");
#ifdef IGREETING_DTL_T
				boot_mode_3 = QueryBootMode(3);
				if ( boot_mode_3 )
					printf(" reset parameter for IOP=%08x_%08x", boot_mode_3[2], boot_mode_3[1]);
#endif
				break;
			case 3:
				printf("Update reboot complete");
				break;
			default:
				break;
		}
		printf("\n");
	}
	if ( !boot_mode_4 || !*(u16 *)boot_mode_4 )
	{
		cop0_processor_mode = mfc0($15);
#ifdef IGREETING_DTL_T
		board_type_str = "";
		boardtype_int = iop_mmio_hwport->exp2_r2[4612];
		board_info_str = boardinfo_common;
		board_has_wide_dma = 1;
		switch ( boardtype_int & 0xF8 )
		{
			case 16:
				board_type_str = "\r\nGmain-1.0 board DSW602\r\n";
				break;
			case 32:
				board_type_str = "\r\nGmain-2.0 board DSW602\r\n";
				break;
			case 48:
				board_type_str = "\r\nGmain-3.0 board DSW602\r\n";
				break;
			case 64:
				board_type_str = "\r\nB3system-1.0 front dipsw\r\n";
				board_has_wide_dma = 0;
				break;
			case 80:
				board_type_str = "\r\nGmain-4.0 board DSW602\r\n";
				break;
			case 88:
				board_type_str = "\r\nGmain-5.0 board DSW602\r\n";
				break;
			case 96:
				board_type_str = "\r\nMPU-4.0 board DSW602\r\n";
				board_has_wide_dma = 0;
				break;
			case 112:
				board_type_str = "\r\nGmain-10.0/11.0 board DSW602\r\n";
				break;
			case 120:
				board_type_str = "\r\nGmain-12.0 board DSW602\r\n";
				break;
			case 124:
				board_type_str = "\r\nGmain-13.0 board DSW602\r\n";
				break;
			default:
				board_info_str = boardinfo_unknown;
				board_has_wide_dma = 0;
				break;
		}
		if ( boardtype_int < 2 )
		{
			board_info_str = boardinfo_uma_dsw202;
			board_has_wide_dma = (boardtype_int & 0xFE) == 0;
		}
		write(1, (char *)board_type_str, strlen(board_type_str));
		// Unofficial: make board info string not go through a structure
		write(1, (char *)board_info_str, strlen(board_info_str));
		write(1, (char *)cpuid_eq_str, strlen(cpuid_eq_str));
		printf("%x", cop0_processor_mode);
		write(1, (char *)romgen_eq_str, strlen(romgen_eq_str));
		printf("%04x-%04x", *(vu16 *)(0xBFC00102), *(vu16 *)(0xBFC00100));
#else
		write(1, (char *)romgen_eq_str, strlen(romgen_eq_str));
		printf("%04x-%04x", *(vu16 *)(0xBFC00102), *(vu16 *)(0xBFC00100));
		write(1, (char *)cpuid_eq_str, strlen(cpuid_eq_str));
		printf("%x", cop0_processor_mode);
#endif
		write(1, (char *)cach_config_eq_str, strlen(cach_config_eq_str));
		printf("%lx, %ldMB", (unsigned long)*(vu32 *)(0xFFFE0130), (long)((u32)QueryMemSize() + 256) >> 20);
		if ( cop0_processor_mode >= 16 )
		{
#ifdef IGREETING_DTL_T
			iop_or_ps_mode_str = ((iop_mmio_hwport->iop_sbus_ctrl[0] & 8) != 0) ? ", PS mode" : ", IOP mode";
#else
			iop_or_ps_mode_str = ((iop_mmio_hwport->iop_sbus_ctrl[0] & 8) != 0) ? ", PS mode)\r\n" : ", IOP mode)\r\n";
#endif
			write(1, (char *)iop_or_ps_mode_str, strlen(iop_or_ps_mode_str));
		}
#ifdef IGREETING_DTL_T
		CpuDisableIntr();
		rom_or_flashrom_boot_str = (flash_probe(default_list)) ? ", FlashROM boot\r\n" : ", ROM boot\r\n";
		CpuEnableIntr();
		write(1, (char *)rom_or_flashrom_boot_str, strlen(rom_or_flashrom_boot_str));
#endif
		// Unofficial: pad filename
		if (
			GetIOPRPStat((u32 *)0xBFC00000, (u32 *)0xBFC10000, &rid)
			&& GetFileStatFromImage(&rid, "ROMDIR\x00\x00\x00\x00\x00", &rdfs) )
		{
			extinfo_entry = do_find_extinfo_entry(&rdfs, 3);
			if ( extinfo_entry )
			{
				extinfo_id_str = (const char *)extinfo_entry->payload;
				comma_index = rindex(extinfo_id_str, ',');
				write(1, " <", 2);
				// Unofficial: if comma not found, just write the whole thing
				write(1, (char *)extinfo_id_str, comma_index ? (comma_index - extinfo_id_str) : (int)strlen(extinfo_id_str));
				printf(":%ld>\n", (long)(u16)(uiptr)extinfo_id_str);
			}
		}
#ifdef IGREETING_DTL_T
		write(1, " SW=b7:", 7);
		do_get_dip_switch_values_chr(switch_values_str, (u8)iop_mmio_hwport->exp2_r2[4352], 8);
		write(1, switch_values_str, 8);
		write(1, ":b0", 3);
		boot_mode_2 = QueryBootMode(2);
		if ( boot_mode_2 )
		{
			printf(", RESET parameter EE=%08x_%08x", boot_mode_2[2], boot_mode_2[1]);
			boot_mode_3 = QueryBootMode(3);
			if ( boot_mode_3 )
				printf(", IOP=%08x_%08x", boot_mode_3[2], boot_mode_3[1]);
		}
		write(1, "\r\n", 2);
		if ( board_has_wide_dma )
		{
			tmp_dma_wide_ch = iop_mmio_hwport->exp2_r2[4608];
			iop_mmio_hwport->exp2_r2[4608] = -1;
			printf(" DMA_WIDE_CH=%x\n", (u8)iop_mmio_hwport->exp2_r2[4608]);
			iop_mmio_hwport->exp2_r2[4608] = tmp_dma_wide_ch;
		}
		if ( (iop_mmio_hwport->exp2_r2[4352] & (1 << 5)) != 0 )
		{
			iop_mmio_hwport->iop_sbus_ctrl[0] |= 1;
		}
#endif
	}
	return 1;
}

#ifdef IGREETING_DTL_T
static void do_get_dip_switch_values_chr(char *str, int val, int count)
{
	int i;

	for ( i = 0; i < count; i += 1 )
	{
		str[i] = (((val >> ((count - i) - 1)) & 1) != 0) ? '_' : '^';
	}
}
#endif

static struct RomImgData *GetIOPRPStat(const u32 *start_addr, const u32 *end_addr, struct RomImgData *rid)
{
	unsigned int cur_offset;

	for ( cur_offset = 0; &start_addr[cur_offset >> 2] < end_addr; cur_offset += 16 )
	{
		if (
			start_addr[cur_offset >> 2] == 0x45534552 && start_addr[(cur_offset >> 2) + 1] == 0x54
			&& !(start_addr[(cur_offset >> 2) + 2] & 0xFFFF)
			&& ((start_addr[(cur_offset >> 2) + 3] + 15) & 0xFFFFFFF0) == cur_offset )
		{
			rid->ImageStart = start_addr;
			rid->RomdirStart = &start_addr[cur_offset >> 2];
			rid->RomdirEnd = (char *)&start_addr[cur_offset >> 2] + *(&start_addr[cur_offset >> 2] + 7);
			return rid;
		}
	}
	rid->ImageStart = 0;
	return 0;
}

static struct RomdirFileStat *
GetFileStatFromImage(const struct RomImgData *rid, const char *filename, struct RomdirFileStat *rdfs)
{
	int cur_addr_aligned;
	int total_extinfo_size;
	const struct RomDirEntry *RomdirStart;
	int i;
	char cur_romdir_name[12];

	cur_addr_aligned = 0;
	total_extinfo_size = 0;
	memset(cur_romdir_name, 0, sizeof(cur_romdir_name));
	for ( i = 0; i < 12 && filename[i] >= ' '; i += 1 )
	{
		cur_romdir_name[i] = filename[i];
	}
	for ( RomdirStart = (const struct RomDirEntry *)rid->RomdirStart;
				(u32 *)(RomdirStart->name) != (u32 *)(cur_romdir_name)
				&& (u32 *)(RomdirStart->name + 4) != (u32 *)(cur_romdir_name + 4)
				&& (u16 *)(RomdirStart->name + 8) != (u16 *)(cur_romdir_name + 8);
				RomdirStart += 1 )
	{
		cur_addr_aligned += ((RomdirStart->size) + 15) & 0xFFFFFFF0;
		total_extinfo_size += (s16)RomdirStart->ExtInfoEntrySize;
	}
	if ( !*(u32 *)RomdirStart->name )
		return 0;
	rdfs->romdirent = RomdirStart;
	rdfs->extinfo = RomdirStart->ExtInfoEntrySize ?
										(const struct ExtInfoFieldEntry *)((const char *)rid->RomdirEnd + total_extinfo_size) :
										0;
	rdfs->data = &((char *)rid->ImageStart)[cur_addr_aligned];
	return rdfs;
}

const struct ExtInfoFieldEntry *do_find_extinfo_entry(const struct RomdirFileStat *rdfs, unsigned int extinfo_type)
{
	const struct ExtInfoFieldEntry *extinfo;
	const struct ExtInfoFieldEntry *extinfo_end;

	extinfo = rdfs->extinfo;
	extinfo_end = &extinfo[(unsigned int)((s16)rdfs->romdirent->ExtInfoEntrySize) >> 2];
	for ( ; extinfo < extinfo_end;
				extinfo = (const struct ExtInfoFieldEntry *)((char *)extinfo + ((extinfo->ExtLength) & 0xFC) + 4) )
	{
		if ( extinfo->type == extinfo_type )
		{
			return extinfo;
		}
	}
	return 0;
}

#ifdef IGREETING_DTL_T
static void flash_reset(const struct rominfo_item *rii)
{
	CpuDisableIntr();
	switch ( rii->m_write_width )
	{
		case 8:
			*((vu8 *)rii->m_base_address + rii->m_config_offset_1) = 0xF0;
			break;
		case 16:
			*((vu16 *)rii->m_base_address + rii->m_config_offset_1) = 0xF0;
			break;
		default:
			break;
	}
	CpuEnableIntr();
}

static int checksig(const struct rominfo_item *rii, struct romflash_sigcheck_res *rsr, int offset)
{
	u8 *effective_address_1;
	u16 *effective_address_2;
	int tmp_manufacturer;
	USE_IOP_MMIO_HWPORT();

	// Unofficial: don't write to m_manufacturer
	tmp_manufacturer = rii->m_manufacturer;
	switch ( rii->m_write_width )
	{
		case 8:
			effective_address_1 = (u8 *)((u8 *)rii->m_base_address + offset);
			rsr->m_config_addr_1 = &effective_address_1[rii->m_config_offset_1];
			rsr->m_config_addr_2 = &effective_address_1[rii->m_config_offset_2];
			rsr->m_base = effective_address_1;
			rsr->m_base_end = &effective_address_1[rii->m_chunk_size];
			*(vu8 *)rsr->m_config_addr_1 = 0xAA;
			*(vu8 *)rsr->m_config_addr_2 = 0x55;
			*(vu8 *)rsr->m_config_addr_1 = 0x90;
			*(vu32 *)&iop_mmio_hwport->exp2_r2[4352] = 0xFFFFFF6F;
			rsr->m_manufacturer_res = *((vu8 *)rsr->m_base);
			rsr->m_device_id_res = *((vu8 *)rsr->m_base_end);
			break;
		case 16:
			effective_address_2 = (u16 *)((u8 *)rii->m_base_address + offset);
			rsr->m_config_addr_1 = &effective_address_2[rii->m_config_offset_1];
			rsr->m_config_addr_2 = &effective_address_2[rii->m_config_offset_2];
			rsr->m_base = effective_address_2;
			rsr->m_base_end = &effective_address_2[rii->m_chunk_size];
			*(vu16 *)rsr->m_config_addr_1 = 0xAA;
			*(vu16 *)rsr->m_config_addr_2 = 0x55;
			*(vu16 *)rsr->m_config_addr_1 = 0x90;
			*(vu32 *)&iop_mmio_hwport->exp2_r2[4352] = 0xFFFFFF6F;
			rsr->m_manufacturer_res = *((vu16 *)rsr->m_base);
			rsr->m_device_id_res = *((vu16 *)rsr->m_base_end);
			break;
		default:
			break;
	}
	flash_reset(rii);
	if ( !tmp_manufacturer && (rsr->m_manufacturer_res == 1 || rsr->m_manufacturer_res == 4) )
		tmp_manufacturer = rsr->m_manufacturer_res;
	return (rsr->m_manufacturer_res == tmp_manufacturer) && (rsr->m_device_id_res == rii->m_device_id);
}

static int flash_checksig(const struct rominfo_item *rii)
{
	int old_address_reg;
	int old_delay_reg;
	int condtmp1;
	int flash_chunks;
	int cur_flash_chunk;
	struct romflash_sigcheck_res rsr;
	USE_IOP_MMIO_HWPORT();

	memset(&rsr, 0, sizeof(rsr));
	old_address_reg = 0;
	old_delay_reg = 0;
	if ( rii->m_address_register )
	{
		old_address_reg = iop_mmio_hwport->ssbus2.ind_1_address;
		iop_mmio_hwport->ssbus2.ind_1_address = rii->m_address_register;
	}
	if ( rii->m_delay_register )
	{
		old_delay_reg = *((vu32 *)rii->m_delay_register);
		*((vu32 *)rii->m_delay_register) = rii->m_delay_register_value;
		*(vu32 *)&iop_mmio_hwport->exp2_r2[4352] = *(vu32 *)rii->m_delay_register;
	}
	flash_chunks = rii->m_flash_size / rii->m_flash_blocksize;
	for ( cur_flash_chunk = 0;
				(condtmp1 = checksig(rii, &rsr, flash_chunks * cur_flash_chunk)) && (cur_flash_chunk < rii->m_flash_blocksize);
				cur_flash_chunk += 1 )
	{
	}
	if ( !condtmp1 )
	{
		if ( rii->m_delay_register )
		{
			*((vu32 *)rii->m_delay_register) = old_delay_reg;
			*(vu32 *)&iop_mmio_hwport->exp2_r2[4352] = *(vu32 *)rii->m_delay_register;
		}
		if ( rii->m_address_register )
			iop_mmio_hwport->ssbus2.ind_1_address = old_address_reg;
	}
	return condtmp1;
}

static const struct rominfo_item *flash_probe(const struct rominfo_item *rii)
{
	CpuDisableIntr();
	for ( ; rii->m_write_width; rii += 1 )
	{
		if ( flash_checksig(rii) )
		{
			CpuEnableIntr();
			return rii;
		}
	}
	CpuEnableIntr();
	return 0;
}
#endif
