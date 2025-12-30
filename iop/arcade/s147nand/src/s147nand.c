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
#include <errno.h>
#include <iop_mmio_hwport.h>
#include <s147_mmio_hwport.h>
#include <s147nand.h>
#include <sys/fcntl.h>

IRX_ID("S147NAN2", 5, 2);
// Text section hash:
// 7894f2d18e733e5f2422b6c61af3b0da

typedef struct s147nand_mdev_privdata_
{
	int m_seek_cur;
	int m_flags;
	int m_seek_max;
	int m_partition_offset;
} s147nand_mdev_privdata_t;

static int do_register_nand_to_mdev(const char *drv_name, const char *drv_desc);
static int nand_mdev_op_init(iop_device_t *dev);
static int nand_mdev_op_deinit(iop_device_t *dev);
static int nand_mdev_op_open(iop_file_t *f, const char *name, int flags);
static int nand_mdev_op_close(iop_file_t *f);
static int nand_mdev_op_read(iop_file_t *f, void *ptr, int size);
static int nand_mdev_op_write(iop_file_t *f, void *ptr, int size);
static int nand_mdev_op_lseek(iop_file_t *f, int offset, int mode);
static int do_nand_open_inner1(s147nand_mdev_privdata_t *privdat, int part, const char *name);
static int do_nand_open_inner2(s147nand_mdev_privdata_t *privdat, const char *name);
static u32 do_get_nand_direntry(s147nand_mdev_privdata_t *privdat, const char *name, int idx, char typ);
static int do_nand_bytes2sector(int pageoffs, int byteoffs);
static int do_nand_bytes2sector_remainder(int byteoffs);
static int do_nand_sector_rw(void *ptr, int pageoffs, int byteoffs, int size);
static int get_nand_partition_offset(int part);
static int nand_mdev_open_special(iop_file_t *f, const char *name);
static int nand_mdev_read_special(iop_file_t *f, void *ptr, int size);
static int nand_mdev_write_special(iop_file_t *f, void *ptr, int size);
static int do_nand_copy_seccode_from_buf(iop_file_t *f, void *ptr, int size);
static int do_nand_copy_videomode_from_buf(iop_file_t *f, void *ptr, int size);
static int nand_lowlevel_read_dma(void *ptr, int pageoffs, int byteoffs, int bytecnt);
static int nand_lowlevel_read_pio(void *ptr, int pageoffs, int byteoffs, int bytecnt);
static int nand_lowlevel_write_dma(void *ptr, int pageoffs, int byteoffs, int bytecnt);
static int nand_lowlevel_write_pio(void *ptr, int pageoffs, int byteoffs, int bytecnt);
static int nand_lowlevel_blockerase(int pageoffs);
static int nand_lowlevel_readid(void *ptr);

extern struct irx_export_table _exp_s147nand;

IOMAN_RETURN_VALUE_IMPL(0);

static iop_device_ops_t nand_mdev_ops = {
	&nand_mdev_op_init,
	&nand_mdev_op_deinit,
	IOMAN_RETURN_VALUE(0),
	&nand_mdev_op_open,
	&nand_mdev_op_close,
	&nand_mdev_op_read,
	&nand_mdev_op_write,
	&nand_mdev_op_lseek,
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
	IOMAN_RETURN_VALUE(0),
};
// Unofficial: move to bss
static void *g_nand_unaligned_buf;
// Unofficial: move to bss
static int g_nand_watchdog_enabled;
// Unofficial: move to bss
static s147nand_info_t g_nand_info;
static iop_device_t g_drv;
static void *g_nand_sector_buffer;
static int g_sema_id_dev;
static int g_sema_id_init;
static s147nand_header_t g_nand_header __attribute__((__aligned__(16)));
static u16 *g_logical_addr_tbl;
static int g_sema_id_nand;
static int g_thid;

int _start(int ac, char **av)
{
	(void)ac;
	(void)av;
	Kprintf("\ns147nand.irx: System147 NAND-Flash File System Driver v%d.%d\n", 5, 2);
	// Unofficial: init vars here instead
	g_nand_info.m_page_size_noecc = 0x800;
	g_nand_info.m_page_size_withecc = 0x840;
	g_nand_info.m_pages_per_block = 0x40;
	g_nand_info.m_block_size = 0x800;
	g_nand_info.m_page_count = 0x20000;
	if ( do_register_nand_to_mdev("nand", "NAND-Flash") )
		return MODULE_NO_RESIDENT_END;
	if ( RegisterLibraryEntries(&_exp_s147nand) )
	{
		Kprintf("s147nand.irx: RegisterLibraryEntries - Failed.\n");
		return MODULE_NO_RESIDENT_END;
	}
	Kprintf("s147nand.irx: RegisterLibraryEntries - OK.\n");
	return MODULE_RESIDENT_END;
}

static int do_register_nand_to_mdev(const char *drv_name, const char *drv_desc)
{
	iop_sema_t semaparam;

	if ( s147nand_5_outerinit() < 0 )
		return -1;
	// Unofficial: make semaparam local var
	semaparam.initial = 1;
	semaparam.max = 1;
	semaparam.attr = SA_THPRI;
	semaparam.option = 0;
	g_sema_id_dev = CreateSema(&semaparam);
	if ( g_sema_id_dev < 0 )
	{
		Kprintf("s147nand.irx: CreateSema error (%d)\n", g_sema_id_dev);
		return -1;
	}
	g_drv.name = drv_name;
	g_drv.type = IOP_DT_FS;
	g_drv.version = 0;
	g_drv.desc = drv_desc;
	g_drv.ops = &nand_mdev_ops;
	s147mdev_5_delfs(0);
	s147mdev_4_addfs(&g_drv, 0);
	// Unofficial: remove redundant var
	return 0;
}

static int nand_mdev_op_init(iop_device_t *dev)
{
	int state;

	(void)dev;
	Kprintf("s147nand.irx: SectorBuffer=%d, FileParam=%d\n", 0, 1);
	CpuSuspendIntr(&state);
	g_nand_sector_buffer = AllocSysMemory(ALLOC_FIRST, 2048, 0);
	CpuResumeIntr(state);
	if ( g_nand_sector_buffer )
		return 0;
	Kprintf("s147nand.irx: AllocSysMemory failed (s147file_Init)\n");
	return -ENOMEM;
}

static int nand_mdev_op_deinit(iop_device_t *dev)
{
	int state;

	(void)dev;
	if ( g_nand_sector_buffer )
	{
		CpuSuspendIntr(&state);
		FreeSysMemory(g_nand_sector_buffer);
		CpuResumeIntr(state);
	}
	return 0;
}

static int nand_mdev_op_open(iop_file_t *f, const char *name, int flags)
{
	s147nand_mdev_privdata_t *privdat;
	int state;
	int retres;

	(void)flags;
	retres = 0;
	WaitSema(g_sema_id_dev);
	if ( f->unit == 9 )
	{
		retres = nand_mdev_open_special(f, name);
		SignalSema(g_sema_id_dev);
		return retres;
	}
	CpuSuspendIntr(&state);
	f->privdata = AllocSysMemory(ALLOC_LAST, sizeof(s147nand_mdev_privdata_t), 0);
	CpuResumeIntr(state);
	if ( !f->privdata )
	{
		Kprintf("s147nand.irx: AllocSysMemory failed (Open)\n");
		retres = -ENOMEM;
	}
	if ( retres >= 0 )
	{
		privdat = (s147nand_mdev_privdata_t *)f->privdata;
		memset(privdat, 0, sizeof(s147nand_mdev_privdata_t));
		retres = do_nand_open_inner1(privdat, f->unit, name);
	}
	if ( retres < 0 )
	{
		// Unofficial: check if not NULL
		if ( f->privdata )
		{
			CpuSuspendIntr(&state);
			FreeSysMemory(f->privdata);
			CpuResumeIntr(state);
		}
		SignalSema(g_sema_id_dev);
		return retres;
	}
	privdat->m_seek_cur = 0;
	SignalSema(g_sema_id_dev);
	return 0;
}

static int nand_mdev_op_close(iop_file_t *f)
{
	int state;

	WaitSema(g_sema_id_dev);
	if ( f->privdata )
	{
		CpuSuspendIntr(&state);
		FreeSysMemory(f->privdata);
		CpuResumeIntr(state);
		f->privdata = 0;
	}
	SignalSema(g_sema_id_dev);
	return 0;
}

static int nand_mdev_op_read(iop_file_t *f, void *ptr, int size)
{
	int xsize3;
	s147nand_mdev_privdata_t *privdat;
	int xsector1;
	int xsector2;
	int xsize2;
	int cursz;

	privdat = (s147nand_mdev_privdata_t *)f->privdata;
	WaitSema(g_sema_id_dev);
	if ( privdat->m_seek_cur >= privdat->m_seek_max )
	{
		SignalSema(g_sema_id_dev);
		return 0;
	}
	xsize2 = (privdat->m_seek_max >= privdat->m_seek_cur + size) ? size : (privdat->m_seek_max - privdat->m_seek_cur);
	if ( privdat->m_seek_cur < 0 || privdat->m_seek_max < 0 )
		Kprintf(
			"s147file: CallBack_Read -> Param=0x%08x Seek=%d/%d Count=%d CountEnd=%d\n",
			privdat,
			privdat->m_seek_cur,
			privdat->m_seek_max,
			size,
			xsize2);
	if ( f->unit == 9 )
	{
		int special;

		special = nand_mdev_read_special(f, ptr, size);
		SignalSema(g_sema_id_dev);
		return special;
	}
	xsector1 = do_nand_bytes2sector(privdat->m_partition_offset, privdat->m_seek_cur + xsize2);
	xsector2 = do_nand_bytes2sector_remainder(privdat->m_seek_cur + xsize2);
	for ( cursz = 0; cursz < xsize2; cursz += xsize3 )
	{
		int pageoffs;

		pageoffs = do_nand_bytes2sector(privdat->m_partition_offset, privdat->m_seek_cur);
		xsize3 = ((pageoffs >= xsector1) ? xsector2 : 2048) - do_nand_bytes2sector_remainder(privdat->m_seek_cur);
		do_nand_sector_rw(((char *)ptr) + cursz, pageoffs, do_nand_bytes2sector_remainder(privdat->m_seek_cur), xsize3);
		privdat->m_seek_cur += xsize3;
	}
	SignalSema(g_sema_id_dev);
	return xsize2;
}

static int nand_mdev_op_write(iop_file_t *f, void *ptr, int size)
{
	WaitSema(g_sema_id_dev);
	if ( f->unit == 9 )
	{
		int retres;

		retres = nand_mdev_write_special(f, ptr, size);
		SignalSema(g_sema_id_dev);
		return retres;
	}
	SignalSema(g_sema_id_dev);
	return size;
}

static int nand_mdev_op_lseek(iop_file_t *f, int offset, int mode)
{
	s147nand_mdev_privdata_t *privdat;

	privdat = (s147nand_mdev_privdata_t *)f->privdata;
	WaitSema(g_sema_id_dev);
	switch ( mode )
	{
		case SEEK_SET:
			privdat->m_seek_cur = offset;
			break;
		case SEEK_CUR:
			privdat->m_seek_cur += offset;
			break;
		case SEEK_END:
			privdat->m_seek_cur = privdat->m_seek_max + offset;
			break;
		default:
			SignalSema(g_sema_id_dev);
			return -EINVAL;
	}
	if ( privdat->m_seek_max < privdat->m_seek_cur )
	{
		Kprintf("s147nand.irx: Out of range (seek=%d, filesize=%d)\n", privdat->m_seek_cur, privdat->m_seek_max);
		privdat->m_seek_cur = privdat->m_seek_max;
		SignalSema(g_sema_id_dev);
		return -EINVAL;
	}
	SignalSema(g_sema_id_dev);
	return privdat->m_seek_cur;
}

int s147nand_4_dumpprintinfo(int part)
{
	int hdrret;
	int nand_partition_offset;
	int i;
	int j;
	int dircnt;
	int filcnt;
	int finished;
	char pathtmp[18];

	hdrret = -1;
	dircnt = 0;
	filcnt = 0;
	finished = 0;
	nand_partition_offset = get_nand_partition_offset(part);
	if ( nand_partition_offset < 0 )
		return -ENODEV;
	for ( i = 0; i < 64; i += 1 )
	{
		s147nand_7_multi_read_dma(g_nand_sector_buffer, nand_partition_offset + i, 1);
		for ( j = 0; j < 64; j += 1 )
		{
			if ( (i << 6) - 1 + j == -1 )
			{
				const s147nand_dir_t *hdrbuf;

				hdrbuf = (const s147nand_dir_t *)g_nand_sector_buffer;
				if ( strncmp(hdrbuf->m_sig, "S147ROM", 8) )
				{
					// Unofficial: use g_drv.name
					Kprintf(" \"%s%d:\" ... No data\n", g_drv.name, part);
					Kprintf(" -----------------------------\n\n");
					return -ENODEV;
				}
				hdrret = hdrbuf->m_entrycnt;
				// Unofficial: use g_drv.name
				Kprintf(" \"%s%d:\"\n", g_drv.name, part);
				Kprintf(" -----------------------------\n");
			}
			else
			{
				const s147nand_direntry_t *dirbuf;

				dirbuf = (const s147nand_direntry_t *)g_nand_sector_buffer;
				if ( (i << 6) - 1 + j >= hdrret )
				{
					finished = 1;
					break;
				}
				strcpy(pathtmp, dirbuf[j].m_name);
				if ( dirbuf[j].m_type == 'D' )
				{
					strcat(pathtmp, "/");
					dircnt += 1;
				}
				else
				{
					filcnt += 1;
				}
				Kprintf(" %9d  %s\n", dirbuf[j].m_size, pathtmp);
			}
		}
		if ( finished )
			break;
	}
	Kprintf(" -----------------------------\n");
	Kprintf("   %d directories, %d files\n", dircnt, filcnt);
	Kprintf("\n");
	return hdrret;
}

static int do_nand_open_inner1(s147nand_mdev_privdata_t *privdat, int part, const char *name)
{
	int nand_partition_offset;

	nand_partition_offset = get_nand_partition_offset(part);
	if ( nand_partition_offset < 0 )
	{
		Kprintf("s147nand.irx: Error invalid unit number\n");
		return -ENODEV;
	}
	privdat->m_partition_offset = nand_partition_offset;
	return do_nand_open_inner2(privdat, name + ((name[0] == '/') ? 1 : 0));
}

static int do_nand_open_inner2(s147nand_mdev_privdata_t *privdat, const char *name)
{
	int i;

	for ( i = 0; name[i] && name[i] != '/'; i += 1 )
		;
	if ( name[i] == '/' )
	{
		int nand_direntry;

		nand_direntry = do_get_nand_direntry(privdat, name, i, 'D');
		return (nand_direntry >= 0) ? do_nand_open_inner2(privdat, &name[i + 1]) : nand_direntry;
	}
	return do_get_nand_direntry(privdat, name, i, 'F');
}

static u32 do_get_nand_direntry(s147nand_mdev_privdata_t *privdat, const char *name, int idx, char typ)
{
	int lvtyp;
	int size;
	int hdrret;
	int offscnt;
	int i;
	char name_trunc[18];

	hdrret = -1;
	size = (idx > (int)(sizeof(name_trunc) - 2)) ? (int)(sizeof(name_trunc) - 2) : idx;
	strncpy(name_trunc, name, size);
	name_trunc[size] = 0;
	for ( offscnt = 0; offscnt < 64; offscnt += 1 )
	{
		s147nand_7_multi_read_dma(g_nand_sector_buffer, privdat->m_partition_offset + offscnt, 1);

		for ( i = 0; i < 64; i += 1 )
		{
			if ( (offscnt << 6) - 1 + i == -1 )
			{
				const s147nand_dir_t *hdrbuf;

				hdrbuf = (const s147nand_dir_t *)g_nand_sector_buffer;
				if ( strncmp(hdrbuf->m_sig, "S147ROM", 8) )
				{
					Kprintf("s147nand.irx: No directory entries\n");
					return -ENODEV;
				}
				if ( hdrbuf->m_ver >= 0x101 )
				{
					Kprintf("s147nand.irx: Version 0x%04x format is not supported\n", hdrbuf->m_ver);
					return -ENODEV;
				}
				hdrret = hdrbuf->m_entrycnt;
			}
			else
			{
				const s147nand_direntry_t *dirbuf;

				dirbuf = (const s147nand_direntry_t *)g_nand_sector_buffer;
				if ( (offscnt << 6) - 1 + i >= hdrret )
					return -ENOENT;
				lvtyp = (char)dirbuf[i].m_type;
				if (
					(((lvtyp == 'D') && typ == 'D') || ((lvtyp == 'F' || lvtyp == '\x00') && typ == 'F'))
					&& (!strcmp(dirbuf[i].m_name, name_trunc)) )
				{
					privdat->m_seek_max = dirbuf[i].m_size;
					privdat->m_partition_offset += dirbuf[i].m_offset;
					return dirbuf[i].m_size;
				}
			}
		}
	}
	return -ENOENT;
}

static int do_nand_bytes2sector(int pageoffs, int byteoffs)
{
	return pageoffs + byteoffs / 2048;
}

static int do_nand_bytes2sector_remainder(int byteoffs)
{
	return byteoffs % 2048;
}

int s147nand_5_outerinit(void)
{
	int initres;
	iop_sema_t semaparam;

	initres = s147nand_15_nandinit();
	if ( initres )
	{
		Kprintf("s147nand.irx: NAND initialize failed (%d)\n", initres);
		return -1;
	}
	// Unofficial: make semaparam local var
	semaparam.initial = 1;
	semaparam.max = 1;
	semaparam.attr = SA_THPRI;
	semaparam.option = 0;
	g_sema_id_init = CreateSema(&semaparam);
	if ( g_sema_id_init < 0 )
	{
		Kprintf("s147nand.irx: CreateSema error (%d)\n", g_sema_id_init);
		return -1;
	}
	s147nand_6_checkformat();
	return 0;
}

void s147nand_6_checkformat(void)
{
	int state;
	s147nand_info_t *nandinf;

	nandinf = s147nand_16_getnandinfo();
	s147nand_20_nand_read_dma(&g_nand_header, 0, 0, sizeof(g_nand_header));
	if ( strncmp(g_nand_header.m_sig, "S147NAND", 9) )
	{
		Kprintf("s147nand.irx: Unformatted device\n");
		Kprintf("\n");
		return;
	}
	Kprintf(
		"s147nand.irx: BootSector format version = %d.%d\n",
		g_nand_header.m_bootsector_ver_1,
		g_nand_header.m_bootsector_ver_2);
	if ( (u32)g_nand_header.m_bootsector_ver_1 < 2 )
	{
		Kprintf("s147nand.irx: Old version format, 256MB-NAND only\n");
	}
	else
	{
		Kprintf("s147nand.irx: %-.32s\n", g_nand_header.m_nand_desc);
		CpuSuspendIntr(&state);
		nandinf->m_page_size_noecc = g_nand_header.m_page_size_noecc;
		nandinf->m_page_size_withecc = g_nand_header.m_page_size_withecc;
		nandinf->m_pages_per_block = g_nand_header.m_pages_per_block;
		nandinf->m_block_size = g_nand_header.m_block_size;
		nandinf->m_page_count = g_nand_header.m_block_size * g_nand_header.m_pages_per_block;
		CpuResumeIntr(state);
	}
	Kprintf(
		"s147nand.irx: PageSize    = %d + %d (Bytes)\n",
		nandinf->m_page_size_noecc,
		nandinf->m_page_size_withecc - nandinf->m_page_size_noecc);
	Kprintf("s147nand.irx: Pages/Block = %d (Pages)\n", nandinf->m_pages_per_block);
	Kprintf("s147nand.irx: BlockSize   = %d (Blocks)\n", nandinf->m_block_size);
	Kprintf("s147nand.irx: PageSize    = %d (Pages)\n", nandinf->m_page_count);
	Kprintf("\n");
}

static void do_update_acdelay(void)
{
	int state;

	Kprintf("s147nand.irx: Update Acdelay\n", g_nand_header.m_bootsector_ver_1, g_nand_header.m_bootsector_ver_2);
	DelayThread(10000);
	if ( (u32)g_nand_header.m_bootsector_ver_1 < 2 )
	{
		Kprintf("s147nand.irx: Old version format, no update\n");
		DelayThread(10000);
		return;
	}
	if ( !g_nand_header.m_acmem_delay_val || (int)g_nand_header.m_acmem_delay_val == -1 )
	{
		Kprintf("s147nand.irx: AcMem = 0x%08x (Default)\n", g_nand_header.m_acmem_delay_val);
	}
	else
	{
		CpuSuspendIntr(&state);
		SetAcMemDelayReg(g_nand_header.m_acmem_delay_val);
		CpuResumeIntr(state);
		Kprintf(
			"s147nand.irx: AcMem = 0x%08x (DMA=%d, Read=%d, Write=%d)\n",
			g_nand_header.m_acmem_delay_val,
			((g_nand_header.m_acmem_delay_val & 0xF000000) >> 24) + 1,
			((u8)(g_nand_header.m_acmem_delay_val & 0xF0) >> 4) + 1,
			(g_nand_header.m_acmem_delay_val & 0xF) + 1);
	}
	DelayThread(10000);
	if ( !g_nand_header.m_acio_delay_val || (int)g_nand_header.m_acio_delay_val == -1 )
	{
		Kprintf("s147nand.irx: AcIo  = 0x%08x (Default)\n", g_nand_header.m_acio_delay_val);
	}
	else
	{
		CpuSuspendIntr(&state);
		SetAcIoDelayReg(g_nand_header.m_acio_delay_val);
		CpuResumeIntr(state);
		Kprintf(
			"s147nand.irx: AcIo  = 0x%08x (DMA=%d, Read=%d, Write=%d)\n",
			g_nand_header.m_acio_delay_val,
			((g_nand_header.m_acio_delay_val & 0xF000000) >> 24) + 1,
			((u8)(g_nand_header.m_acio_delay_val & 0xF0) >> 4) + 1,
			(g_nand_header.m_acio_delay_val & 0xF) + 1);
	}
	DelayThread(10000);
	Kprintf("\n");
	DelayThread(10000);
}

static int do_nand_sector_rw(void *ptr, int pageoffs, int byteoffs, int size)
{
	int dma;

	WaitSema(g_sema_id_init);
	if ( ((uiptr)ptr & 3) != 0 || (byteoffs & 3) != 0 || (size & 3) != 0 )
	{
		dma = s147nand_20_nand_read_dma(
			g_nand_unaligned_buf, s147nand_14_translate_pageoffs(pageoffs), 0, s147nand_16_getnandinfo()->m_page_size_noecc);
		memcpy(ptr, (char *)g_nand_unaligned_buf + byteoffs, size);
	}
	else
	{
		dma = s147nand_20_nand_read_dma(ptr, s147nand_14_translate_pageoffs(pageoffs), byteoffs, size);
	}
	SignalSema(g_sema_id_init);
	return dma;
}

int s147nand_7_multi_read_dma(void *ptr, int pageoffs, int pagecnt)
{
	int i;
	int retres;

	// Unofficial: initialize retres
	retres = 0;
	WaitSema(g_sema_id_init);
	for ( i = 0; i < pagecnt; i += 1 )
	{
		retres = s147nand_20_nand_read_dma(
			(char *)ptr + ((s147nand_16_getnandinfo()->m_page_size_noecc >> 2) << 2) * i,
			s147nand_14_translate_pageoffs(pageoffs + i),
			0,
			s147nand_16_getnandinfo()->m_page_size_noecc);
		if ( retres )
			return retres;
	}
	SignalSema(g_sema_id_init);
	return retres;
}

int s147nand_8_multi_write_dma(void *ptr, int pageoffs, int pagecnt)
{
	int i;
	int retres;

	// Unofficial: initialize retres
	retres = 0;
	for ( i = 0; i < pagecnt; i += 1 )
	{
		retres = s147nand_22_nand_write_dma(
			(char *)ptr + ((s147nand_16_getnandinfo()->m_page_size_noecc >> 2) << 2) * i,
			s147nand_14_translate_pageoffs(pageoffs + i),
			0,
			s147nand_16_getnandinfo()->m_page_size_noecc);
		if ( retres )
			return retres;
	}
	return retres;
}

int s147nand_9_get_nand_partition(int part)
{
	if ( part == 8 )
		return g_nand_header.m_nand_partition_8_info.m_offset;
	if ( part >= 0 && part < 8 )
		return g_nand_header.m_nand_partition_info[part].m_offset;
	return 0;
}

static int get_nand_partition_offset(int part)
{
	return s147nand_9_get_nand_partition(part) * g_nand_header.m_pages_per_block;
}

int s147nand_10_get_nand_partition_size(int part)
{
	if ( part == 8 )
		return g_nand_header.m_nand_partition_8_info.m_size;
	if ( part >= 0 && part < 8 )
		return g_nand_header.m_nand_partition_info[part].m_size;
	return 0;
}

static int nand_mdev_open_special(iop_file_t *f, const char *name)
{
	s147nand_mdev_privdata_t *privdat;
	int state;

	CpuSuspendIntr(&state);
	f->privdata = AllocSysMemory(ALLOC_FIRST, sizeof(s147nand_mdev_privdata_t), 0);
	CpuResumeIntr(state);
	if ( !f->privdata )
	{
		Kprintf("s147nand.irx: AllocSysMemory failed (9:Open)\n");
		// Unofficial: return early on error
		return -ENOENT;
	}
	privdat = (s147nand_mdev_privdata_t *)f->privdata;
	memset(privdat, 0, sizeof(s147nand_mdev_privdata_t));
	privdat->m_seek_cur = 0;
	if ( !strcmp(name, "watchdog-enable") )
	{
		s147nand_18_enable_nand_watchdog();
		privdat->m_flags = 0;
		privdat->m_seek_max = 0;
		privdat->m_partition_offset = 0;
		privdat->m_seek_cur = 0;
		Kprintf("s147nand.irx: WatchDogTimer Enable\n");
		DelayThread(10000);
	}
	else if ( !strcmp(name, "acdelay") )
	{
		do_update_acdelay();
		privdat->m_flags = 0;
		privdat->m_seek_max = 0;
		privdat->m_partition_offset = 0;
		privdat->m_seek_cur = 0;
	}
	else if ( !strcmp(name, "seccode") )
	{
		privdat->m_flags = 0x10000;
		privdat->m_seek_max = 2;
		privdat->m_partition_offset = 0;
		privdat->m_seek_cur = 0;
	}
	else if ( !strcmp(name, "videomode") )
	{
		privdat->m_flags = 0x40000;
		privdat->m_seek_max = 4;
		privdat->m_partition_offset = 0;
		privdat->m_seek_cur = 0;
	}
	else if ( !strcmp(name, "info") )
	{
		privdat->m_flags = 0x100000;
		privdat->m_seek_max = 2048;
		privdat->m_partition_offset = (s147nand_9_get_nand_partition(8) - 1) * g_nand_header.m_pages_per_block;
		privdat->m_seek_cur = 0;
	}
	else if ( !strcmp(name, "romwrite-tmp") )
	{
		privdat->m_flags = 0x3000000;
		privdat->m_seek_max = 0;
		privdat->m_partition_offset = s147nand_9_get_nand_partition(8) * g_nand_header.m_pages_per_block;
		privdat->m_seek_cur = 0;
	}
	else
	{
		CpuSuspendIntr(&state);
		FreeSysMemory(f->privdata);
		CpuResumeIntr(state);
		return -ENOENT;
	}
	return 0;
}

static int nand_mdev_read_special(iop_file_t *f, void *ptr, int size)
{
	s147nand_mdev_privdata_t *privdata;
	int retres1;

	privdata = (s147nand_mdev_privdata_t *)f->privdata;
	if ( (privdata->m_flags & 0x10000) != 0 )
		return do_nand_copy_seccode_from_buf(f, ptr, size);
	if ( (privdata->m_flags & 0x40000) != 0 )
		return do_nand_copy_videomode_from_buf(f, ptr, size);
	if ( (privdata->m_flags & 0x100000) != 0 )
	{
		retres1 = do_nand_sector_rw(ptr, privdata->m_partition_offset, privdata->m_seek_cur, size);
		if ( retres1 < 0 )
			return retres1;
		privdata->m_seek_cur += size;
		return size;
	}
	if ( (privdata->m_flags & 0x1000000) != 0 )
	{
		retres1 = do_nand_sector_rw(ptr, privdata->m_partition_offset, privdata->m_seek_cur, size);
		if ( retres1 < 0 )
			return retres1;
		privdata->m_seek_cur += size;
		return size;
	}
	return -ENOENT;
}

static int nand_mdev_write_special(iop_file_t *f, void *ptr, int size)
{
	s147nand_mdev_privdata_t *privdata;
	int retres1;
	int xsz;

	privdata = (s147nand_mdev_privdata_t *)f->privdata;
	xsz =
		g_nand_header.m_nand_partition_8_info.m_size * g_nand_header.m_pages_per_block * g_nand_header.m_page_size_noecc;
	if ( (privdata->m_flags & 0x1000000) == 0 )
		return -EINVAL;
	if ( xsz < privdata->m_seek_cur + size )
	{
		Kprintf("s147nand.irx: Out of rewritable partition, lseek(%d) > max(%d)\n", privdata->m_seek_cur + size, xsz);
		return -EFBIG;
	}
	if ( (privdata->m_flags & 0x2000000) != 0 )
	{
		int i;

		retres1 = 0;
		for ( i = 0; i < g_nand_header.m_nand_partition_8_info.m_size; i += 1 )
			retres1 = s147nand_11_erasetranslatepageoffs(
				s147nand_27_blocks2pages(i + g_nand_header.m_nand_partition_8_info.m_offset));
		privdata->m_flags &= ~0x2000000;
		if ( retres1 )
			return retres1;
	}
	if ( !g_nand_header.m_page_size_noecc )
		__builtin_trap();
	if ( g_nand_header.m_page_size_noecc == -1 && privdata->m_seek_cur == (int)0x80000000 )
		__builtin_trap();
	if ( !g_nand_header.m_page_size_noecc )
		__builtin_trap();
	if ( g_nand_header.m_page_size_noecc == -1 && size == (int)0x80000000 )
		__builtin_trap();
	retres1 = s147nand_8_multi_write_dma(
		ptr,
		privdata->m_seek_cur / g_nand_header.m_page_size_noecc + privdata->m_partition_offset,
		size / g_nand_header.m_page_size_noecc);
	if ( retres1 < 0 )
		return retres1;
	privdata->m_seek_cur += size;
	privdata->m_seek_max = privdata->m_seek_cur;
	return size;
}

static int do_nand_copy_seccode_from_buf(iop_file_t *f, void *ptr, int size)
{
	s147nand_mdev_privdata_t *privdata;
	int xsize;

	privdata = (s147nand_mdev_privdata_t *)f->privdata;
	if ( privdata->m_seek_cur >= privdata->m_seek_max )
		return 0;
	xsize =
		(privdata->m_seek_max >= (int)(privdata->m_seek_cur + size)) ? size : (privdata->m_seek_max - privdata->m_seek_cur);
	memcpy(ptr, &g_nand_header.m_nand_seccode[privdata->m_seek_cur], xsize);
	privdata->m_seek_cur += xsize;
	return xsize;
}

static int do_nand_copy_videomode_from_buf(iop_file_t *f, void *ptr, int size)
{
	s147nand_mdev_privdata_t *privdata;
	int xsize;

	privdata = (s147nand_mdev_privdata_t *)f->privdata;
	if ( privdata->m_seek_cur >= privdata->m_seek_max )
		return 0;
	xsize =
		(privdata->m_seek_max >= (int)(privdata->m_seek_cur + size)) ? size : (privdata->m_seek_max - privdata->m_seek_cur);
	memcpy(ptr, &g_nand_header.m_nand_vidmode[privdata->m_seek_cur], xsize);
	privdata->m_seek_cur += xsize;
	return xsize;
}

int s147nand_11_erasetranslatepageoffs(int pageoffs)
{
	return s147nand_24_eraseoffset(s147nand_14_translate_pageoffs(pageoffs));
}

int s147nand_12_load_logaddrtable(void)
{
	s147nand_header_t hdr __attribute__((__aligned__(16)));
	int state;

	s147nand_20_nand_read_dma(&hdr, 0, 0, sizeof(hdr));
	if ( strncmp(hdr.m_sig, "S147NAND", 9) )
	{
		Kprintf("s147nand.irx: Unformatted device error.\n");
		return -19;
	}
	CpuSuspendIntr(&state);
	g_logical_addr_tbl = (u16 *)AllocSysMemory(ALLOC_FIRST, sizeof(u16) * hdr.m_block_size, 0);
	CpuResumeIntr(state);
	s147nand_19_logaddr_read(g_logical_addr_tbl, 1, sizeof(u16) * hdr.m_block_size);
	CpuSuspendIntr(&state);
	g_nand_unaligned_buf = AllocSysMemory(ALLOC_FIRST, s147nand_16_getnandinfo()->m_page_size_noecc, 0);
	CpuResumeIntr(state);
	if ( !g_nand_unaligned_buf )
	{
		Kprintf("s147nand.irx: AllocSysMemory failed (LogAddrTable)\n");
		return -1;
	}
	// Unofficial: remove redundant var
	return 0;
}

int s147nand_13_translate_blockoffs(int blockoffs)
{
	int tbladdr;

	if ( blockoffs <= 0 || blockoffs >= s147nand_16_getnandinfo()->m_block_size )
	{
		Kprintf("s147nand.irx: Invalid logical block address %d\n", blockoffs);
		return -1470010;
	}
	// Unofficial: check g_nand_unaligned_buf instead
	if ( !g_nand_unaligned_buf )
	{
		int logaddrtable;

		logaddrtable = s147nand_12_load_logaddrtable();
		if ( logaddrtable < 0 )
			return logaddrtable;
	}
	tbladdr = g_logical_addr_tbl[blockoffs];
	switch ( tbladdr )
	{
		case 0xAAAA:
			return blockoffs;
		case 0xCCCC:
		case 0xEEEE:
			return -1470010;
		default:
			return tbladdr;
	}
}

int s147nand_14_translate_pageoffs(int pageoffs)
{
	int tblockoffs;

	tblockoffs = s147nand_13_translate_blockoffs(s147nand_28_pages2blocks(pageoffs));
	if ( tblockoffs == -1470010 )
		return -1470010;
	if ( !g_nand_header.m_pages_per_block )
		__builtin_trap();
	if ( g_nand_header.m_pages_per_block == -1 && pageoffs == (int)0x80000000 )
		__builtin_trap();
	return s147nand_27_blocks2pages(tblockoffs) + (pageoffs % g_nand_header.m_pages_per_block);
}

static int dev9_intr_handler(void *unusd)
{
	(void)unusd;

	iWakeupThread(g_thid);
	return 1;
}

int s147nand_15_nandinit(void)
{
	int intrstate;
	iop_sema_t semaparam;
	USE_IOP_MMIO_HWPORT();

	DisableIntr(IOP_IRQ_DMA_DEV9, &intrstate);
	ReleaseIntrHandler(IOP_IRQ_DMA_DEV9);
	// Unofficial: removed unused userdata var
	RegisterIntrHandler(IOP_IRQ_DMA_DEV9, 1, dev9_intr_handler, NULL);
	EnableIntr(IOP_IRQ_DMA_DEV9);
	sceDisableDMAChannel(IOP_DMAC_DEV9);
	sceSetDMAPriority(IOP_DMAC_DEV9, 7);
	sceEnableDMAChannel(IOP_DMAC_DEV9);
	// Unofficial: use uncached mirror
	iop_mmio_hwport->ssbus2.ind_B_address = 0xB4000008;
	// Unofficial: make semaparam local var
	semaparam.initial = 1;
	semaparam.max = 1;
	semaparam.attr = SA_THPRI;
	semaparam.option = 0;
	g_sema_id_nand = CreateSema(&semaparam);
	if ( g_sema_id_nand >= 0 )
		return 0;
	printf("nand_Init: CreateSema error (%d)\n", g_sema_id_nand);
	return -1;
}

s147nand_info_t *s147nand_16_getnandinfo(void)
{
	return &g_nand_info;
}

int s147nand_17_get_sema(void)
{
	return g_sema_id_nand;
}

void s147nand_18_enable_nand_watchdog(void)
{
	g_nand_watchdog_enabled = 1;
}

int s147nand_19_logaddr_read(u16 *tbl, int pageoffs, int bytecnt)
{
	int i;
	int pagecnt;

	pagecnt = s147nand_30_bytes2pagesnoeccround(bytecnt);
	for ( i = 0; i < pagecnt; i += 1 )
	{
		int retres;

		retres = s147nand_20_nand_read_dma(
			&tbl[((g_nand_info.m_page_size_noecc >> 2) << 1) * i], pageoffs + i, 0, g_nand_info.m_page_size_noecc);
		if ( retres )
			return retres;
	}
	return 0;
}

int s147nand_20_nand_read_dma(void *ptr, int pageoffs, int byteoffs, int bytecnt)
{
	int retres;

	WaitSema(g_sema_id_nand);
	retres = nand_lowlevel_read_dma(ptr, pageoffs, byteoffs, bytecnt);
	SignalSema(g_sema_id_nand);
	return retres;
}

int s147nand_21_nand_read_pio(void *ptr, int pageoffs, int byteoffs, int bytecnt)
{
	int retres;
	int state;

	WaitSema(g_sema_id_nand);
	CpuSuspendIntr(&state);
	retres = nand_lowlevel_read_pio(ptr, pageoffs, byteoffs, bytecnt);
	CpuResumeIntr(state);
	SignalSema(g_sema_id_nand);
	return retres;
}

int s147nand_22_nand_write_dma(void *ptr, int pageoffs, int byteoffs, int bytecnt)
{
	int retres;

	WaitSema(g_sema_id_nand);
	retres = nand_lowlevel_write_dma(ptr, pageoffs, byteoffs, bytecnt);
	SignalSema(g_sema_id_nand);
	return retres;
}

int s147nand_23_nand_write_pio(void *ptr, int pageoffs, int byteoffs, int bytecnt)
{
	int retres;
	int state;

	WaitSema(g_sema_id_nand);
	CpuSuspendIntr(&state);
	retres = nand_lowlevel_write_pio(ptr, pageoffs, byteoffs, bytecnt);
	CpuResumeIntr(state);
	SignalSema(g_sema_id_nand);
	return retres;
}

int s147nand_24_eraseoffset(int pageoffs)
{
	int retres;
	int state;
	u8 val1;
	u8 val2;

	s147nand_21_nand_read_pio(
		&val1,
		s147nand_27_blocks2pages(s147nand_28_pages2blocks(pageoffs)) + 0,
		g_nand_info.m_page_size_noecc,
		sizeof(val1));
	s147nand_21_nand_read_pio(
		&val2,
		s147nand_27_blocks2pages(s147nand_28_pages2blocks(pageoffs)) + 1,
		g_nand_info.m_page_size_noecc,
		sizeof(val2));
	if ( val1 != 255 || val2 != 255 )
		return -1470020;
	WaitSema(g_sema_id_nand);
	CpuSuspendIntr(&state);
	retres = nand_lowlevel_blockerase(pageoffs);
	CpuResumeIntr(state);
	SignalSema(g_sema_id_nand);
	return retres;
}

int s147nand_25_nand_blockerase(int pageoffs)
{
	int retres;
	int state;

	WaitSema(g_sema_id_nand);
	CpuSuspendIntr(&state);
	retres = nand_lowlevel_blockerase(pageoffs);
	CpuResumeIntr(state);
	SignalSema(g_sema_id_nand);
	return retres;
}

int s147nand_26_nand_readid(void *ptr)
{
	int retres;
	int state;

	WaitSema(g_sema_id_nand);
	CpuSuspendIntr(&state);
	retres = nand_lowlevel_readid(ptr);
	CpuResumeIntr(state);
	SignalSema(g_sema_id_nand);
	return retres;
}

static int nand_lowlevel_read_dma(void *ptr, int pageoffs, int byteoffs, int bytecnt)
{
	int state;
	USE_S147_DEV9_MEM_MMIO();
	USE_S147MAMD_DEV9_IO_MMIO();

	if ( pageoffs < 0 || pageoffs >= g_nand_info.m_page_count )
		return -1470010;
	if ( byteoffs < 0 || byteoffs >= g_nand_info.m_page_size_withecc )
		return -1470010;
	if ( bytecnt < 4 || g_nand_info.m_page_size_withecc < byteoffs + bytecnt )
		return -1470010;
	CpuSuspendIntr(&state);
	s147nand_dev9_io_mmio->m_nand_cmd_enable = 1;
	s147nand_dev9_io_mmio->m_nand_cmd_sel = 0;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (u16)(byteoffs);
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (u16)(byteoffs & 0xF00) >> 8;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = pageoffs;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (u16)(pageoffs & 0xFF00) >> 8;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (pageoffs & 0xFF0000) >> 16;
	s147nand_dev9_io_mmio->m_nand_cmd_sel = 0x30;
	CpuResumeIntr(state);
	while ( (s147nand_dev9_io_mmio->m_nand_waitflag & 1) != 0 )
		;
	CpuSuspendIntr(&state);
	s147_dev9_mem_mmio->m_security_unlock_unlock = 0;
	sceSetSliceDMA(IOP_DMAC_DEV9, ptr, bytecnt >> 2, 1, 0);
	g_thid = GetThreadId();
	CpuResumeIntr(state);
	sceStartDMA(IOP_DMAC_DEV9);
	SleepThread();
	s147nand_dev9_io_mmio->m_nand_cmd_enable = 0;
	if ( g_nand_watchdog_enabled == 1 )
		s147_dev9_mem_mmio->m_watchdog_flag2 = 0;
	return 0;
}

static int nand_lowlevel_read_pio(void *ptr, int pageoffs, int byteoffs, int bytecnt)
{
	int i;
	USE_S147_DEV9_MEM_MMIO();
	USE_S147MAMD_DEV9_IO_MMIO();

	if ( pageoffs < 0 || pageoffs >= g_nand_info.m_page_count )
		return -1470010;
	if ( byteoffs < 0 || byteoffs >= g_nand_info.m_page_size_withecc )
		return -1470010;
	if ( bytecnt <= 0 || g_nand_info.m_page_size_withecc < byteoffs + bytecnt )
		return -1470010;
	while ( (s147nand_dev9_io_mmio->m_nand_waitflag & 1) != 0 )
		;
	s147nand_dev9_io_mmio->m_nand_cmd_enable = 1;
	s147nand_dev9_io_mmio->m_nand_cmd_sel = 0;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = byteoffs;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (u16)(byteoffs & 0xF00) >> 8;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = pageoffs;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (u16)(pageoffs & 0xFF00) >> 8;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (pageoffs & 0xFF0000) >> 16;
	s147nand_dev9_io_mmio->m_nand_cmd_sel = 0x30;
	while ( (s147nand_dev9_io_mmio->m_nand_waitflag & 1) != 0 )
		;
	for ( i = 0; i < bytecnt; i += 1 )
		((u8 *)ptr)[i] = s147nand_dev9_io_mmio->m_nand_outbyte;
	s147nand_dev9_io_mmio->m_nand_cmd_enable = 0;
	if ( g_nand_watchdog_enabled == 1 )
		s147_dev9_mem_mmio->m_watchdog_flag2 = 0;
	return 0;
}

static int nand_lowlevel_write_dma(void *ptr, int pageoffs, int byteoffs, int bytecnt)
{
	int state;
	u8 flgtmp;
	USE_S147_DEV9_MEM_MMIO();
	USE_S147MAMD_DEV9_IO_MMIO();

	if ( pageoffs < 0 || pageoffs >= g_nand_info.m_page_count )
		return -1470010;
	if ( byteoffs < 0 || byteoffs >= g_nand_info.m_page_size_withecc )
		return -1470010;
	if ( bytecnt < 4 || g_nand_info.m_page_size_withecc < byteoffs + bytecnt )
		return -1470010;
	CpuSuspendIntr(&state);
	s147nand_dev9_io_mmio->m_nand_write_cmd_unlock = 0xA5;
	s147nand_dev9_io_mmio->m_nand_cmd_enable = 1;
	s147nand_dev9_io_mmio->m_nand_cmd_sel = 0x80;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (u16)(byteoffs);
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (u16)(byteoffs & 0xF00) >> 8;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = pageoffs;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (u16)(pageoffs & 0xFF00) >> 8;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (pageoffs & 0xFF0000) >> 16;
	s147_dev9_mem_mmio->m_security_unlock_unlock = 0;
	sceSetSliceDMA(IOP_DMAC_DEV9, ptr, bytecnt >> 2, 1, 1);
	g_thid = GetThreadId();
	CpuResumeIntr(state);
	sceStartDMA(IOP_DMAC_DEV9);
	SleepThread();
	s147nand_dev9_io_mmio->m_nand_cmd_sel = 0x10;
	while ( (s147nand_dev9_io_mmio->m_nand_waitflag & 1) != 0 )
		;
	s147nand_dev9_io_mmio->m_nand_cmd_sel = 0x70;
	flgtmp = s147nand_dev9_io_mmio->m_nand_outbyte;
	s147nand_dev9_io_mmio->m_nand_cmd_enable = 0;
	s147nand_dev9_io_mmio->m_nand_write_cmd_unlock = 0;
	if ( g_nand_watchdog_enabled == 1 )
		s147_dev9_mem_mmio->m_watchdog_flag2 = 0;
	if ( (flgtmp & 0x80) == 0 )
		return -1470030;
	if ( (flgtmp & 1) != 0 )
		return -1470020;
	return 0;
}

static int nand_lowlevel_write_pio(void *ptr, int pageoffs, int byteoffs, int bytecnt)
{
	u8 flgtmp;
	int i;
	USE_S147_DEV9_MEM_MMIO();
	USE_S147MAMD_DEV9_IO_MMIO();

	if ( pageoffs < 0 || pageoffs >= g_nand_info.m_page_count )
		return -1470010;
	if ( byteoffs < 0 || byteoffs >= g_nand_info.m_page_size_withecc )
		return -1470010;
	if ( bytecnt <= 0 || g_nand_info.m_page_size_withecc < byteoffs + bytecnt )
		return -1470010;
	s147nand_dev9_io_mmio->m_nand_write_cmd_unlock = 0xA5;
	s147nand_dev9_io_mmio->m_nand_cmd_enable = 1;
	s147nand_dev9_io_mmio->m_nand_cmd_sel = 0x80;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = byteoffs;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (u16)(byteoffs & 0xF00) >> 8;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = pageoffs;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (u16)(pageoffs & 0xFF00) >> 8;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (pageoffs & 0xFF0000) >> 16;
	for ( i = 0; i < bytecnt; i += 1 )
		s147nand_dev9_io_mmio->m_nand_outbyte = ((u8 *)ptr)[i];
	s147nand_dev9_io_mmio->m_nand_cmd_sel = 0x10;
	while ( (s147nand_dev9_io_mmio->m_nand_waitflag & 1) != 0 )
		;
	s147nand_dev9_io_mmio->m_nand_cmd_sel = 0x70;
	flgtmp = s147nand_dev9_io_mmio->m_nand_outbyte;
	s147nand_dev9_io_mmio->m_nand_cmd_enable = 0;
	s147nand_dev9_io_mmio->m_nand_write_cmd_unlock = 0;
	if ( g_nand_watchdog_enabled == 1 )
		s147_dev9_mem_mmio->m_watchdog_flag2 = 0;
	if ( (flgtmp & 0x80) == 0 )
		return -1470030;
	if ( (flgtmp & 1) != 0 )
		return -1470020;
	return 0;
}

static int nand_lowlevel_blockerase(int pageoffs)
{
	u8 flgtmp;
	USE_S147_DEV9_MEM_MMIO();
	USE_S147MAMD_DEV9_IO_MMIO();

	if ( pageoffs < 0 || pageoffs >= g_nand_info.m_page_count )
		return -1470010;
	s147nand_dev9_io_mmio->m_nand_write_cmd_unlock = 0xA5;
	s147nand_dev9_io_mmio->m_nand_cmd_enable = 1;
	s147nand_dev9_io_mmio->m_nand_cmd_sel = 0x60;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = pageoffs & 0xC0;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (u16)(pageoffs & 0xFF00) >> 8;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = (pageoffs & 0xFF0000) >> 16;
	s147nand_dev9_io_mmio->m_nand_cmd_sel = 0xD0;
	while ( (s147nand_dev9_io_mmio->m_nand_waitflag & 1) != 0 )
		;
	s147nand_dev9_io_mmio->m_nand_cmd_sel = 0x70;
	flgtmp = s147nand_dev9_io_mmio->m_nand_outbyte;
	s147nand_dev9_io_mmio->m_nand_cmd_enable = 0;
	s147nand_dev9_io_mmio->m_nand_write_cmd_unlock = 0;
	if ( g_nand_watchdog_enabled == 1 )
		s147_dev9_mem_mmio->m_watchdog_flag2 = 0;
	if ( (flgtmp & 0x80) == 0 )
		return -1470030;
	if ( (flgtmp & 1) != 0 )
		return -1470020;
	return 0;
}

static int nand_lowlevel_readid(void *ptr)
{
	int i;
	USE_S147_DEV9_MEM_MMIO();
	USE_S147MAMD_DEV9_IO_MMIO();

	if ( !ptr )
		return -1470010;
	s147nand_dev9_io_mmio->m_nand_cmd_enable = 1;
	s147nand_dev9_io_mmio->m_nand_cmd_sel = 0x90;
	s147nand_dev9_io_mmio->m_nand_cmd_offs = 0;
	for ( i = 0; i < 5; i += 1 )
		((u8 *)ptr)[i] = s147nand_dev9_io_mmio->m_nand_outbyte;
	s147nand_dev9_io_mmio->m_nand_cmd_enable = 0;
	if ( g_nand_watchdog_enabled == 1 )
		s147_dev9_mem_mmio->m_watchdog_flag2 = 0;
	return 0;
}

int s147nand_27_blocks2pages(int blocks)
{
	return blocks * g_nand_info.m_pages_per_block;
}

int s147nand_28_pages2blocks(int pages)
{
	if ( !g_nand_info.m_pages_per_block )
		__builtin_trap();
	if ( g_nand_info.m_pages_per_block == -1 && pages == (int)0x80000000 )
		__builtin_trap();
	return pages / g_nand_info.m_pages_per_block;
}

int s147nand_29_pages2blockround(int pages)
{
	int blocks;

	if ( !g_nand_info.m_pages_per_block )
		__builtin_trap();
	if ( g_nand_info.m_pages_per_block == -1 && pages == (int)0x80000000 )
		__builtin_trap();
	if ( !g_nand_info.m_pages_per_block )
		__builtin_trap();
	blocks = pages / g_nand_info.m_pages_per_block;
	if ( g_nand_info.m_pages_per_block == -1 && pages == (int)0x80000000 )
		__builtin_trap();
	return blocks + ((pages % g_nand_info.m_pages_per_block) ? 1 : 0);
}

int s147nand_30_bytes2pagesnoeccround(int bytes)
{
	int pages;

	if ( !g_nand_info.m_page_size_noecc )
		__builtin_trap();
	if ( g_nand_info.m_page_size_noecc == -1 && bytes == (int)0x80000000 )
		__builtin_trap();
	if ( !g_nand_info.m_page_size_noecc )
		__builtin_trap();
	pages = bytes / g_nand_info.m_page_size_noecc;
	if ( g_nand_info.m_page_size_noecc == -1 && bytes == (int)0x80000000 )
		__builtin_trap();
	return pages + ((bytes % g_nand_info.m_page_size_noecc) ? 1 : 0);
}
