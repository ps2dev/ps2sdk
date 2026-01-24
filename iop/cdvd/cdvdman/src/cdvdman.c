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

#include "iomanX.h"

#include <cdvdman.h>

#include <cdvd-ioctl.h>
#include <dev5_mmio_hwport.h>
#include <errno.h>
#include <hdd-ioctl.h>
#include <iop_mmio_hwport.h>
#include <kerr.h>

IRX_ID("cdvd_driver", 2, 38);
// Based on the module from SCE SDK 3.1.0.

extern struct irx_export_table _exp_cdvdman;

#ifdef CDVD_VARIANT_OSD
#define PRID $15

#define _mfc0(reg)                                                                                                     \
	({                                                                                                                   \
		u32 val;                                                                                                           \
		__asm__ volatile("mfc0 %0, " #reg : "=r"(val));                                                                    \
		val;                                                                                                               \
	})

#define mfc0(reg) _mfc0(reg)
#endif

typedef struct cdvdman_dirtbl_entry_
{
	int m_number;
	int m_parent;
	int m_extent;
	char m_name[32];
} cdvdman_dirtbl_entry_t;

typedef struct cdvdman_fhinfo_
{
	u32 m_file_lsn;
	u32 m_read_pos;
	u32 m_file_size;
	u32 m_filemode;
	int m_fd_flags;
	int m_fd_layer;
	int m_cache_file_fd;
	u8 *m_fd_rcvbuf;
	u32 m_fd_rbsize;
	void *m_max_cluster;
	int m_sector_count_total;
	int m_cluster_cur;
} cdvdman_fhinfo_t;

typedef struct cdvdman_pathtbl_
{
	int m_cache_path_sz;
	int m_lsn;
	unsigned int m_nsec;
	int m_layer;
	unsigned int m_cache_hit_count;
} cdvdman_pathtbl_t;

typedef struct cdvdman_filetbl_entry_
{
	sceCdlFILE m_file_struct;
	int m_flags;
} cdvdman_filetbl_entry_t;

typedef struct iso9660_desc_
{
	// cppcheck-suppress unusedStructMember
	unsigned char m_type[1];
	unsigned char m_id[5];
	// cppcheck-suppress unusedStructMember
	unsigned char m_version[1];
	// cppcheck-suppress unusedStructMember
	unsigned char m_unused1[1];
	// cppcheck-suppress unusedStructMember
	unsigned char m_system_id[32];
	// cppcheck-suppress unusedStructMember
	unsigned char m_volume_id[32];
	// cppcheck-suppress unusedStructMember
	unsigned char m_unused2[8];
	// cppcheck-suppress unusedStructMember
	unsigned char m_volume_space_size[8];
	// cppcheck-suppress unusedStructMember
	unsigned char m_unused3[32];
	// cppcheck-suppress unusedStructMember
	unsigned char m_volume_set_size[4];
	// cppcheck-suppress unusedStructMember
	unsigned char m_volume_sequence_number[4];
	// cppcheck-suppress unusedStructMember
	unsigned char m_logical_block_size[4];
	// cppcheck-suppress unusedStructMember
	unsigned char m_path_table_size[8];
	unsigned char m_type_l_path_table[4];
	// cppcheck-suppress unusedStructMember
	unsigned char m_opt_type_l_path_table[4];
	// cppcheck-suppress unusedStructMember
	unsigned char m_type_m_path_table[4];
	// cppcheck-suppress unusedStructMember
	unsigned char m_opt_type_m_path_table[4];
	// cppcheck-suppress unusedStructMember
	unsigned char m_root_directory_record[34];
	// cppcheck-suppress unusedStructMember
	unsigned char m_volume_set_id[128];
	// cppcheck-suppress unusedStructMember
	unsigned char m_publisher_id[128];
	// cppcheck-suppress unusedStructMember
	unsigned char m_preparer_id[128];
	// cppcheck-suppress unusedStructMember
	unsigned char m_application_id[128];
	// cppcheck-suppress unusedStructMember
	unsigned char m_copyright_file_id[37];
	// cppcheck-suppress unusedStructMember
	unsigned char m_abstract_file_id[37];
	// cppcheck-suppress unusedStructMember
	unsigned char m_bibliographic_file_id[37];
	// cppcheck-suppress unusedStructMember
	unsigned char m_creation_date[17];
	// cppcheck-suppress unusedStructMember
	unsigned char m_modification_date[17];
	// cppcheck-suppress unusedStructMember
	unsigned char m_expiration_date[17];
	// cppcheck-suppress unusedStructMember
	unsigned char m_effective_date[17];
	// cppcheck-suppress unusedStructMember
	unsigned char m_file_structure_version[1];
	// cppcheck-suppress unusedStructMember
	unsigned char m_unused4[1];
	// cppcheck-suppress unusedStructMember
	unsigned char m_application_data[512];
	// cppcheck-suppress unusedStructMember
	unsigned char m_unused5[653];
} iso9660_desc_t;

typedef struct iso9660_path_
{
	unsigned char m_name_len[2];
	unsigned char m_extent[4];
	unsigned char m_parent[2];
	unsigned char m_name[];
} iso9660_path_t;

typedef struct iso9660_dirent_
{
	unsigned char m_length[1];
	unsigned char m_ext_attr_length[1];
	unsigned char m_extent[8];
	unsigned char m_size[8];
	unsigned char m_date[7];
	unsigned char m_flags[1];
	unsigned char m_file_unit_size[1];
	unsigned char m_interleave[1];
	unsigned char m_volume_sequence_number[4];
	unsigned char m_name_len[1];
	unsigned char m_name[];
} iso9660_dirent_t;

static int cdrom_init(iop_device_t *dev);
extern void cdvdman_termcall(int with_stop);
static int cdrom_deinit(iop_device_t *dev);
static int cdrom_dopen(iop_file_t *f, const char *dirname);
static int cdrom_getstat(iop_file_t *f, const char *name, iox_stat_t *buf);
static int cdrom_dread(iop_file_t *f, iox_dirent_t *buf);
static int cdrom_open(iop_file_t *f, const char *name, int mode, int arg4);
static int cdrom_close(iop_file_t *f);
static int cdrom_read(iop_file_t *f, void *buf, int nbytes);
static int cdrom_ioctl(iop_file_t *f, int arg, void *param);
static int cdrom_ioctl2(iop_file_t *f, int request, void *argp, size_t arglen, void *bufp, size_t buflen);
static int
cdrom_devctl(iop_file_t *f, const char *, int cmd, void *argp, unsigned int arglen, void *bufp, unsigned int buflen);
static int cdrom_lseek(iop_file_t *f, int offset, int pos);
static int CdSearchFileInner(cdvdman_filetbl_entry_t *fp, const char *name, int layer);
static int sceCdSearchDir(char *dirname, int layer);
static int sceCdReadDir(sceCdlFILE *fp, int dsec, int index, int layer);
static int cdvdman_cmpname(const char *p, const char *q);
static int CD_newmedia(int arg);
static int cdvdman_finddir(int target_parent, const char *target_name);
static int CD_cachefile(int dsec, int layer);
static int disc_read(int size, int loc, void *buffer, int layer);
static int path_tbl_init(u32 blocks, char *fname, int action);
extern unsigned int optimized_memcpy(char *dst, const char *src, unsigned int n);
static int vSetAlarm(iop_sys_clock_t *sys_clock, unsigned int (*alarm_cb)(void *arg), void *arg);
static int vCancelAlarm(unsigned int (*alarm_cb)(void *arg), void *arg);
static int vSetEventFlag(int ef, u32 bits);
static int vDelayThread(int usec);
static int DvdDual_infochk();
static void cdvdman_init();
static void cdvdman_write_scmd(cdvdman_internal_struct_t *s);
static int intrh_dma_3(void *userdata);
static int cdvdman_mediactl(int code);
#ifndef CDVD_VARIANT_XOSD
static int cdvdman_ncmd_sender_06();
#endif
static int cdvdman_gettoc(u8 *toc);
static int cdvdman_isdvd();
static int sceCdRead0_Rty(u32 lsn, u32 nsec, void *buf, const sceCdRMode *mode, int ncmd, int dintrsec, void *func);
static int cdvdman_syncdec(int decflag, int decxor, int shift, u32 data);
static void Read2intrCDVD(int read2_flag);
#ifndef CDVD_VARIANT_XOSD
static int cdvdman_scmd_sender_03_30(u8 *buf, u32 *status);
#endif
static int cdvdman_scmd_sender_3B(int arg1);
#ifdef CDVD_VARIANT_DNAS
static int cdvdman_ncmd_sender_0C(int arg1, u32 arg2, u32 arg3);
#endif
#ifdef CDVD_VARIANT_XOSD
static unsigned int sceCdChgSpdlCtrl(int mode);
#endif
#ifdef CDVD_VARIANT_XOSD
static int cdvdman_change_drive(int mode);
static int cdvdman_get_renewal_date(u8 *buffer, u32 *status);
static int get_cdvd_register(int regnr);
static int get_disk_type_ex();
static int sc_ffffffd8(u32 *status);
static int set_cdvd_dev5_base_addr_atapi(int mode);
static int update_cd_mode_ps2_atapi(void);
#endif

static char g_cdvdman_cache_name[256] = "host0:";
static int g_cdvdman_cache_sector_size_count = 1;
static int g_cdvdman_srchspd = 0;
static int g_cdvdman_spinctl = -1;
static int g_cdvdman_spinnom = -1;
static int g_cdvdman_trycnt = -1;
static int g_cdvdman_iocache = 0;
static unsigned int g_cdvdman_lcn_offset = 0;
static unsigned int g_cdvdman_numbytes_offset = 0;
static int g_cdvdman_strmerr = 0;

IOMANX_RETURN_VALUE_IMPL(EIO);

static iop_device_ops_t g_cdvdman_cddev_ops = {
	&cdrom_init,                   // init,
	&cdrom_deinit,                 // deinit,
	IOMANX_RETURN_VALUE(EIO),      // format,
	&cdrom_open,                   // open,
	&cdrom_close,                  // close,
	&cdrom_read,                   // read,
	IOMANX_RETURN_VALUE(EIO),      // write,
	&cdrom_lseek,                  // lseek,
	&cdrom_ioctl,                  // ioctl,
	IOMANX_RETURN_VALUE(EIO),      // remove,
	IOMANX_RETURN_VALUE(EIO),      // mkdir,
	IOMANX_RETURN_VALUE(EIO),      // rmdir,
	&cdrom_dopen,                  // dopen,
	&cdrom_close,                  // close,
	&cdrom_dread,                  // dread
	&cdrom_getstat,                // getstat
	IOMANX_RETURN_VALUE(EIO),      // chstat,
	IOMANX_RETURN_VALUE(EIO),      // rename,
	IOMANX_RETURN_VALUE(EIO),      // chdir
	IOMANX_RETURN_VALUE(EIO),      // sync
	IOMANX_RETURN_VALUE(EIO),      // mount,
	IOMANX_RETURN_VALUE(EIO),      // umount,
	IOMANX_RETURN_VALUE_S64(EIO),  // lseek64,
	&cdrom_devctl,                 // devctl,
	IOMANX_RETURN_VALUE(EIO),      // readdir,
	IOMANX_RETURN_VALUE(EIO),      // readlink,
	&cdrom_ioctl2,                 // ioctl2,
};
static iop_device_t g_cdvdman_cddev = {"cdrom", IOP_DT_FSEXT | IOP_DT_FS, 1, "CD-ROM ", &g_cdvdman_cddev_ops};
static int g_cdvdman_sync_timeout = 15000;
static int g_cdvdman_stream_timeout = 5000;
#ifdef CDVD_VARIANT_DNAS
static iop_sys_clock_t g_readid_systemtime = {0, 0};
#endif
static int g_verbose_level = 0;
static cdvdman_pathtbl_t *g_cdvdman_pathtbl = NULL;
static unsigned int g_cache_count = 0;
static unsigned int g_cache_table = 0;
static unsigned int g_cdvdman_pathtblsize = 0;
static int g_cache_path_size = 0;
static int g_cache_path_fd = -1;
static int g_cdvdman_fs_cdsec = 0;
static int g_cdvdman_fs_layer = -1;
static int g_cdvdman_fs_cache = 0;
static int g_cdvdman_fs_base2 = 0;
#ifdef CDVD_VARIANT_XOSD
static vu8 *g_cdvdreg_bf801460 = (vu8 *)0xBF801460;  // &iop_mmio_hwport->dev9c[0]
// static u8 *cdvdreg_bf801462 = 0xBF801462;
static vu8 *g_cdvdreg_bf801464 = (vu8 *)0xBF801464;  // &iop_mmio_hwport->dev9c[4]
static vu8 *g_cdvdreg_bf80146c = (vu8 *)0xBF80146C;  // &iop_mmio_hwport->dev9c[0xC]
// static u8 *cdvdreg_bf80146e = 0xBF80146E;
static int g_cd_atapi_evfid = 0xFFFFFFFF;
static int g_acmd_evfid = 0xFFFFFFFF;
static int g_adma_evfid = 0xFFFFFFFF;
#endif
static int g_cdvdman_clk_flg = 0;
static int g_cdvdman_cd36key = 0;
static int g_cdvdman_ee_rpc_fno = 0;
static int g_cdvdman_mmode = 0;
static int g_cdvdman_last_cmdfunc = 0;
static int g_cdvdman_minver_10700 = 0;
static int g_cdvdman_minver_20200 = 0;
static int g_cdvdman_minver_20400 = 0;
static int g_cdvdman_minver_20800 = 0;
static int g_cdvdman_emudvd9 = 0;
static int g_cdvdman_minver_50000 = 0;
static int g_cdvdman_minver_50200 = 0;
static int g_cdvdman_minver_50400 = 0;
static int g_cdvdman_minver_50600 = 0;
static int g_cdvdman_minver_60000 = 0;
static int g_cdvdman_minver_60200 = 0;
#ifdef CDVD_VARIANT_OSD
static int g_cdvdman_minver_60600 = 0;
#endif
#ifdef CDVD_VARIANT_XOSD
static int g_cdvdman_vernotxxx1x = 0;
#endif
static int g_cdvdman_minver_x_model_15 = 0;
#ifdef CDVD_VARIANT_XOSD
static int g_cdvdman_debug_atapi_intr = 0;
#endif
static const char *g_masterdisc_header = "PlayStation Master Disc";
static char g_cdvdman_ncmd = 0x06;
static int g_cdvdman_chmedia = 0;
static int g_cdvdman_chflags[4] = {1, 1, 1, 1};
static int g_cdvdman_rtindex = 0;
static int g_cdvdman_retries = 0;
static u8 *g_cdvdman_ptoc;
#ifdef CDVD_VARIANT_XOSD
static int g_cdvdman_csys_evfid;
#endif
static int g_scmd_evfid;
static void *g_cdvdman_temp_buffer_ptr;
static int g_sfile_evfid;
static int g_ncmd_evfid;
#ifdef CDVD_VARIANT_OSD
static s32 g_cdvdman_apply_scmd_sema;
#endif
static int g_fio_fsv_evfid;
static int g_cdvdman_intr_evfid;
#ifdef CDVD_VARIANT_XOSD
static int (*g_cdvdman_atapi_eject_bs_power_callback)(int interrupt_nr, void *userdata);
#endif
static sceCdCBFunc g_cdvdman_user_cb;
static void *g_cdvdman_power_off_callback_userdata;
static void (*g_cdvdman_cdstm0cb)(int val);
static sceCdCLOCK g_cdvdman_clock;
static void (*g_cdvdman_power_off_callback)(void *userdata);
#ifdef CDVD_VARIANT_XOSD
static void *g_cdvdman_atapi_eject_bs_power_callback_userdata;
#endif
static void (*g_cdvdman_cdstm1cb)(int val);
static int g_cdvdman_cmdfunc;
static cdvdman_fhinfo_t g_cdvdman_fhinfo[16];
static char g_cdvdman_sfname[1024];
static cdvdman_filetbl_entry_t g_cdvdman_filetbl[64];
static cdvdman_dirtbl_entry_t g_cdvdman_dirtbl[128];
static int g_cdvdman_pathtblflag;
static char g_cdvdman_fs_rbuf[0x800];
static int g_cdvdman_readptr;
static iop_sys_clock_t g_cdvdman_read_alarm_cb_timeout;
static iop_sys_clock_t g_cdvdman_ncmd_timeout;
static void *g_cdvdman_readbuf;
static iop_sys_clock_t g_cdvdman_power_off_timeout;
#ifdef CDVD_VARIANT_OSD
static int g_cdvdman_config_numblocks;
#endif
static char g_cdvdman_fsvrbuf[42128];
static cdvdman_internal_struct_t g_cdvdman_istruct;
static const int g_cdvdman_cache_sector_count = 16;

int _start(int ac, char **av)
{
	(void)ac;
	(void)av;

	if ( RegisterLibraryEntries(&_exp_cdvdman) )
	{
		return MODULE_NO_RESIDENT_END;
	}
	DelDrv(g_cdvdman_cddev.name);
	if ( AddDrv(&g_cdvdman_cddev) )
	{
		cdrom_deinit(&g_cdvdman_cddev);
		return MODULE_NO_RESIDENT_END;
	}
	g_cdvdman_ptoc = (u8 *)&g_cdvdman_fsvrbuf[0x924];
	g_cdvdman_temp_buffer_ptr = g_cdvdman_fsvrbuf;
	cdvdman_init();
#if 0
	SetRebootTimeLibraryHandlingMode(&_exp_cdvdman, 2);
#else
	// Call termination before disabling interrupts
	_exp_cdvdman.mode &= ~6;
	_exp_cdvdman.mode |= 2;
#endif
	return MODULE_RESIDENT_END;
}

void *sceGetFsvRbuf(void)
{
	return g_cdvdman_fsvrbuf;
}

static int cdrom_init(iop_device_t *dev)
{
	unsigned int i;
	iop_event_t event;
#ifdef CDVD_VARIANT_OSD
	iop_sema_t sema;
#endif
#ifdef CDVD_VARIANT_XOSD
	USE_DEV5_MMIO_HWPORT();
#endif
	int scres_unused;

	(void)dev;

	PRINTF("cdvdman Init\n");
	g_cdvdman_istruct.m_wait_flag = 1;
	g_cdvdman_istruct.m_scmd_flag = 1;
	g_cdvdman_istruct.m_read2_flag = 0;
	g_cdvdman_istruct.m_stream_flag = 0;
	g_cdvdman_istruct.m_last_error = SCECdErNO;
	g_cdvdman_istruct.m_layer_1_lsn = 0;
	g_cdvdman_istruct.m_use_toc = 0;
	g_cdvdman_istruct.m_last_read_timeout = 0;
	g_cdvdman_istruct.m_power_flag = 0;
	g_cdvdman_istruct.m_current_dvd = 0;
	g_cdvdman_istruct.m_dual_layer_emulation = 0;
	g_cdvdman_istruct.m_dec_state = 0;
	g_cdvdman_istruct.m_check_version = 0;
	g_cdvdman_istruct.m_dec_shift = 0;
	g_cdvdman_istruct.m_opo_or_para = -1;
	g_cdvdman_istruct.m_no_dec_flag = 0;
#ifdef CDVD_VARIANT_XOSD
	g_cdvdman_istruct.m_atapi_disk_ejected = 0;
#endif
	g_cdvdman_istruct.m_cd_inited = 0;
#ifdef CDVD_VARIANT_XOSD
	g_cdvdman_istruct.m_chgsys_callback_next_disktype_last = 0;
	g_cdvdman_istruct.m_chgsys_writer_drive_shell_is_open = 1;
#endif
	g_cdvdman_istruct.m_tray_is_open = 0;
	g_cdvdman_ee_rpc_fno = 0;
	g_cdvdman_spinctl = -1;
	event.attr = EA_MULTI;
	event.bits = 0;
	event.option = 0;
	g_cdvdman_intr_evfid = CreateEventFlag(&event);
	g_scmd_evfid = CreateEventFlag(&event);
	g_ncmd_evfid = CreateEventFlag(&event);
	g_sfile_evfid = CreateEventFlag(&event);
	g_fio_fsv_evfid = CreateEventFlag(&event);
#ifdef CDVD_VARIANT_XOSD
	g_cdvdman_csys_evfid = CreateEventFlag(&event);
#endif
	ClearEventFlag(g_cdvdman_intr_evfid, ~0x4);
	ClearEventFlag(g_cdvdman_intr_evfid, ~0x10);
	SetEventFlag(g_cdvdman_intr_evfid, 0x29);
	SetEventFlag(g_ncmd_evfid, 1);
	SetEventFlag(g_scmd_evfid, 1);
	SetEventFlag(g_sfile_evfid, 1);
	SetEventFlag(g_fio_fsv_evfid, 1);
#ifdef CDVD_VARIANT_XOSD
	SetEventFlag(g_cdvdman_csys_evfid, 1);
#endif
#ifdef CDVD_VARIANT_OSD
	sema.attr = 1;
	sema.initial = 1;
	sema.max = 1;
	sema.option = 0;
	g_cdvdman_apply_scmd_sema = CreateSema(&sema);
#endif
	g_cdvdman_spinnom = -1;
	g_cdvdman_trycnt = -1;
	sceCdSC(0xFFFFFFF3, &scres_unused);
	for ( i = 0; i < (sizeof(g_cdvdman_fhinfo) / sizeof(g_cdvdman_fhinfo[0])); i += 1 )
	{
		g_cdvdman_fhinfo[i].m_fd_flags = 0;
	}
#ifdef CDVD_VARIANT_XOSD
	set_cdvd_dev5_base_addr_atapi(2);

	for ( i = 0; i < 600 && !dev5_mmio_hwport->m_dev5_reg_015; i += 1 )
	{
		DelayThread(16000);
	}
	g_cdvdman_istruct.m_cd_mode_ps2_atapi =
		(!(dev5_mmio_hwport->m_dev5_reg_00A & SCECdStatShellOpen) && !(dev5_mmio_hwport->m_dev5_reg_015 & 0x80)
		 && dev5_mmio_hwport->m_dev5_reg_00F != 6) ?
			2 :
			1;
	KPRINTF("Mode %s Drive\n", g_cdvdman_istruct.m_cd_mode_ps2_atapi == 1 ? "Atapi" : "PS2");
	set_cdvd_dev5_base_addr_atapi(g_cdvdman_istruct.m_cd_mode_ps2_atapi);
#endif
	return 0;
}

void cdvdman_termcall(int with_stop)
{
	int i;
	int oldstate;
	USE_DEV5_MMIO_HWPORT();

	VERBOSE_KPRINTF(1, "CDVD:library Terminate Call %d\n", with_stop);
#ifdef CDVD_VARIANT_XOSD
	if ( update_cd_mode_ps2_atapi() == 1 )
	{
		set_cdvd_dev5_base_addr_atapi(2);
	}
#endif
	if ( with_stop )
	{
		return;
	}
	sceCdBreak();
	sceCdSync(0);
#ifndef CDVD_VARIANT_XOSD
	if ( g_cdvdman_istruct.m_cd_inited )
	{
		cdvdman_ncmd_sender_06();
	}
#endif
	for ( i = 0; i < 50000 && (dev5_mmio_hwport->m_dev5_reg_017 & 0x80); i += 1 )
	{
		DelayThread(100);
	}
	sceCdDecSet(0, 0, 0);
	if ( (dmac_ch_get_chcr(3) & 0x1000000) )
	{
		dev5_mmio_hwport->m_dev5_reg_007 = 1;
	}
	dmac_ch_set_chcr(3, 0);
	DisableIntr(IOP_IRQ_DMA_CDVD, &oldstate);
	ReleaseIntrHandler(IOP_IRQ_DMA_CDVD);
	DisableIntr(IOP_IRQ_CDVD, &oldstate);
	ReleaseIntrHandler(IOP_IRQ_CDVD);
}

static int cdrom_deinit(iop_device_t *dev)
{
	unsigned int i;

	(void)dev;

	for ( i = 0; i < (sizeof(g_cdvdman_fhinfo) / sizeof(g_cdvdman_fhinfo[0])); i += 1 )
	{
		g_cdvdman_fhinfo[i].m_fd_flags = 0;
	}
	DeleteEventFlag(g_fio_fsv_evfid);
	DeleteEventFlag(g_cdvdman_intr_evfid);
	DeleteEventFlag(g_ncmd_evfid);
	DeleteEventFlag(g_scmd_evfid);
	DeleteEventFlag(g_sfile_evfid);
#ifdef CDVD_VARIANT_XOSD
	DeleteEventFlag(g_cdvdman_csys_evfid);
#endif
	return 0;
}

static int cdvdman_devready(void)
{
	int i;

	for ( i = 0; i < 100; i += 1 )
	{
		// The following direct register access was replaced with call for ioprp300x
		if ( (sceCdStatus() & SCECdStatShellOpen) )
		{
			g_cdvdman_iocache = 0;
			return -EIO;
		}
		// The following direct register access was replaced with call for ioprp300x
		if ( (sceCdDiskReady(8) & 0xC0) == 0x40 && !g_cdvdman_istruct.m_read2_flag && !g_cdvdman_ee_rpc_fno )
		{
			return 1;
		}
		DelayThread(10000);
	}
	return -EBUSY;
}

static int cdvdman_l0check(int layer)
{
	return !layer
			&& (g_cdvdman_istruct.m_dual_layer_emulation || g_cdvdman_istruct.m_opo_or_para == 1 || g_cdvdman_istruct.m_opo_or_para == 2);
}

static void cdvdman_iormode(sceCdRMode *rmode, int fmode, int layer)
{
	rmode->datapattern = SCECdSecS2048;
	rmode->trycount = (g_cdvdman_trycnt == -1) ? 16 : g_cdvdman_trycnt;
	if ( cdvdman_l0check(layer) )
	{
		if ( g_cdvdman_spinnom == -1 )
		{
			rmode->spindlctrl = (fmode == SCECdSpinX1 || fmode == SCECdSpinMx) ? fmode : SCECdSpinStm;
			return;
		}
		if ( fmode != SCECdSpinX1 && fmode != SCECdSpinMx )
		{
			rmode->spindlctrl = SCECdSpinStm;
			return;
		}
		rmode->spindlctrl = (u8)g_cdvdman_spinnom;
	}
	else if ( g_cdvdman_spinnom == -1 )
	{
		switch ( fmode )
		{
			case SCECdSpinStm:
				rmode->spindlctrl = SCECdSpinStm;
				break;
			case SCECdSpinNom:
				rmode->spindlctrl = SCECdSpinNom;
				break;
			case SCECdSpinX1:
				rmode->spindlctrl = SCECdSpinX1;
				break;
			case SCECdSpinX2:
				rmode->spindlctrl = SCECdSpinX2;
				break;
			case SCECdSpinX4:
				rmode->spindlctrl = SCECdSpinX4;
				break;
			case SCECdSpinX12:
				rmode->spindlctrl = SCECdSpinX12;
				break;
			case SCECdSpinMx:
				rmode->spindlctrl = SCECdSpinMx;
				break;
			default:
				rmode->spindlctrl = SCECdSpinNom;
				break;
		}
	}
}

static int cdrom_dopen(iop_file_t *f, const char *dirname)
{
	unsigned int i;
	int is_devready;
	size_t path_name_ind;
	int file_lsn_tmp;
	char path_name[128];
	u32 efbits;

	VERBOSE_PRINTF(1, "fileIO DOPEN name= %s layer %d\n", dirname, f->unit);
	WaitEventFlag(g_fio_fsv_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	for ( i = 0; (i < (sizeof(g_cdvdman_fhinfo) / sizeof(g_cdvdman_fhinfo[0]))) && g_cdvdman_fhinfo[i].m_fd_flags;
				i += 1 )
	{
	}
	if ( i == (sizeof(g_cdvdman_fhinfo) / sizeof(g_cdvdman_fhinfo[0])) )
	{
		SetEventFlag(g_fio_fsv_evfid, 1);
		return -EMFILE;
	}
	f->privdata = (void *)i;
	is_devready = cdvdman_devready();
	if ( is_devready < 0 )
	{
		SetEventFlag(g_fio_fsv_evfid, 1);
		return is_devready;
	}
	strncpy(path_name, dirname, sizeof(path_name));
	if ( !strcmp(path_name, ".") )
	{
		strcpy(path_name, "\\.");
	}
	path_name_ind = strlen(path_name);
	path_name_ind -= (path_name_ind >= 2) ? 2 : 0;
	if ( strcmp(&path_name[path_name_ind], "\\.") )
	{
		strcat(path_name, "\\.");
	}
	if ( (unsigned int)(f->unit) >= 2 )
	{
		PRINTF("open fail name %s\n", path_name);
		SetEventFlag(g_fio_fsv_evfid, 1);
		return -ENOENT;
	}
	g_cdvdman_fhinfo[i].m_file_lsn = 0;
	g_cdvdman_srchspd = 0;
	file_lsn_tmp = sceCdSearchDir(path_name, f->unit);
	if ( file_lsn_tmp < 0 )
	{
		PRINTF("open fail directory %s\n", path_name);
		SetEventFlag(g_fio_fsv_evfid, 1);
		return -ENOENT;
	}
	g_cdvdman_fhinfo[i].m_file_lsn = file_lsn_tmp;
	g_cdvdman_fhinfo[i].m_read_pos = 0;
	g_cdvdman_fhinfo[i].m_filemode = 0;
	g_cdvdman_fhinfo[i].m_fd_flags = 1;
	g_cdvdman_fhinfo[i].m_fd_layer = f->unit;
	SetEventFlag(g_fio_fsv_evfid, 1);
	return 0;
}

static void cdvdman_fillstat(void *dummy, iox_stat_t *buf, cdvdman_filetbl_entry_t *fp)
{
	unsigned int i;

	(void)dummy;

	buf->attr = 0;
	buf->private_5 = 0;
	buf->private_4 = 0;
	buf->private_3 = 0;
	buf->private_2 = 0;
	buf->private_1 = 0;
	buf->private_0 = 0;
	buf->hisize = 0;
	for ( i = 0; i < (sizeof(buf->mtime) / sizeof(buf->mtime[0])); i += 1 )
	{
		buf->mtime[i] = fp->m_file_struct.date[i];
		buf->atime[i] = fp->m_file_struct.date[i];
		buf->ctime[i] = fp->m_file_struct.date[i];
	}
	buf->size = fp->m_file_struct.size;
	buf->mode = (((fp->m_flags & 2)) ? (FIO_S_IFDIR | FIO_S_IXUSR | FIO_S_IXGRP | FIO_S_IXOTH) : FIO_S_IFREG)
						| (FIO_S_IRUSR | FIO_S_IRGRP | FIO_S_IROTH);
}

static int cdvdman_cdfname(char *filename)
{
	size_t filename_len;

	filename_len = strlen(filename);
	if ( filename_len >= 3 && !(filename[filename_len - 2] != ';' && filename[filename_len - 1] != '1') )
	{
		return 0;
	}
	strcat(filename, ";1");
	return 1;
}

static int cdrom_getstat(iop_file_t *f, const char *name, iox_stat_t *buf)
{
	int devready_tmp;
	cdvdman_filetbl_entry_t fp;
	char filename[128];
	u32 efbits;

	VERBOSE_PRINTF(1, "fileIO GETSTAT name= %s layer= %d\n", name, f->unit);
	WaitEventFlag(g_fio_fsv_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	devready_tmp = cdvdman_devready();
	if ( devready_tmp < 0 )
	{
		SetEventFlag(g_fio_fsv_evfid, 1);
		return devready_tmp;
	}
	strncpy(filename, name, sizeof(filename));
	if ( !strcmp(filename, ".") )
	{
		strcpy(filename, "\\.");
	}
	if ( !strcmp(filename, "\\") )
	{
		strcpy(filename, "\\.");
	}
	if ( !strlen(filename) )
	{
		strcpy(filename, "\\.");
	}
	g_cdvdman_srchspd = 0;
	// Unofficial: initialize to 0
	memset(&fp, 0, sizeof(fp));
	if (
		!sceCdLayerSearchFile(&fp.m_file_struct, filename, f->unit)
		&& !(cdvdman_cdfname(filename) && sceCdLayerSearchFile(&fp.m_file_struct, filename, f->unit)) )
	{
		PRINTF("open fail name %s\n", name);
		SetEventFlag(g_fio_fsv_evfid, 1);
		return -ENOENT;
	}
	cdvdman_fillstat(filename, buf, &fp);
	SetEventFlag(g_fio_fsv_evfid, 1);
	return 1;
}

// cppcheck-suppress constParameterCallback
static int cdrom_dread(iop_file_t *f, iox_dirent_t *buf)
{
	int devready_tmp;
	cdvdman_fhinfo_t *fh;
	cdvdman_filetbl_entry_t fp;
	u32 efbits;

	memset(&fp, 0, sizeof(fp));
	VERBOSE_PRINTF(1, "fileIO DREAD\n");
	WaitEventFlag(g_fio_fsv_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	devready_tmp = cdvdman_devready();
	if ( devready_tmp < 0 )
	{
		SetEventFlag(g_fio_fsv_evfid, 1);
		return devready_tmp;
	}
	fh = &g_cdvdman_fhinfo[(int)f->privdata];
	g_cdvdman_srchspd = 0;
	devready_tmp = sceCdReadDir(&fp.m_file_struct, fh->m_file_lsn, fh->m_read_pos, fh->m_fd_layer);
	if ( devready_tmp < 0 )
	{
		SetEventFlag(g_fio_fsv_evfid, 1);
		return -ENOENT;
	}
	if ( devready_tmp )
	{
		fh->m_read_pos += 1;
		devready_tmp = strlen(fp.m_file_struct.name);
	}
	strncpy(buf->name, fp.m_file_struct.name, sizeof(buf->name));
	cdvdman_fillstat(fp.m_file_struct.name, &buf->stat, &fp);
	SetEventFlag(g_fio_fsv_evfid, 1);
	return devready_tmp;
}

static int cdvd_odcinit(cdvdman_fhinfo_t *fh, int open_or_close, int id)
{
	int cache_remove_result;
	int cache_result;
	int cache_file_fd_new;
	u32 file_size_bsr_3;
	unsigned int file_size_bsr_17;
	char cache_filename[512];
	int state;
	unsigned int ioctl_arg;

	g_cdvdman_iocache = 0;
	sprintf(
		cache_filename,
		"%sCache_%d_%d_%d_%d",
		g_cdvdman_cache_name,
		fh->m_fd_layer,
		(int)fh->m_file_lsn,
		(int)fh->m_file_size,
		id);
	cache_remove_result = 0;
	VERBOSE_KPRINTF(1, "Cachefile:%s Open_or_Close:%d\n", cache_filename, open_or_close);
	cache_result = 0;
	if ( open_or_close )
	{
		u32 i;

		CpuSuspendIntr(&state);
		fh->m_fd_rbsize = g_cdvdman_cache_sector_size_count ? (g_cdvdman_cache_sector_size_count << 11) : 0x800;
		fh->m_fd_rcvbuf = (u8 *)AllocSysMemory(ALLOC_LAST, fh->m_fd_rbsize, 0);
		if ( !fh->m_fd_rcvbuf )
		{
			VERBOSE_KPRINTF(1, "Rcvbuf MemAlloc Fail\n");
			CpuResumeIntr(state);
			return -ENOMEM;
		}
		CpuResumeIntr(state);
		fh->m_cache_file_fd = -1;
		cache_result = open(cache_filename, O_TRUNC | O_CREAT | O_RDWR, 0x1ff /* 0o777 */);
		cache_file_fd_new = cache_result;
		if ( cache_result >= 0 )
		{
			u32 file_size_sectors;
			unsigned int file_size_bsr_6;

			file_size_sectors = (fh->m_file_size >> 11) + ((!!((fh->m_file_size & 0x7FF))));
			file_size_bsr_3 = (file_size_sectors >> 3) + (!!(file_size_sectors & 7));
			file_size_bsr_6 = (file_size_bsr_3 >> 3) + (!!((file_size_bsr_3 & 7)));
			file_size_bsr_17 = (file_size_bsr_6 >> 11) + (!!((file_size_bsr_6 & 0x7FF)));
			ioctl_arg = (file_size_bsr_17 + file_size_sectors + 8) << 11;
			for ( i = 0; i < fh->m_fd_rbsize; i += 1 )
			{
				fh->m_fd_rcvbuf[i] = 0;
			}
			if ( !strncmp(cache_filename, "pfs", 3) )
			{
				cache_result = ioctl2(cache_file_fd_new, PIOCALLOC, &ioctl_arg, 4, 0, 0);
			}
		}
		if ( cache_result >= 0 )
		{
			cache_result = lseek(cache_file_fd_new, 0, 0);
		}
		if ( cache_result >= 0 )
		{
			if ( !g_cdvdman_cache_sector_count )
				__builtin_trap();
			for ( i = 0; (int)i < (int)(g_cdvdman_cache_sector_count << 11); i += 1 )
			{
				((char *)g_cdvdman_temp_buffer_ptr)[i] = 0;
			}
			for ( i = 0; (int)i < (int)((ioctl_arg >> 11) / g_cdvdman_cache_sector_count); i += 1 )
			{
				cache_result = write(cache_file_fd_new, g_cdvdman_temp_buffer_ptr, g_cdvdman_cache_sector_count << 11);
				if ( cache_result != (g_cdvdman_cache_sector_count << 11) )
				{
					if ( cache_result >= 0 )
					{
						cache_result = -EIO;
					}
					break;
				}
			}
		}
		if ( cache_result >= 0 )
		{
			for ( i = 0; (int)i < (int)((ioctl_arg >> 11)
																	- g_cdvdman_cache_sector_count * ((ioctl_arg >> 11) / g_cdvdman_cache_sector_count));
						i += 1 )
			{
				cache_result = write(cache_file_fd_new, fh->m_fd_rcvbuf, 0x800);
				if ( cache_result != 0x800 )
				{
					if ( cache_result >= 0 )
					{
						cache_result = -EIO;
					}
					break;
				}
			}
		}
	}
	if ( !open_or_close || cache_result < 0 )
	{
		if ( fh->m_cache_file_fd != -1 )
		{
			cache_remove_result = close(fh->m_cache_file_fd);
			VERBOSE_KPRINTF(1, "Cache File Close: %d\n", cache_remove_result);
			if ( cache_remove_result >= 0 )
			{
				if ( !strncmp(cache_filename, "pfs", 3) )
				{
					cache_remove_result = remove(cache_filename);
				}
				else if ( !strncmp(cache_filename, "host", 4) )
				{
					cache_remove_result = 0;
					remove(cache_filename);
				}
				VERBOSE_KPRINTF(1, "Cache File %s remove: %d\n", cache_filename, cache_remove_result);
			}
		}
		fh->m_cache_file_fd = -1;
		fh->m_max_cluster = 0;
		fh->m_cluster_cur = -1;
		fh->m_sector_count_total = 0;
		CpuSuspendIntr(&state);
		FreeSysMemory(fh->m_fd_rcvbuf);
		CpuResumeIntr(state);
		fh->m_fd_rcvbuf = 0;
		if ( cache_result < 0 )
		{
			VERBOSE_KPRINTF(1, "cdvd_odcinit Open  Error %d\n", cache_result);
		}
		if ( cache_remove_result < 0 )
		{
			VERBOSE_KPRINTF(1, "cdvd_odcinit Close Error %d\n", cache_remove_result);
		}
		return (!open_or_close) ? cache_remove_result : cache_result;
	}
	fh->m_sector_count_total = file_size_bsr_17 << 11;
	fh->m_cache_file_fd = cache_file_fd_new;
	fh->m_max_cluster = (void *)file_size_bsr_3;
	fh->m_cluster_cur = -1;
	VERBOSE_KPRINTF(1, "Cache File Maked\n");
	return 0;
}

static int cdvdman_cache_invalidate(cdvdman_fhinfo_t *fh, int index)
{
	u32 i;
	int fileio_res;

	if ( fh->m_cluster_cur == -1 )
	{
		return 0;
	}
	fh->m_cluster_cur = -1;
	for ( i = 0; i < fh->m_fd_rbsize; i += 1 )
	{
		fh->m_fd_rcvbuf[i] = 0;
	}
	fileio_res = lseek(fh->m_cache_file_fd, 0, 0);
	if ( fileio_res >= 0 )
	{
		for ( i = 0; i < ((unsigned int)fh->m_sector_count_total >> 11); i += 1 )
		{
			fileio_res = write(fh->m_cache_file_fd, fh->m_fd_rcvbuf, 0x800);
			if ( fileio_res < 0 )
			{
				break;
			}
		}
	}
	if ( fileio_res >= 0 )
	{
		return fileio_res;
	}
	fh->m_fd_flags &= ~4;
	cdvd_odcinit(fh, 0, index);
	return fileio_res;
}

static int cdvdman_invcaches(void)
{
	unsigned int i;

	for ( i = 0; i < (sizeof(g_cdvdman_fhinfo) / sizeof(g_cdvdman_fhinfo[0])); i += 1 )
	{
		if ( (g_cdvdman_fhinfo[i].m_fd_flags & 4) )
		{
			cdvdman_cache_invalidate(&g_cdvdman_fhinfo[i], i);
		}
	}
	return 0;
}

static int cdrom_internal_cache_read(const iop_file_t *f, int nbytes)
{
	cdvdman_fhinfo_t *fh;
	s16 readpos_plus_nbytes;
	unsigned int readpos_plus_nbytes_bsr_14;
	int readpos_bsr_14;
	int cluster_cur;
	unsigned int i;

	fh = &g_cdvdman_fhinfo[(uiptr)f->privdata];
	if ( cdvdman_devready() < 0 )
	{
		g_cdvdman_iocache = 0;
		return -EBUSY;
	}
	if ( fh->m_cluster_cur == -1 )
	{
		if ( (void *)(8 * fh->m_fd_rbsize) < fh->m_max_cluster )
		{
			fh->m_cluster_cur = (fh->m_read_pos >> 14) & ~0x7;
			if (
				lseek(fh->m_cache_file_fd, fh->m_cluster_cur >> 3, 0) < 0
				|| read(fh->m_cache_file_fd, fh->m_fd_rcvbuf, fh->m_fd_rbsize) < 0 )
			{
				fh->m_cluster_cur = -1;
				return -EIO;
			}
		}
		else
		{
			fh->m_cluster_cur = -2;
		}
	}
	readpos_plus_nbytes = fh->m_read_pos + nbytes;
	readpos_plus_nbytes_bsr_14 = (readpos_plus_nbytes >> 14) - (!(readpos_plus_nbytes & 0x3FFF));
	readpos_bsr_14 = fh->m_read_pos >> 14;
	VERBOSE_KPRINTF(
		1, "max_claster %d meta_size_clst %d claster_cur %d\n", fh->m_max_cluster, 8 * fh->m_fd_rbsize, fh->m_cluster_cur);
	cluster_cur = fh->m_cluster_cur;
	if ( cluster_cur < 0 )
	{
		cluster_cur = 0;
	}
	else if (
		(unsigned int)readpos_bsr_14 < (unsigned int)cluster_cur
		|| readpos_plus_nbytes_bsr_14 >= cluster_cur + 8 * fh->m_fd_rbsize )
	{
		int cluster_write_tmp2;
		unsigned int readpos_band;

		if ( lseek(fh->m_cache_file_fd, cluster_cur >> 3, 0) < 0 )
		{
			fh->m_cluster_cur = -1;
			return -EIO;
		}
		cluster_write_tmp2 = (unsigned int)fh->m_max_cluster >= fh->m_cluster_cur + 8 * fh->m_fd_rbsize ?
													 fh->m_fd_rbsize :
													 ((unsigned int)fh->m_max_cluster - fh->m_cluster_cur + 7) >> 3;
		if ( write(fh->m_cache_file_fd, fh->m_fd_rcvbuf, cluster_write_tmp2) != cluster_write_tmp2 )
		{
			fh->m_cluster_cur = -1;
			return -EIO;
		}
		readpos_band = readpos_bsr_14 & ~0x7;
		fh->m_cluster_cur = readpos_band;
		readpos_band += (readpos_bsr_14 < 0) ? 7 : 0;
		if ( (lseek(fh->m_cache_file_fd, readpos_band >> 3, 0) < 0) || (read(fh->m_cache_file_fd, fh->m_fd_rcvbuf, ( (unsigned int)fh->m_max_cluster < fh->m_cluster_cur + 8 * fh->m_fd_rbsize ) ? (((unsigned int)fh->m_max_cluster - fh->m_cluster_cur + 7) >> 3) : (fh->m_fd_rbsize)) < 0) )
		{
			fh->m_cluster_cur = -1;
			return -EIO;
		}
		cluster_cur = fh->m_cluster_cur;
	}
	for ( i = readpos_bsr_14; i <= readpos_plus_nbytes_bsr_14; i += 1 )
	{
		if ( !(((int)fh->m_fd_rcvbuf[(i - cluster_cur) >> 3] >> ((i - cluster_cur) & 7)) & 1) )
		{
			break;
		}
	}
	return i <= readpos_plus_nbytes_bsr_14;
}

static int cdrom_internal_write_cache(const iop_file_t *f, unsigned int nbytes)
{
	int lseek_result;
	cdvdman_fhinfo_t *fh;
	unsigned int cur;
	unsigned int rst;
	int cluster_cur;
	int write_ret;
	unsigned int i;
	int tray_open;
	int last_err;
	sceCdRMode rmode;

	g_cdvdman_iocache = 0;
	if ( cdvdman_devready() < 0 )
	{
		return -EBUSY;
	}
	fh = &g_cdvdman_fhinfo[(uiptr)f->privdata];
	if ( nbytes > fh->m_file_size - fh->m_read_pos )
	{
		nbytes = fh->m_file_size - fh->m_read_pos;
	}
	if ( !nbytes )
	{
		return 0;
	}
	VERBOSE_KPRINTF(1, "_cdvdfile_cache_read %d<->%d\n", fh->m_read_pos, fh->m_read_pos + nbytes);
	cur = ((fh->m_read_pos + nbytes) >> 14) - (!((fh->m_read_pos + nbytes) & 0x3FFF));
	rst = fh->m_read_pos >> 14;
	cluster_cur = (fh->m_cluster_cur >= 0) ? fh->m_cluster_cur : 0;
	cdvdman_iormode(&rmode, fh->m_filemode, f->unit);
	write_ret = 0;
	VERBOSE_KPRINTF(1, "cache_fill rst:%d<->%d cur:%d cnt:%d\n", rst, cur, fh->m_read_pos, nbytes);
	for ( i = rst; i <= cur; i += 1 )
	{
		VERBOSE_KPRINTF(
			1,
			"FIO Usr addr LSN:%d SEC:%d ADDR:%08x cpos= %d\n",
			fh->m_file_lsn + 8 * i,
			8,
			g_cdvdman_temp_buffer_ptr,
			(i * 0x4000) + fh->m_sector_count_total);
		if ( !(((int)fh->m_fd_rcvbuf[(i - cluster_cur) >> 3] >> ((i - cluster_cur) & 7)) & 1) )
		{
			tray_open = 0;
			while ( !sceCdRE(fh->m_file_lsn + 8 * i, 8, g_cdvdman_temp_buffer_ptr, &rmode) )
			{
				if ( (sceCdStatus() & SCECdStatShellOpen) )
				{
					g_cdvdman_iocache = 0;
					tray_open = 1;
					break;
				}
				DelayThread(10000);
			}
			sceCdSync(0);
			last_err = sceCdGetError();
			if ( last_err != SCECdErNO || tray_open )
			{
				// The following Kprintf was modified for ioprp300x
				VERBOSE_KPRINTF(0, "Read Error= 0x%02x Tray open %d\n", last_err, tray_open);
				return -ECOMM;
			}
			lseek_result = lseek(fh->m_cache_file_fd, (i * 0x4000) + fh->m_sector_count_total, 0);
			if ( lseek_result < 0 )
			{
				return lseek_result;
			}
			write_ret = write(fh->m_cache_file_fd, g_cdvdman_temp_buffer_ptr, 0x4000);
			if ( write_ret != 0x4000 )
			{
				VERBOSE_KPRINTF(1, "write: ret:%d\n", write_ret);
				if ( write_ret >= 0 )
				{
					return -EIO;
				}
				break;
			}
			fh->m_fd_rcvbuf[(i - cluster_cur) >> 3] |= 1 << ((i - cluster_cur) & 7);
		}
	}
	return write_ret;
}

static int cdvdfile_cache_read(const iop_file_t *f, void *buf, int nbyte)
{
	int nbyte_tmp;
	int fd_result;
	cdvdman_fhinfo_t *fh;

	if ( nbyte < 0 )
	{
		return -EINVAL;
	}
	fh = &g_cdvdman_fhinfo[(uiptr)f->privdata];
	nbyte_tmp =
		((unsigned int)nbyte > fh->m_file_size - fh->m_read_pos) ? fh->m_file_size - fh->m_read_pos : (unsigned int)nbyte;
	VERBOSE_KPRINTF(1, "_cdvdfile_cache_read %d<->%d\n", fh->m_read_pos, fh->m_read_pos + nbyte_tmp);
	fd_result = 0;
	if ( nbyte_tmp > 0 )
	{
		fd_result = lseek(fh->m_cache_file_fd, fh->m_read_pos + fh->m_sector_count_total, 0);
		if ( fd_result >= 0 )
		{
			fd_result = read(fh->m_cache_file_fd, buf, nbyte_tmp);
			fh->m_read_pos += (fd_result >= 0) ? fd_result : 0;
		}
	}
	return fd_result;
}

static int cdvdfile_cache_fill_read(const iop_file_t *f, void *buf, int nbytes)
{
	int op_result;

	op_result = cdvdman_devready();
	if ( op_result >= 0 )
	{
		op_result = cdrom_internal_write_cache(f, nbytes);
	}
	else
	{
		g_cdvdman_iocache = 0;
	}
	if ( op_result >= 0 )
	{
		op_result = cdvdfile_cache_read(f, buf, nbytes);
	}
	return op_result;
}

static int cdrom_open(iop_file_t *f, const char *name, int mode, int arg4)
{
	int fds1;
	unsigned int i;
	int emptyfdfound;
	int streamfdfound;
	cdvdman_fhinfo_t *fh;
	int devready_tmp;
	char filename[128];
	sceCdlFILE fp;
	u32 efbits;

	(void)arg4;
	devready_tmp = 0;
	fds1 = 0;
	VERBOSE_PRINTF(1, "fileIO OPEN name= %s mode= 0x%08x layer %d\n", name, mode, f->unit);
	WaitEventFlag(g_fio_fsv_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	emptyfdfound = 0;
	streamfdfound = 0;
	for ( i = 0; i < (sizeof(g_cdvdman_fhinfo) / sizeof(g_cdvdman_fhinfo[0])); i += 1 )
	{
		if ( !g_cdvdman_fhinfo[i].m_fd_flags && !emptyfdfound )
		{
			fds1 = i;
			emptyfdfound = 1;
		}
		if ( (g_cdvdman_fhinfo[i].m_fd_flags & 8) )
		{
			streamfdfound = 1;
		}
	}
	if ( !emptyfdfound || streamfdfound )
	{
		PRINTF("open fail name %s\n", name);
		SetEventFlag(g_fio_fsv_evfid, 1);
		return -EMFILE;
	}
	f->privdata = (void *)fds1;
	fh = &g_cdvdman_fhinfo[(uiptr)f->privdata];
	strncpy(filename, name, sizeof(filename));
	cdvdman_cdfname(filename);
	g_cdvdman_srchspd = (!(mode & 0x40000000) && !cdvdman_l0check(f->unit)) ?
												((g_cdvdman_spinnom == -1) ? (u16)mode : g_cdvdman_spinnom != SCECdSpinStm) :
												0;
	if ( (unsigned int)(f->unit) >= 2u )
	{
		devready_tmp = -ENOENT;
	}
	else
	{
		if ( !strncmp(name, "sce_cdvd_lsn", 12) )
		{
			strncpy(filename, name, sizeof(filename));
			// Unofficial: Avoid out of bounds access
			for ( i = 12; i < (sizeof(filename) - 5) && filename[i] && filename[i] != '_'; i += 1 )
				;
			if ( !filename[i] || i >= (sizeof(filename) - 5) )
			{
				devready_tmp = -ENOENT;
			}
			else
			{
				fp.size = strtol(&filename[i + 5], 0, 10);
				filename[i] = 0;
				fp.lsn = strtol(&filename[12], 0, 10);
				if ( f->unit )
				{
					if ( cdvdman_devready() < 0 || !DvdDual_infochk() )
					{
						devready_tmp = -ENOENT;
					}
					else
					{
						fp.lsn += g_cdvdman_istruct.m_layer_1_lsn;
					}
				}
			}
		}
#ifdef CDVD_VARIANT_OSD
		else if ( !strcmp(filename, "sce_dev5;1") )
		{
			fp.lsn = -1;
			fp.size = 0;
		}
#endif
		else
		{
			devready_tmp = cdvdman_devready();
			if ( devready_tmp < 0 )
			{
				SetEventFlag(g_fio_fsv_evfid, 1);
				return devready_tmp;
			}
			if ( !sceCdLayerSearchFile(&fp, filename, f->unit) )
			{
				devready_tmp = -ENOENT;
			}
		}
	}
	if ( devready_tmp < 0 )
	{
		PRINTF("open fail name %s\n", filename);
		SetEventFlag(g_fio_fsv_evfid, 1);
		return devready_tmp;
	}
	fh->m_fd_flags = 1;
	g_cdvdman_srchspd = 0;
	if ( (mode & 0x40000000) )
	{
		sceCdRMode rmode;
		memset(&rmode, 0, sizeof(rmode));
		rmode.datapattern = SCECdSecS2048;
		rmode.spindlctrl = SCECdSpinStm;
		rmode.trycount = (g_cdvdman_trycnt != -1) ? g_cdvdman_trycnt : 0;
		// The following call to sceCdStStart was inlined
		if ( !sceCdStStart(fp.lsn, &rmode) )
		{
			fh->m_fd_flags = 0;
			SetEventFlag(g_fio_fsv_evfid, 1);
			return -ENOENT;
		}
		g_cdvdman_strmerr = 0;
		fh->m_fd_flags |= 8;
	}
	fh->m_file_lsn = fp.lsn;
	fh->m_read_pos = 0;
	fh->m_filemode = mode & ~0x40000000;
	fh->m_file_size = fp.size;
	fh->m_fd_layer = f->unit;
	if ( (mode & 0x50000000) == 0x10000000 )
	{
		devready_tmp = -ENODEV;
		if ( g_cache_path_fd != -1 )
		{
			devready_tmp = cdvd_odcinit(fh, 1, (int)f->privdata);
			if ( devready_tmp >= 0 )
			{
				fh->m_fd_flags |= 4;
			}
		}
	}
	if ( devready_tmp < 0 )
	{
		fh->m_fd_flags = 0;
		fh->m_fd_layer = 0;
		fh->m_filemode = 0;
		fh->m_read_pos = 0;
		fh->m_file_size = 0;
		fh->m_file_lsn = 0;
		SetEventFlag(g_fio_fsv_evfid, 1);
		return devready_tmp;
	}
	f->mode = O_RDONLY;
	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(1, "open end file_lbn= %d size= %d fds= %d\n", fh->m_file_lsn, fh->m_file_size, fds1);
	SetEventFlag(g_fio_fsv_evfid, 1);
	return 0;
}

static int cdrom_close(iop_file_t *f)
{
	cdvdman_fhinfo_t *fh;
	u32 efbits;

	VERBOSE_PRINTF(1, "fileIO CLOSE\n");
	WaitEventFlag(g_fio_fsv_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	fh = &g_cdvdman_fhinfo[(uiptr)f->privdata];
	if ( (fh->m_fd_flags & 8) )
	{
		g_cdvdman_strmerr = 0;
		// The following call to sceCdStStop was inlined
		if ( !sceCdStStop() )
		{
			SetEventFlag(g_fio_fsv_evfid, 1);
			return -EIO;
		}
	}
	if ( ((fh->m_fd_flags & 0xC) == 4 && cdvd_odcinit(fh, 0, (int)f->privdata) < 0) )
	{
		SetEventFlag(g_fio_fsv_evfid, 1);
		return -EIO;
	}
	fh->m_file_lsn = 0;
	fh->m_file_size = 0;
	fh->m_read_pos = 0;
	fh->m_fd_flags = 0;
	fh->m_fd_layer = 0;
	fh->m_filemode = 1;
	f->mode = 0;
	SetEventFlag(g_fio_fsv_evfid, 1);
	return 0;
}

static int cdrom_internal_read(const iop_file_t *f, char *addr, int nbytes)
{
	unsigned int sectors;
	int op_result;
	int nbytes_segment;
	int nbytes_div_2048;
	unsigned int sec;
	unsigned int sectors_count;
	int last_err;
	sceCdRMode rmode;
	int i;
	unsigned int filesize_bsr_11;
	cdvdman_fhinfo_t *fh;
	void *buf;

	sectors = 1;
	VERBOSE_PRINTF(1, "cdvd fileIO read start\n");
	op_result = cdvdman_devready();
	if ( op_result < 0 )
	{
		g_cdvdman_iocache = 0;
		return op_result;
	}
	fh = &g_cdvdman_fhinfo[(uiptr)f->privdata];
	cdvdman_iormode(&rmode, fh->m_filemode, f->unit);
	if ( nbytes < 0 )
	{
		return -EINVAL;
	}
	if ( (unsigned int)nbytes > fh->m_file_size - fh->m_read_pos )
	{
		nbytes = fh->m_file_size - fh->m_read_pos;
	}
	filesize_bsr_11 = (fh->m_file_size >> 11) + (!!((fh->m_file_size & 0x7FF))) + fh->m_file_lsn;
	VERBOSE_PRINTF(
		1, "fds= %d read file_lbn= %d offset= %d\n", (int)(uiptr)f->privdata, (int)fh->m_file_lsn, (int)fh->m_read_pos);
	buf = 0;
	for ( i = 0; i < nbytes; i += nbytes_segment )
	{
		unsigned int lbn;
		int pos_extra;
		int lbn_tmp;
		int pos_sub_2048;
		int nbytes_cur;

		nbytes_cur = nbytes - i;
		if ( g_cdvdman_spinctl != -1 )
		{
			rmode.spindlctrl = g_cdvdman_spinctl;
			switch ( g_cdvdman_spinctl )
			{
				case SCECdSpinStm:
					fh->m_filemode = 0;
					break;
				case SCECdSpinNom:
					fh->m_filemode = 1;
					break;
				case SCECdSpinX1:
					fh->m_filemode = 2;
					break;
				case SCECdSpinX2:
					fh->m_filemode = 3;
					break;
				case SCECdSpinX4:
					fh->m_filemode = 4;
					break;
				case SCECdSpinX12:
					fh->m_filemode = 5;
					break;
				default:
					fh->m_filemode = 1;
					break;
			}
		}
		pos_sub_2048 = 0;
		lbn = fh->m_file_lsn + ((fh->m_read_pos + i) >> 11);
		lbn_tmp = lbn;
		pos_extra = (fh->m_read_pos + i) & 0x7FF;
		if ( nbytes_cur <= 0x4000 )
		{
			nbytes_segment = nbytes_cur;
			nbytes_div_2048 = (nbytes_cur / 0x800) + (!!((nbytes_cur & 0x7FF)));
			sectors = nbytes_div_2048 + (!!pos_extra);
		}
		else
		{
			nbytes_segment = 0x4000;
			if ( buf && ((fh->m_read_pos + i) & 0x7FF) && g_cdvdman_iocache )
			{
				lbn += 1;
				pos_sub_2048 = 0x800 - pos_extra;
				optimized_memcpy(
					&addr[i], &((const char *)g_cdvdman_temp_buffer_ptr)[0x800 * (sectors - 1) + pos_extra], 0x800 - pos_extra);
				g_cdvdman_iocache = 0;
				sectors = 8;
			}
			else
			{
				sectors = 8 + (!!(((fh->m_read_pos + i) & 0x7FF)));
			}
		}
		buf = g_cdvdman_temp_buffer_ptr;
		if ( (unsigned int)(sectors) > filesize_bsr_11 - lbn )
		{
			sectors = filesize_bsr_11 - lbn;
		}
		VERBOSE_PRINTF(1, "sce_Read LBN= %d sectors= %d\n", (int)lbn, (int)sectors);
		if (
			g_cdvdman_iocache && (lbn >= g_cdvdman_lcn_offset)
			&& (g_cdvdman_lcn_offset + g_cdvdman_numbytes_offset >= lbn + sectors) )
		{
			optimized_memcpy(&addr[i], &((char *)buf)[0x800 * (lbn - g_cdvdman_lcn_offset) + pos_extra], nbytes_segment);
		}
		else if ( ((uiptr)(addr + i) & 3) || pos_extra || (nbytes_segment != 0x4000) )
		{
			sec = (sectors >= 8) ? sectors : 8;
			if ( sec > filesize_bsr_11 - lbn )
			{
				sec = filesize_bsr_11 - lbn;
			}
			VERBOSE_PRINTF(1, "FIO Cache LSN:%d SEC:%d ADDR:%08x\n", (int)lbn, (int)sec, (unsigned int)(uiptr)buf);
			sectors_count = (sec >= 9) ? (sec - 8) : 0;
			if ( sectors_count )
			{
				while ( !sceCdRE(lbn, sectors_count, buf, &rmode) )
				{
					VERBOSE_PRINTF(1, "sce_Read ON Delay\n");
					if ( (sceCdStatus() & SCECdStatShellOpen) )
					{
						g_cdvdman_iocache = 0;
						g_cdvdman_spinctl = -1;
						return -EIO;
					}
					DelayThread(10000);
				}
				sceCdSync(0);
				last_err = sceCdGetError();
				if ( last_err != SCECdErNO )
				{
					PRINTF("Read Error= 0x%02x\n", last_err);
					g_cdvdman_iocache = 0;
					break;
				}
			}
			while ( !sceCdRE(lbn + sectors_count, sec - sectors_count, (char *)buf + 0x800 * sectors_count, &rmode) )
			{
				VERBOSE_PRINTF(1, "sce_Read ON Delay\n");
				if ( (sceCdStatus() & SCECdStatShellOpen) )
				{
					g_cdvdman_iocache = 0;
					g_cdvdman_spinctl = -1;
					return -EIO;
				}
				DelayThread(10000);
			}
			sceCdSync(0);
			last_err = sceCdGetError();
			if ( last_err != SCECdErNO )
			{
				PRINTF("Read Error= 0x%02x\n", last_err);
				g_cdvdman_iocache = 0;
				break;
			}
			g_cdvdman_lcn_offset = lbn_tmp;
			g_cdvdman_numbytes_offset = sec;
			g_cdvdman_iocache = 1;
			optimized_memcpy(
				&addr[pos_sub_2048 + i],
				&((const char *)g_cdvdman_temp_buffer_ptr)[!pos_sub_2048 ? pos_extra : 0],
				nbytes_segment - pos_sub_2048);
		}
		else
		{
			VERBOSE_PRINTF(
				1, "FIO Usr addr LSN:%d SEC:%d ADDR:%08x\n", (int)lbn, (int)sectors, (unsigned int)(uiptr)(addr + i));
			while ( !sceCdRE(lbn, sectors, addr + i, &rmode) )
			{
				VERBOSE_PRINTF(1, "sce_Read ON Delay\n");
				if ( (sceCdStatus() & SCECdStatShellOpen) )
				{
					g_cdvdman_iocache = 0;
					g_cdvdman_spinctl = -1;
					return -EIO;
				}
				DelayThread(10000);
			}
			sceCdSync(0);
			last_err = sceCdGetError();
			g_cdvdman_iocache = 0;
			if ( last_err != SCECdErNO )
			{
				PRINTF("Read Error= 0x%02x\n", last_err);
				break;
			}
		}
		if ( nbytes_segment <= 0 )
		{
			g_cdvdman_spinctl = -1;
			return nbytes_segment;
		}
	}
	fh->m_read_pos += i;
	VERBOSE_PRINTF(1, "fileIO read ended\n");
	g_cdvdman_spinctl = -1;
	return i;
}

static int cdrom_stream_read(const iop_file_t *f, char *bbuf, int nbytes)
{
	cdvdman_fhinfo_t *fh;
	sceCdRMode rmode;
	int buf;

	fh = &g_cdvdman_fhinfo[(uiptr)f->privdata];
	g_cdvdman_strmerr = 0;
	cdvdman_iormode(&rmode, fh->m_filemode, f->unit);
	rmode.spindlctrl = SCECdSpinStm;
	rmode.trycount = 0;
	// The following sceCdStRead call was inlined
	buf = sceCdStRead(nbytes >> 11, (u32 *)bbuf, STMNBLK, (u32 *)&g_cdvdman_strmerr);
	fh->m_read_pos += buf << 11;
	return buf << 11;
}

// cppcheck-suppress constParameterCallback
static int cdrom_read(iop_file_t *f, void *buf, int nbytes)
{
	cdvdman_fhinfo_t *fh;
	int rc;
	u32 efbits;

	if ( nbytes < 0 )
	{
		return -EINVAL;
	}
	WaitEventFlag(g_fio_fsv_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	fh = &g_cdvdman_fhinfo[(uiptr)f->privdata];
	if ( cdvdman_mediactl(2) )
	{
		g_cdvdman_iocache = 0;
		cdvdman_invcaches();
	}
	if ( (fh->m_fd_flags & 8) )
	{
		rc = cdrom_stream_read(f, (char *)buf, nbytes);
	}
	else
	{
		if ( !(fh->m_fd_flags & 4) )
		{
			rc = cdrom_internal_read(f, (char *)buf, nbytes);
		}
		else
		{
			rc = cdrom_internal_cache_read(f, nbytes);
			if ( rc )
			{
				if ( rc != -EBUSY )
				{
					if ( rc != 1 )
					{
						fh->m_fd_flags &= ~4;
						rc = -EIO;
						cdvd_odcinit(fh, 0, (int)f->privdata);
					}
					else
					{
						rc = cdvdfile_cache_fill_read(f, buf, nbytes);
						VERBOSE_KPRINTF(1, "called _cdvdfile_cache_fill_read %d\n", rc);
					}
				}
			}
			else
			{
				rc = cdvdfile_cache_read(f, buf, nbytes);
				VERBOSE_KPRINTF(1, "called _cdvdfile_cache_read %d\n", rc);
			}
			if ( rc == -EBUSY || rc == -ECOMM )
			{
				cdvdman_cache_invalidate(fh, (int)f->privdata);
				KPRINTF("_cdvdfile_cache Read_err OR Drive_not_ready\n", rc);
			}
			if ( rc < 0 )
			{
				rc = cdrom_internal_read(f, (char *)buf, nbytes);
			}
		}
	}
	SetEventFlag(g_fio_fsv_evfid, 1);
	return rc;
}

static int cdrom_ioctl(iop_file_t *f, int arg, void *param)
{
#ifdef CDVD_VARIANT_OSD
	USE_IOP_MMIO_HWPORT();
#endif

	(void)f;

	switch ( arg )
	{
#ifdef CDVD_VARIANT_OSD
		case 0x6310:
			return mfc0(PRID);
		case 0x6311:
			if ( mfc0(PRID) < 35 )
			{
				return -22;
			}
			if ( *(u32 *)param )
			{
				/* 0xBF808284 */ *(u32 *)&iop_mmio_hwport->sio2.unused[0] |= 1;
			}
			else
			{
				/* 0xBF808284 */ *(u32 *)&iop_mmio_hwport->sio2.unused[0] &= ~1;
			}
			return 0;
		case 0x6312:
			return mfc0(PRID) >= 35 ? (int)(/* 0xBF808284 */ *(u32 *)&iop_mmio_hwport->sio2.unused[0] & 1) : -22;
#endif
		case 0x10000:
			g_cdvdman_spinnom = -1;
			sceCdSpinCtrlIOP((u32)param);
			return 0;
#ifdef CDVD_VARIANT_OSD
		case 0x10008:
			sceCdDecSet(*(u32 *)param, *((u32 *)param + 1), *((u32 *)param + 2));
			return 0;
		case 0x10020:
			return sceCdReadKey(*(u32 *)param, *((u32 *)param + 1), *((u32 *)param + 2), (u8 *)param + 12) ? 1 : -16;
		case 0x10030:
			return sceCdGetDiskType();
#endif
		default:
			return -EIO;
	}
}

// cppcheck-suppress constParameterCallback
static int cdrom_ioctl2(iop_file_t *f, int request, void *argp, size_t arglen, void *bufp, size_t buflen)
{
	const cdvdman_fhinfo_t *fh;
	int retval;
	u32 efbits;

	(void)argp;
	(void)arglen;
	(void)bufp;
	(void)buflen;

	WaitEventFlag(g_fio_fsv_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	fh = &g_cdvdman_fhinfo[(uiptr)f->privdata];
	retval = -EIO;
	if ( (fh->m_fd_flags & 8) )
	{
		switch ( request )
		{
			case CIOCSTREAMPAUSE:
				// The following call to sceCdStPause was inlined
				if ( sceCdStPause() )
				{
					retval = 0;
				}
				break;
			case CIOCSTREAMRESUME:
				// The following call to sceCdStResume was inlined
				if ( sceCdStResume() )
				{
					retval = 0;
				}
				break;
			case CIOCSTREAMSTAT:
				// The following call to sceCdStStat was inlined
				// Unofficial: return 0 instead of negative value
				retval = sceCdStStat();
				break;
			default:
				break;
		}
	}
	SetEventFlag(g_fio_fsv_evfid, 1);
	return retval;
}

static int
cdrom_devctl(iop_file_t *f, const char *name, int cmd, void *argp, unsigned int arglen, void *bufp, unsigned int buflen)
{
	unsigned int i;
	int retval2;
	char *sc_tmp_2;
	unsigned int sc_tmp_3;
	u32 efbits;
	int on_dual_tmp;
	int scres_unused;

	(void)f;
	(void)name;
	(void)buflen;

	retval2 = 0;
	if ( cmd != CDIOC_BREAK )
	{
		WaitEventFlag(g_fio_fsv_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	}
	switch ( cmd )
	{
		case CDIOC_READCLOCK:
			for ( i = 0; i < 3 && !retval2; i += 1 )
			{
				WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
				retval2 = sceCdReadClock((sceCdCLOCK *)bufp);
			}
			retval2 = (retval2 != 1) ? -EIO : 0;
			break;
#ifdef CDVD_VARIANT_DNAS
		case 0x431D:
			for ( i = 0; i < 3 && !retval2; i += 1 )
			{
				WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
				retval2 = sceCdReadGUID((u64 *)bufp);
			}
			retval2 = (retval2 != 1) ? -EIO : 0;
			break;
		case 0x431E:
			retval2 = (sceCdReadDiskID((unsigned int *)bufp) != 1) ? -EIO : 0;
			break;
#endif
		case CDIOC_GETDISKTYP:
			*(u32 *)bufp = sceCdGetDiskType();
			break;
		case CDIOC_GETERROR:
			*(u32 *)bufp = g_cdvdman_strmerr ? g_cdvdman_strmerr : sceCdGetError();
			break;
		case CDIOC_TRAYREQ:
			retval2 = (sceCdTrayReq(*(u32 *)argp, (u32 *)bufp) != 1) ? -EIO : 0;
			break;
		case CDIOC_STATUS:
			*(u32 *)bufp = sceCdStatus();
			break;
		case CDIOC_POWEROFF:
			for ( i = 0; i < 3 && !retval2; i += 1 )
			{
				WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
				retval2 = sceCdPowerOff((u32 *)bufp);
			}
			retval2 = (retval2 != 1) ? -EIO : 0;
			break;
		case CDIOC_MMODE:
			sceCdMmode(*(u32 *)argp);
			break;
		case CDIOC_DISKRDY:
			*(u32 *)bufp = sceCdDiskReady(*(u32 *)argp);
			break;
#ifdef CDVD_VARIANT_DNAS
		case 0x4326:
			for ( i = 0; i < 3 && !retval2; i += 1 )
			{
				WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
				retval2 = sceCdReadModelID((unsigned int *)bufp);
			}
			retval2 = (retval2 != 1) ? -EIO : 0;
			break;
#endif
		case CDIOC_STREAMINIT:
			// The following call to sceCdStInit was inlined
			retval2 = (!sceCdStInit(*(u32 *)argp, *((u32 *)argp + 1), (void *)*((u32 *)argp + 2))) ? -EIO : 0;
			break;
		case CDIOC_BREAK:
			retval2 = (!sceCdBreak()) ? -EIO : 0;
			sceCdSync(4);
			break;
#ifdef CDVD_VARIANT_XOSD
		case 0x437F:
			sc_tmp_3 = *(u32 *)argp;
			retval2 = (sc_tmp_3 - 4 >= 0x15) ? -22 : sceCdSC(0xFFFFFFDD, (int *)&sc_tmp_3);
			break;
#endif
		case CDIOC_SPINNOM:
			g_cdvdman_spinnom = SCECdSpinNom;
			break;
		case CDIOC_SPINSTM:
			g_cdvdman_spinnom = SCECdSpinStm;
			break;
		case CDIOC_TRYCNT:
			g_cdvdman_trycnt = *(u8 *)argp;
			break;
		case 0x4383:
			retval2 = (sceCdSeek(*(u32 *)argp) != 1) ? -EIO : 0;
			sceCdSync(6);
			break;
		case CDIOC_STANDBY:
			retval2 = (sceCdStandby() != 1) ? -EIO : 0;
			sceCdSync(4);
			break;
		case CDIOC_STOP:
			retval2 = (sceCdStop() != 1) ? -EIO : 0;
			sceCdSync(4);
			break;
		case CDIOC_PAUSE:
			retval2 = (sceCdPause() != 1) ? -EIO : 0;
			sceCdSync(6);
			break;
		case CDIOC_GETTOC:
			switch ( sceCdGetDiskType() )
			{
				case SCECdPSCD:
				case SCECdPSCDDA:
				case SCECdPS2CD:
				case SCECdPS2CDDA:
					retval2 = (sceCdGetToc((u8 *)bufp) != 1) ? -EIO : 0;
					break;
				default:
					break;
			}
			break;
		case CDIOC_SETTIMEOUT:
			retval2 = (sceCdSetTimeout(1, *(u32 *)argp) != 1) ? -EIO : 0;
			break;
		case CDIOC_READDVDDUALINFO:
			retval2 = (!sceCdReadDvdDualInfo(&on_dual_tmp, (unsigned int *)bufp)) ? -EIO : 0;
			break;
		case CDIOC_INIT:
			sceCdInit(*(u32 *)argp);
			retval2 = 0;
			break;
		case 0x438C:
			g_cdvdman_spinnom = SCECdSpinX1;
			break;
		case 0x438D:
			g_cdvdman_spinnom = SCECdSpinX2;
			break;
		case 0x438E:
			g_cdvdman_spinnom = SCECdSpinX4;
			break;
		case 0x438F:
			g_cdvdman_spinnom = SCECdSpinX12;
			break;
		case 0x4390:
			g_cdvdman_spinnom = SCECdSpinMx;
			break;
		case 0x4391:
			*(u32 *)bufp = sceCdSC(0xFFFFFFF5, &scres_unused);
			break;
		case 0x4392:
			retval2 = (sceCdApplySCmd(*(u8 *)argp, (char *)argp + 4, arglen - 4, bufp) != 1) ? -EIO : 0;
			break;
		case CDIOC_FSCACHEINIT:
			retval2 = -EBUSY;
			if ( g_cache_path_fd != -1 )
			{
				break;
			}
			retval2 = -EINVAL;
			sc_tmp_2 = (char *)argp;
			for ( i = 0; i < arglen && sc_tmp_2[i] && sc_tmp_2[i] != ','; i += 1 )
			{
				g_cdvdman_cache_name[i] = sc_tmp_2[i];
			}
			sc_tmp_3 = i;
			if ( i <= arglen )
			{
				i += 1;
				for ( ; (i < arglen) && sc_tmp_2[i] && sc_tmp_2[i] != ','; i += 1 )
				{
				}
			}
			if ( i <= arglen )
			{
				sc_tmp_2[i] = 0;
				g_cdvdman_cache_sector_size_count = strtol((const char *)argp + sc_tmp_3, 0, 10);
				retval2 = path_tbl_init(strtol(&sc_tmp_2[i + 1], 0, 10), g_cdvdman_cache_name, 1);
			}
			break;
		case CDIOC_FSCACHEDELETE:
			for ( i = 0; i < (sizeof(g_cdvdman_fhinfo) / sizeof(g_cdvdman_fhinfo[0])); i += 1 )
			{
				if ( (g_cdvdman_fhinfo[i].m_fd_flags & 4) )
				{
					break;
				}
			}
			retval2 = (i == (sizeof(g_cdvdman_fhinfo) / sizeof(g_cdvdman_fhinfo[0]))) ? path_tbl_init(0, 0, 0) : -EBUSY;
			break;
		default:
			retval2 = -EIO;
			break;
	}
	if ( cmd != CDIOC_BREAK )
	{
		SetEventFlag(g_fio_fsv_evfid, 1);
	}
	return retval2;
}

// cppcheck-suppress constParameterCallback
static int cdrom_lseek(iop_file_t *f, int offset, int pos)
{
	int retval;
	cdvdman_fhinfo_t *fh;
	u32 efbits;

	retval = -EPERM;
	VERBOSE_PRINTF(1, "fileIO SEEK\n");
	WaitEventFlag(g_fio_fsv_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	fh = &g_cdvdman_fhinfo[(uiptr)f->privdata];
	switch ( pos )
	{
		case 0:
			retval = offset;
			break;
		case 1:
			retval = fh->m_read_pos + offset;
			break;
		case 2:
			retval = fh->m_file_size - offset;
			break;
		default:
			break;
	}
	if ( retval > (int)(fh->m_file_size) )
	{
		retval = fh->m_file_size;
	}
	fh->m_read_pos = retval;
	if ( (fh->m_fd_flags & 8) )
	{
		// The following call to sceCdStSeekF was inlined
		if ( !sceCdStSeekF(fh->m_file_lsn + retval / 0x800) )
		{
			retval = -EIO;
		}
	}
	SetEventFlag(g_fio_fsv_evfid, 1);
	return retval;
}

static unsigned int sync_timeout_alarm_cb(void *userdata)
{
	const iop_sys_clock_t *sys_clock;

	sys_clock = (const iop_sys_clock_t *)userdata;
	KPRINTF("Cdvd Time Out %d(msec)\n", sys_clock->lo / 0x9000);
	return !sceCdBreak();
}

int sceCdSetTimeout(int param, int timeout)
{
	if ( !sceCdCheckCmd() || g_cdvdman_istruct.m_read2_flag )
	{
		return 0;
	}
	switch ( param )
	{
		case 1:
			g_cdvdman_sync_timeout = timeout;
			return 1;
		case 2:
			g_cdvdman_stream_timeout = timeout;
			return 1;
		default:
			return 0;
	}
}

int sceCdSync(int mode)
{
	iop_sys_clock_t sysclk;
	iop_event_info_t efinfo;
	u32 efbits;

	VERBOSE_KPRINTF(1, "sceCdSync: Call mode %d Com %x\n", mode, (u8)g_cdvdman_istruct.m_cdvdman_command);
	switch ( mode )
	{
		case 0:
			while ( !sceCdCheckCmd() || g_cdvdman_istruct.m_read2_flag )
			{
				WaitEventFlag(g_cdvdman_intr_evfid, 1, WEF_AND, &efbits);
			}
			break;
		case 1:
			return !!(!sceCdCheckCmd() || (g_cdvdman_istruct.m_read2_flag));
		case 3:
			sysclk.hi = 0;
			sysclk.lo = 0x9000 * g_cdvdman_sync_timeout;
			vSetAlarm(&sysclk, sync_timeout_alarm_cb, &sysclk);
			while ( !sceCdCheckCmd() || g_cdvdman_istruct.m_read2_flag )
			{
				WaitEventFlag(g_cdvdman_intr_evfid, 1, WEF_AND, &efbits);
			}
			vCancelAlarm(sync_timeout_alarm_cb, &sysclk);
			break;
		case 4:
			sysclk.hi = 0;
			sysclk.lo = 0x41EB0000;
			vSetAlarm(&sysclk, sync_timeout_alarm_cb, &sysclk);
			while ( !sceCdCheckCmd() || g_cdvdman_istruct.m_read2_flag )
			{
				WaitEventFlag(g_cdvdman_intr_evfid, 1, WEF_AND, &efbits);
			}
			vCancelAlarm(sync_timeout_alarm_cb, &sysclk);
			break;
		case 5:
			while ( !sceCdCheckCmd() )
			{
				WaitEventFlag(g_cdvdman_intr_evfid, 1, WEF_AND, &efbits);
			}
			break;
		case 6:
			sysclk.hi = 0;
			sysclk.lo = 0x9000 * g_cdvdman_sync_timeout;
			vSetAlarm(&sysclk, sync_timeout_alarm_cb, &sysclk);
			while ( !sceCdCheckCmd() || g_cdvdman_istruct.m_read2_flag )
			{
				WaitEventFlag(g_cdvdman_intr_evfid, 1, WEF_AND, &efbits);
			}
			vCancelAlarm(sync_timeout_alarm_cb, &sysclk);
			break;
		case 16:
			while ( !sceCdCheckCmd() || g_cdvdman_istruct.m_read2_flag || g_cdvdman_ee_rpc_fno
							|| g_cdvdman_istruct.m_stream_flag )
			{
				WaitEventFlag(g_cdvdman_intr_evfid, 1, WEF_AND, &efbits);
				if ( g_cdvdman_ee_rpc_fno )
				{
					DelayThread(8000);
				}
			}
			break;
		case 17:
			return !!(
				!sceCdCheckCmd() || g_cdvdman_istruct.m_read2_flag || g_cdvdman_ee_rpc_fno
				|| (g_cdvdman_istruct.m_stream_flag));
		case 32:
			WaitEventFlag(g_cdvdman_intr_evfid, 0x21, WEF_OR, &efbits);
			ReferEventFlagStatus(g_cdvdman_intr_evfid, &efinfo);
			if ( !(efinfo.currBits & 0x20) )
			{
				if ( g_cdvdman_istruct.m_last_error != SCECdErNO )
				{
					SetEventFlag(g_cdvdman_intr_evfid, 0x20);
				}
				else
				{
					WaitEventFlag(g_cdvdman_intr_evfid, 0x20, WEF_AND, &efbits);
				}
			}
			break;
		default:
			while ( !sceCdCheckCmd() || g_cdvdman_istruct.m_read2_flag )
			{
				WaitEventFlag(g_cdvdman_intr_evfid, 1, WEF_AND, &efbits);
			}
			break;
	}
	VERBOSE_KPRINTF(
		1,
		"sceCdSync: Command= %d Error= %d\n",
		(u8)g_cdvdman_istruct.m_cdvdman_command,
		(u8)g_cdvdman_istruct.m_last_error);
	return 0;
}

int sceCdSpinCtrlIOP(u32 speed)
{
	VERBOSE_KPRINTF(1, "sceCdSpinCtrlIOP speed= %d\n", speed);
	g_cdvdman_spinctl = speed;
	return 1;
}

int sceCdLayerSearchFile(sceCdlFILE *fp, const char *path, int layer)
{
	unsigned int i;
	int search_res;
	u32 efbits;

	if ( PollEventFlag(g_sfile_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND )
	{
		return 0;
	}
	for ( i = 0; i < ((sizeof(g_cdvdman_sfname) / sizeof(g_cdvdman_sfname[0])) - 1) && path[i]; i += 1 )
	{
		g_cdvdman_sfname[i] = path[i];
	}
	g_cdvdman_sfname[i] = 0;
	g_cdvdman_srchspd = 1;
	search_res = CdSearchFileInner((cdvdman_filetbl_entry_t *)fp, g_cdvdman_sfname, layer);
	vSetEventFlag(g_sfile_evfid, 1);
	return search_res;
}

int sceCdSearchFile(sceCdlFILE *file, const char *name)
{
	return sceCdLayerSearchFile(file, name, 0);
}

int sceCdGetToc(u8 *toc)
{
	return cdvdman_gettoc(toc);
}

int sceCdDiskReady(int mode)
{
	u32 efbits;
	USE_DEV5_MMIO_HWPORT();

	efbits = 0;
	// The following Kprintf was modified for ioprp300x
	VERBOSE_KPRINTF(1, "DISK READY call from iop %d\n", mode);
#ifdef CDVD_VARIANT_XOSD
	if ( g_cdvdman_istruct.m_cd_mode_ps2_atapi == 1 )
	{
		return mode == 8 ? -1 : 6;
	}
	switch ( get_disk_type_ex() )
	{
		case SCECdNODISC:
		case 257:
			return 2;
		default:
			break;
	}
#endif
	switch ( mode )
	{
		case 0:
			VERBOSE_KPRINTF(1, "Wait Drive Ready %x\n", dev5_mmio_hwport->m_dev5_reg_005);
			while ( g_cdvdman_istruct.m_read2_flag )
			{
				// The following call to sceCdGetDiskType was inlined
#ifdef CDVD_VARIANT_XOSD
				switch ( get_disk_type_ex() )
#else
				switch ( sceCdGetDiskType() )
#endif
				{
					case SCECdDETCT:
					case SCECdDETCTCD:
					case SCECdDETCTDVDS:
					case SCECdDETCTDVDD:
						while ( (dev5_mmio_hwport->m_dev5_reg_005 & 0xC0) != 0x40 )
						{
							vDelayThread(2000);
							WaitEventFlag(g_cdvdman_intr_evfid, 1, WEF_AND, &efbits);
#ifdef CDVD_VARIANT_XOSD
							if ( !get_disk_type_ex() )
							{
								return 2;
							}
#endif
						}
						continue;
					default:
						break;
				}
				break;
			}
			// The following Kprintf was added for ioprp300x
			VERBOSE_KPRINTF(1, "Wait Exit\n");
			return 2;
		case 8:
			return (u8)dev5_mmio_hwport->m_dev5_reg_005;
		case 1:
		default:
			if ( (dev5_mmio_hwport->m_dev5_reg_005 & 0xC0) == 0x40 && !g_cdvdman_istruct.m_read2_flag )
			{
				return 2;
			}
			VERBOSE_KPRINTF(1, "Drive Not Ready\n");
			return 6;
	}
}

int sceCdGetDiskType(void)
{
#ifdef CDVD_VARIANT_XOSD
	int cdvd_register;

	VERBOSE_KPRINTF(1, "sceCdGetDiskType Call\n");
	cdvd_register = get_cdvd_register(0x0F);
	VERBOSE_KPRINTF(1, "sceCdGetDiskType 0x%02x\n", cdvd_register);
	return cdvd_register;
#else
	USE_DEV5_MMIO_HWPORT();

	return (u8)dev5_mmio_hwport->m_dev5_reg_00F;
#endif
}

#ifdef CDVD_VARIANT_XOSD
static int get_disk_type_ex(void)
{
	int IntrContext;
	int retval;
	int state;
	USE_DEV5_MMIO_HWPORT();

	IntrContext = QueryIntrContext();
	if ( IntrContext )
	{
		CpuSuspendIntr(&state);
	}
	retval = (g_cdvdman_istruct.m_cd_mode_ps2_atapi == 1) ? 257 : dev5_mmio_hwport->m_dev5_reg_00F;
	if ( IntrContext )
	{
		CpuResumeIntr(state);
	}
	return retval;
}
#endif

#ifdef CDVD_VARIANT_XOSD
int sceCdGetWakeUpReason(void)
{
	int wakeup_reason_reg;

	VERBOSE_KPRINTF(1, "sceCdGetWakeUpReason Call\n");
	wakeup_reason_reg = get_cdvd_register(0x15);
	if ( wakeup_reason_reg != -1 )
	{
		wakeup_reason_reg &= 0xF;
	}
	VERBOSE_KPRINTF(1, "sceCdGetWakeUpReason 0x%02x\n", wakeup_reason_reg);
	return wakeup_reason_reg;
}
#endif

int sceCdStatus(void)
{
	int reg_00A_tmp;
#ifndef CDVD_VARIANT_XOSD
	u32 status_tmp;
#endif
#ifndef CDVD_VARIANT_XOSD
	USE_DEV5_MMIO_HWPORT();
#endif

#ifdef CDVD_VARIANT_XOSD
	reg_00A_tmp = get_cdvd_register(0x0A);
#else
	reg_00A_tmp = dev5_mmio_hwport->m_dev5_reg_00A;
#endif
#ifndef CDVD_VARIANT_XOSD
	// The following call to sceCdGetDiskType was inlined
	if ( sceCdGetDiskType() == SCECdNODISC )
	{
		u8 rdata_tmp;

		if ( !g_cdvdman_istruct.m_tray_is_open && cdvdman_scmd_sender_03_30(&rdata_tmp, &status_tmp) == 1 && !status_tmp )
		{
			reg_00A_tmp &= ~SCECdStatShellOpen;
			if ( (rdata_tmp & 8) )
			{
				reg_00A_tmp |= SCECdStatShellOpen;
			}
		}
		if ( (reg_00A_tmp & 0x1E) )
		{
			reg_00A_tmp = SCECdStatStop;
		}
	}
#endif
	if ( g_cdvdman_istruct.m_use_toc )
	{
		reg_00A_tmp &= ~SCECdStatShellOpen;
	}
	if ( g_cdvdman_istruct.m_power_flag )
	{
		return -1;
	}
	return reg_00A_tmp;
}

#ifdef CDVD_VARIANT_OSD
int sceCdStatus2(void)
{
	USE_DEV5_MMIO_HWPORT();

	return !g_cdvdman_istruct.m_power_flag ? (u8)dev5_mmio_hwport->m_dev5_reg_00A : -1;
}
#endif

sceCdlLOCCD *sceCdIntToPos(u32 i, sceCdlLOCCD *p)
{
	p->sector = 16 * ((i + 150) % 75 / 10) + (i + 150) % 75 % 10;
	p->second = 16 * ((i + 150) / 75 % 60 / 10) + (i + 150) / 75 % 60 % 10;
	p->minute = 16 * ((i + 150) / 75 / 60 / 10) + (i + 150) / 75 / 60 % 10;
	return p;
}

// cppcheck-suppress constParameterPointer
u32 sceCdPosToInt(sceCdlLOCCD *p)
{
	return 75 * (60 * (10 * (p->minute >> 4) + (p->minute & 0xF)) + 10 * (p->second >> 4) + (p->second & 0xF))
			 + 10 * (p->sector >> 4) + (p->sector & 0xF) - 150;
}

#ifdef CDVD_VARIANT_DNAS
static int read_id_from_rom(int mode, int *buf)
{
	int chksumint;
	unsigned int i;
	unsigned int j;
	int idinfo[0x20];
	int chksumstk;

	chksumint = 0;
	chksumstk = 0;
	for ( i = 0; i < (sizeof(idinfo) / sizeof(idinfo[0])); i += 1 )
	{
		for ( j = 0; j < sizeof(chksumstk); j += 1 )
		{
			((char *)&idinfo)[(i * 4) + j] = ((char *)0xBFBF0000)[(i * 4) + j];
			((char *)&chksumstk)[j] = ((char *)0xBFBF0000)[(i * 4) + j];
		}
		chksumint += chksumstk;
	}
	for ( ; i < 0x4000; i += 1 )
	{
		for ( j = 0; j < sizeof(chksumstk); j += 1 )
		{
			((char *)&chksumstk)[j] = ((char *)0xBFBF0000)[(i * 4) + j];
		}
		chksumint += chksumstk;
	}
	if ( chksumint )
	{
		KPRINTF("# checksum error %d\n", chksumint);
		return 0;
	}
	if ( mode )
	{
		*buf = idinfo[11];
	}
	else
	{
		*buf = idinfo[2];
		buf[1] = idinfo[3];
	}
	return 1;
}

static int query_boot_mode_6_nonzero(void)
{
	int *BootMode;

	BootMode = QueryBootMode(6);
	return !(!BootMode || (*(u16 *)BootMode & 0xFFFC) != 0x60);
}

static int query_boot_mode_6_zero(void)
{
	return !QueryBootMode(6);
}

static int cdvdman_readID(int mode, int *buf)
{
	u8 id_val[8];
	iop_sys_clock_t sysclk;
	u32 id_result;

	id_result = -1;
	if ( query_boot_mode_6_nonzero() )
	{
		if ( read_id_from_rom(mode, buf) && mode == 1 )
		{
			if ( *buf == -1 )
			{
				*buf = 0x1A0002;
			}
			return 1;
		}
		return 0;
	}
	else
	{
		if ( query_boot_mode_6_zero() )
		{
			if ( !sceCdRI(id_val, &id_result) || id_result )
			{
				return 0;
			}
		}
		else
		{
			if ( !g_readid_systemtime.lo && !g_readid_systemtime.hi )
			{
				GetSystemTime(&sysclk);
				g_readid_systemtime = sysclk;
			}
			*(iop_sys_clock_t *)id_val = g_readid_systemtime;
		}
		if ( mode )
		{
			*buf = *(u32 *)id_val >> 8;
		}
		else
		{
			*buf = id_val[0] | 0x8004600;
			buf[1] = *(u32 *)&id_val[4];
		}
		return 1;
	}
}

int sceCdReadGUID(u64 *guid)
{
	return cdvdman_readID(0, (int *)guid);
}

int sceCdReadModelID(unsigned int *id)
{
	return cdvdman_readID(1, (int *)id);
}
#endif

int sceCdStInit(u32 bufmax, u32 bankmax, void *buffer)
{
	cdrom_stm_devctl_t devctl_req;
	int buf;

	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(1, "sceCdStInit call IOP\n");
	memset(&devctl_req, 0, sizeof(devctl_req));
	devctl_req.m_cmdid = 5;
	devctl_req.m_posszarg1 = bufmax;
	devctl_req.m_posszarg2 = bankmax;
	devctl_req.m_buffer = buffer;
	return (devctl("cdrom_stm0:", 0x4393, &devctl_req, sizeof(devctl_req), &buf, 4) >= 0) ? buf : 0;
}

// cppcheck-suppress constParameterPointer
int sceCdStStart(u32 lbn, sceCdRMode *mode)
{
	cdrom_stm_devctl_t devctl_req;
	int buf;

	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(1, "sceCdStStart call IOP\n");
	memset(&devctl_req, 0, sizeof(devctl_req));
	devctl_req.m_rmode.datapattern = mode->datapattern;
	devctl_req.m_rmode.spindlctrl = mode->spindlctrl;
	devctl_req.m_cmdid = 1;
	devctl_req.m_posszarg1 = lbn;
	devctl_req.m_rmode.trycount = mode->trycount;
	return (devctl("cdrom_stm0:", 0x4393, &devctl_req, sizeof(devctl_req), &buf, 4) >= 0) ? buf : 0;
}

int sceCdStSeekF(unsigned int lsn)
{
	cdrom_stm_devctl_t devctl_req;
	int buf;

	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(1, "sceCdStSeekF call IOP\n");
	memset(&devctl_req, 0, sizeof(devctl_req));
	devctl_req.m_cmdid = 9;
	devctl_req.m_posszarg1 = lsn;
	return (devctl("cdrom_stm0:", 0x4393, &devctl_req, sizeof(devctl_req), &buf, 4) >= 0) ? buf : 0;
}

int sceCdStSeek(u32 lbn)
{
	cdrom_stm_devctl_t devctl_req;
	int buf;

	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(1, "sceCdStSeek call IOP\n");
	memset(&devctl_req, 0, sizeof(devctl_req));
	devctl_req.m_posszarg1 = lbn;
	devctl_req.m_cmdid = 4;
	return (devctl("cdrom_stm0:", 0x4393, &devctl_req, sizeof(devctl_req), &buf, 4) >= 0) ? buf : 0;
}

int sceCdStStop(void)
{
	cdrom_stm_devctl_t devctl_req;
	int buf;

	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(1, "sceCdStStop call IOP\n");
	memset(&devctl_req, 0, sizeof(devctl_req));
	devctl_req.m_cmdid = 3;
	return (devctl("cdrom_stm0:", 0x4393, &devctl_req, sizeof(devctl_req), &buf, 4) >= 0) ? buf : 0;
}

int sceCdStRead(u32 sectors, u32 *buffer, u32 mode, u32 *error)
{
	cdrom_stm_devctl_t devctl_req;
	int buf;

	(void)mode;

	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(1, "sceCdStRead call IOP\n");
	memset(&devctl_req, 0, sizeof(devctl_req));
	devctl_req.m_cmdid = 1;
	devctl_req.m_posszarg2 = sectors;
	devctl_req.m_buffer = buffer;
	if ( devctl("cdrom_stm0:", 0x4394, &devctl_req, sizeof(devctl_req), &buf, 4) < 0 )
	{
		buf = 0;
	}
	*error = devctl_req.m_error;
	return buf;
}

int sceCdStPause(void)
{
	cdrom_stm_devctl_t devctl_req;
	int buf;

	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(1, "sceCdStPause call IOP\n");
	memset(&devctl_req, 0, sizeof(devctl_req));
	devctl_req.m_cmdid = 7;
	return (devctl("cdrom_stm0:", 0x4393, &devctl_req, sizeof(devctl_req), &buf, 4) >= 0) ? buf : 0;
}

int sceCdStResume(void)
{
	cdrom_stm_devctl_t devctl_req;
	int buf;

	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(1, "sceCdStResume call IOP\n");
	memset(&devctl_req, 0, sizeof(devctl_req));
	devctl_req.m_cmdid = 8;
	return (devctl("cdrom_stm0:", 0x4393, &devctl_req, sizeof(devctl_req), &buf, 4) >= 0) ? buf : 0;
}

int sceCdStStat(void)
{
	cdrom_stm_devctl_t devctl_req;
	int buf;

	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(1, "sceCdStStat call IOP\n");
	memset(&devctl_req, 0, sizeof(devctl_req));
	devctl_req.m_cmdid = 6;
	return (devctl("cdrom_stm0:", 0x4393, &devctl_req, sizeof(devctl_req), &buf, 4) >= 0) ? buf : 0;
}

static int CdSearchFileInner(cdvdman_filetbl_entry_t *fp, const char *name, int layer)
{
	int parent_level;
	int i;
	unsigned int j;
	char name_buf[32];

	VERBOSE_PRINTF(1, "CdSearchFile: start name= %s layer= %d\n", name, layer);
	if ( g_cdvdman_fs_layer != layer )
	{
		g_cdvdman_fs_cache = 0;
	}
	if ( !cdvdman_mediactl(0) && g_cdvdman_fs_cache )
	{
		VERBOSE_KPRINTF(1, "CdSearchFile: cache dir data used\n");
	}
	else
	{
		VERBOSE_PRINTF(1, "CdSearchFile Topen= %s\n", name);
		if ( !CD_newmedia(layer) )
		{
			g_cdvdman_fs_cache = 0;
			return 0;
		}
		g_cdvdman_fs_cache = 1;
	}
	if ( *name != '\\' )
	{
		return 0;
	}
	name_buf[0] = 0;
	parent_level = 1;
	j = 0;
	for ( i = 0; i < 8 && name[i]; i += 1 )
	{
		for ( j = 0; name[i + j] != '\\'; j += 1 )
		{
			name_buf[j] = name[i + j];
		}
		name_buf[j] = 0;
		parent_level = cdvdman_finddir(parent_level, name_buf);
		if ( parent_level == -1 )
		{
			name_buf[0] = 0;
			break;
		}
	}
	if ( i >= 8 )
	{
		VERBOSE_PRINTF(1, "%s: path level (%d) error\n", name, i);
		return 0;
	}
	if ( !name_buf[0] )
	{
		VERBOSE_PRINTF(1, "%s: dir was not found\n", name);
		return 0;
	}
	name_buf[j] = 0;
	if ( !CD_cachefile(parent_level, layer) )
	{
		VERBOSE_PRINTF(1, "CdSearchFile: disc error\n");
		return 0;
	}
	VERBOSE_PRINTF(2, "CdSearchFile: searching %s...\n", name_buf);
	for ( j = 0;
				j < (sizeof(g_cdvdman_filetbl) / sizeof(g_cdvdman_filetbl[0])) && g_cdvdman_filetbl[j].m_file_struct.name[0];
				j += 1 )
	{
		VERBOSE_PRINTF(1, "%d %s %s\n", (int)j, g_cdvdman_filetbl[j].m_file_struct.name, name_buf);
		if ( cdvdman_cmpname(g_cdvdman_filetbl[j].m_file_struct.name, name_buf) )
		{
			VERBOSE_PRINTF(2, "%s:\t found\n", name_buf);
			// The following memcpy was inlined
			memcpy(fp, &g_cdvdman_filetbl[j], sizeof(cdvdman_filetbl_entry_t));
			fp->m_file_struct.lsn += layer ? g_cdvdman_fs_base2 : 0;
			return 1;
		}
	}
	VERBOSE_PRINTF(1, "%s: not found\n", name_buf);
	return 0;
}

static int sceCdSearchDir(char *dirname, int layer)
{
	sceCdlFILE fp;

	VERBOSE_PRINTF(1, "_sceCdSearchDir: dir name %s layer %d\n", dirname, layer);
	return sceCdLayerSearchFile(&fp, dirname, layer) ? g_cdvdman_fs_cdsec : SCECdErREADCFR;
}

static int sceCdReadDir(sceCdlFILE *fp, int dsec, int index, int layer)
{
	VERBOSE_PRINTF(1, "_sceCdReadDir: current= %d dsec= %d layer= %d\n", g_cdvdman_fs_cdsec, dsec, layer);
	if ( g_cdvdman_fs_cdsec != dsec || g_cdvdman_fs_layer != layer )
	{
		if ( g_cdvdman_fs_layer != layer )
		{
			if ( !CD_newmedia(layer) )
			{
				return -ENOENT;
			}
			g_cdvdman_fs_cache = 1;
		}
		if ( !CD_cachefile(dsec, layer) )
		{
			return -ENOENT;
		}
	}
	if ( g_cdvdman_filetbl[index].m_file_struct.name[0] )
	{
		VERBOSE_PRINTF(1, "%s:\t found dir_point %d\n", g_cdvdman_filetbl[index].m_file_struct.name, index);
		// The following memcpy was inlined
		memcpy(fp, &g_cdvdman_filetbl[index], sizeof(cdvdman_filetbl_entry_t));
		return 1;
	}
	return 0;
}

static int cdvdman_cmpname(const char *p, const char *q)
{
	return !strncmp(p, q, 12);
}

static int CD_newmedia(int arg)
{
	unsigned int DiskType;
	unsigned int i;
	iso9660_path_t *path_cur;
	int state;
	int ptsector;

	ptsector = 0;
#ifdef CDVD_VARIANT_XOSD
	DiskType = get_disk_type_ex();
#else
	DiskType = sceCdGetDiskType();
#endif
	switch ( DiskType )
	{
		case SCECdPSCD:
		case SCECdPSCDDA:
		case SCECdPS2CD:
		case SCECdPS2CDDA:
		case SCECdPS2DVD:
		case SCECdDVDVR:
		case SCECdDVDV:
		case SCECdIllegalMedia:
			break;
		default:
			VERBOSE_PRINTF(1, "CD_newmedia: Illegal disc media type =%d\n", (int)DiskType);
			return 0;
	}
	g_cdvdman_fs_base2 = 0;
	if ( DiskType == SCECdPS2DVD )
	{
		if ( !DvdDual_infochk() )
		{
			VERBOSE_PRINTF(1, "CD_newmedia: Get DvdDual_infochk fail\n");
			return 0;
		}
		g_cdvdman_fs_base2 = arg ? g_cdvdman_istruct.m_layer_1_lsn : 0;
	}
	if ( disc_read(1, g_cdvdman_fs_base2 + 0x10, g_cdvdman_fs_rbuf, arg) != 1 )
	{
		VERBOSE_PRINTF(1, "CD_newmedia: Read error in disc_read(PVD)\n");
		return 0;
	}
	CpuSuspendIntr(&state);
	for ( i = 0; i < g_cdvdman_pathtblsize; i += 1 )
	{
		g_cdvdman_pathtbl[i].m_cache_hit_count = 0;
		g_cdvdman_pathtbl[i].m_layer = 0;
		g_cdvdman_pathtbl[i].m_nsec = 0;
		g_cdvdman_pathtbl[i].m_lsn = 0;
		g_cdvdman_pathtbl[i].m_cache_path_sz = 0;
	}
	g_cache_count = 0;
	g_cache_table = 0;
	g_cache_path_size = 0;
	CpuResumeIntr(state);
	if ( strncmp((char *)((iso9660_desc_t *)g_cdvdman_fs_rbuf)->m_id, "CD001", 5) )
	{
		VERBOSE_PRINTF(1, "CD_newmedia: Disc format error in cd_read(PVD)\n");
		return 0;
	}
	switch ( DiskType )
	{
		case SCECdPSCD:
		case SCECdPSCDDA:
		case SCECdPS2CD:
		case SCECdPS2CDDA:
			VERBOSE_PRINTF(1, "CD_newmedia: CD Read mode\n");
			ptsector = *(u32 *)(((iso9660_desc_t *)g_cdvdman_fs_rbuf)->m_type_l_path_table);
			break;
		case SCECdPS2DVD:
		case SCECdDVDVR:
		case SCECdDVDV:
			VERBOSE_PRINTF(1, "CD_newmedia: DVD Read mode\n");
			ptsector = 257;
			break;
	}
	if ( disc_read(1, g_cdvdman_fs_base2 + ptsector, g_cdvdman_fs_rbuf, arg) != 1 )
	{
		VERBOSE_PRINTF(1, "CD_newmedia: Read error (PT:%08x)\n", ptsector);
		return 0;
	}
	VERBOSE_PRINTF(2, "CD_newmedia: sarching dir..\n");
	for ( i = 0, path_cur = (iso9660_path_t *)g_cdvdman_fs_rbuf;
				i < (sizeof(g_cdvdman_dirtbl) / sizeof(g_cdvdman_dirtbl[0]))
				&& path_cur < (iso9660_path_t *)&g_cdvdman_fs_rbuf[sizeof(g_cdvdman_fs_rbuf)] && path_cur->m_name_len[0];
				i += 1,
				path_cur = (iso9660_path_t *)(((char *)path_cur) + path_cur->m_name_len[0] + (path_cur->m_name_len[0] & 1)
																			+ sizeof(iso9660_path_t)) )
	{
		memcpy(&g_cdvdman_dirtbl[i].m_extent, path_cur->m_extent, sizeof(path_cur->m_extent));
		g_cdvdman_dirtbl[i].m_number = i;
		memcpy(&g_cdvdman_dirtbl[i].m_parent, path_cur->m_parent, sizeof(path_cur->m_parent));
		memcpy(g_cdvdman_dirtbl[i].m_name, path_cur->m_name, path_cur->m_name_len[0]);
		g_cdvdman_dirtbl[i].m_name[path_cur->m_name_len[0]] = 0;
		VERBOSE_PRINTF(
			2,
			"\t%08x,%04x,%04x,%s\n",
			g_cdvdman_dirtbl[i].m_extent,
			g_cdvdman_dirtbl[i].m_number,
			g_cdvdman_dirtbl[i].m_parent,
			g_cdvdman_dirtbl[i].m_name);
	}
	if ( i < (sizeof(g_cdvdman_dirtbl) / sizeof(g_cdvdman_dirtbl[0])) )
	{
		g_cdvdman_dirtbl[i].m_parent = 0;
	}
	g_cdvdman_fs_cdsec = 0;
	g_cdvdman_fs_layer = arg;
	VERBOSE_PRINTF(2, "CD_newmedia: %d dir entries found\n", (int)i);
	return 1;
}

static int cdvdman_finddir(int target_parent, const char *target_name)
{
	unsigned int i;

	for ( i = 0; i < (sizeof(g_cdvdman_dirtbl) / sizeof(g_cdvdman_dirtbl[0])) && g_cdvdman_dirtbl[i].m_parent; i += 1 )
	{
		if ( g_cdvdman_dirtbl[i].m_parent == target_parent && !strcmp(target_name, g_cdvdman_dirtbl[i].m_name) )
		{
			return i + 1;
		}
	}
	return -1;
}

static int CD_cachefile(int dsec, int layer)
{
	iso9660_dirent_t *dirent_cur;
	unsigned int i;

	if ( dsec == g_cdvdman_fs_cdsec )
	{
		return 1;
	}
	if (
		disc_read(1, g_cdvdman_dirtbl[dsec - 1].m_extent + (layer ? g_cdvdman_fs_base2 : 0), g_cdvdman_fs_rbuf, layer)
		!= 1 )
	{
		VERBOSE_PRINTF(1, "CD_cachefile: dir not found\n");
		g_cdvdman_fs_cdsec = 0;
		return 0;
	}
	VERBOSE_PRINTF(2, "CD_cachefile: searching...\n");
	for ( i = 0, dirent_cur = (iso9660_dirent_t *)g_cdvdman_fs_rbuf;
				i < (sizeof(g_cdvdman_filetbl) / sizeof(g_cdvdman_filetbl[0]))
				&& dirent_cur < (iso9660_dirent_t *)&g_cdvdman_fs_rbuf[sizeof(g_cdvdman_fs_rbuf)];
				i += 1, dirent_cur = (iso9660_dirent_t *)((char *)dirent_cur + dirent_cur->m_length[0]) )
	{
		int file_year;

		if ( !dirent_cur->m_length[0] )
		{
			break;
		}
		memcpy(
			&g_cdvdman_filetbl[i].m_file_struct.lsn, dirent_cur->m_extent, sizeof(g_cdvdman_filetbl[i].m_file_struct.lsn));
		memcpy(
			&g_cdvdman_filetbl[i].m_file_struct.size, dirent_cur->m_size, sizeof(g_cdvdman_filetbl[i].m_file_struct.size));
		file_year = dirent_cur->m_date[0] + 1900;
		g_cdvdman_filetbl[i].m_file_struct.date[7] = file_year >> 8;
		g_cdvdman_filetbl[i].m_file_struct.date[6] = file_year;
		g_cdvdman_filetbl[i].m_file_struct.date[5] = dirent_cur->m_date[1];
		g_cdvdman_filetbl[i].m_file_struct.date[4] = dirent_cur->m_date[2];
		g_cdvdman_filetbl[i].m_file_struct.date[3] = dirent_cur->m_date[3];
		g_cdvdman_filetbl[i].m_file_struct.date[2] = dirent_cur->m_date[4];
		g_cdvdman_filetbl[i].m_file_struct.date[1] = dirent_cur->m_date[5];
		g_cdvdman_filetbl[i].m_flags = dirent_cur->m_flags[0];
		switch ( i )
		{
			case 0:
				strcpy(g_cdvdman_filetbl[i].m_file_struct.name, ".");
				break;
			case 1:
				strcpy(g_cdvdman_filetbl[i].m_file_struct.name, "..");
				break;
			default:
				memcpy(g_cdvdman_filetbl[i].m_file_struct.name, dirent_cur->m_name, dirent_cur->m_name_len[0]);
				g_cdvdman_filetbl[i].m_file_struct.name[dirent_cur->m_name_len[0]] = 0;
				break;
		}
		VERBOSE_PRINTF(
			2,
			"\t lsn %d size %d name:%d:%s %d/%d/%d %d:%d:%d\n",
			(int)(g_cdvdman_filetbl[i].m_file_struct.lsn),
			(int)(g_cdvdman_filetbl[i].m_file_struct.size),
			dirent_cur->m_name_len[0],
			g_cdvdman_filetbl[i].m_file_struct.name,
			file_year,
			g_cdvdman_filetbl[i].m_file_struct.date[5],
			g_cdvdman_filetbl[i].m_file_struct.date[4],
			g_cdvdman_filetbl[i].m_file_struct.date[3],
			g_cdvdman_filetbl[i].m_file_struct.date[2],
			g_cdvdman_filetbl[i].m_file_struct.date[1]);
	}
	g_cdvdman_fs_cdsec = dsec;
	if ( i < (sizeof(g_cdvdman_filetbl) / sizeof(g_cdvdman_filetbl[0])) )
	{
		g_cdvdman_filetbl[i].m_file_struct.name[0] = 0;
	}
	VERBOSE_PRINTF(2, "CD_cachefile: %d files found\n", (int)i);
	return 1;
}

static int disc_read(int size, int loc, void *buffer, int layer)
{
	int f;
	int i;
	sceCdRMode rmode;
	int has_success;

	has_success = 1;
	f = 0;
	rmode.trycount = 16;
	VERBOSE_PRINTF(1, "cd_read:lsn= %d size= %d layer= %d\n", loc, size, layer);
	if ( cdvdman_l0check(layer) )
	{
		g_cdvdman_srchspd = 0;
	}
	switch ( g_cdvdman_srchspd )
	{
		case SCECdSpinX1:
		case SCECdSpinX2:
		case SCECdSpinX4:
			rmode.spindlctrl = g_cdvdman_srchspd;
			break;
		case SCECdSpinStm:
		case SCECdSpinNom:
			rmode.spindlctrl = !!g_cdvdman_srchspd;
			break;
		default:
			rmode.spindlctrl = SCECdSpinNom;
			break;
	}
	rmode.datapattern = SCECdSecS2048;
	if ( !g_cdvdman_pathtblflag )
	{
		has_success = 1;
	}
	if ( !has_success )
	{
		int pathcachecnt;

		pathcachecnt = (g_cache_count < g_cdvdman_pathtblsize) ? g_cache_count : g_cdvdman_pathtblsize;
		for ( i = 0; i < pathcachecnt; i += 1 )
		{
			VERBOSE_KPRINTF(
				1,
				"Path table Cache Search lsn:%d:%d nsec:%d:%d layer%d:%d\n",
				g_cdvdman_pathtbl[i].m_lsn,
				loc,
				g_cdvdman_pathtbl[i].m_nsec,
				size,
				g_cdvdman_pathtbl[i].m_layer,
				layer);
			if (
				g_cdvdman_pathtbl[i].m_lsn == loc && g_cdvdman_pathtbl[i].m_nsec == (unsigned int)size
				&& g_cdvdman_pathtbl[i].m_layer == layer )
			{
				break;
			}
		}
		if ( i != pathcachecnt )
		{
			VERBOSE_KPRINTF(1, "Path table Cache ON:%d\n", g_cdvdman_pathtbl[i].m_cache_path_sz);
			if ( lseek(g_cache_path_fd, g_cdvdman_pathtbl[i].m_cache_path_sz, 0) >= 0 )
			{
				read(g_cache_path_fd, buffer, size << 11);
				f = 1;
				g_cdvdman_pathtbl[i].m_cache_hit_count += 1;
			}
			has_success = 1;
		}
		if ( !has_success )
		{
			if ( !sceCdRE(loc, size, buffer, &rmode) )
			{
				return 0;
			}
			sceCdSync(3);
		}
	}
	if ( !has_success && sceCdGetError() != SCECdErNO )
	{
		int cache_path_sz;

		if ( g_cache_count >= g_cdvdman_pathtblsize )
		{
			int cachetblo1;
			unsigned int cacheblo2;

			g_cache_table += 1;
			if ( g_cache_table >= g_cdvdman_pathtblsize )
			{
				g_cache_table = 0;
			}
			cachetblo1 = g_cache_table;
			cacheblo2 = cachetblo1;
			for ( i = 0; (unsigned int)i < g_cache_count; i += 1 )
			{
				if ( cacheblo2 >= g_cdvdman_pathtblsize )
				{
					cacheblo2 = 0;
				}
				if (
					g_cdvdman_pathtbl[cacheblo2].m_nsec >= (unsigned int)size
					&& g_cdvdman_pathtbl[cacheblo2].m_cache_hit_count
							 < (unsigned int)g_cdvdman_pathtbl[cachetblo1].m_cache_hit_count )
				{
					cachetblo1 = cacheblo2;
				}
				cacheblo2 += 1;
			}
			cache_path_sz = g_cdvdman_pathtbl[cachetblo1].m_cache_path_sz;
			g_cache_table = cachetblo1;
		}
		else
		{
			cache_path_sz = g_cache_path_size;
			g_cache_table = g_cache_count;
			g_cache_count += 1;
		}
		if ( lseek(g_cache_path_fd, cache_path_sz, 0) >= 0 )
		{
			int ptbl_wcache_write_res;

			ptbl_wcache_write_res = write(g_cache_path_fd, buffer, size << 11);
			if ( ptbl_wcache_write_res == size << 11 )
			{
				f = 1;
				g_cdvdman_pathtbl[g_cache_table].m_cache_path_sz = cache_path_sz;
				g_cdvdman_pathtbl[g_cache_table].m_lsn = loc;
				g_cdvdman_pathtbl[g_cache_table].m_nsec = size;
				g_cdvdman_pathtbl[g_cache_table].m_layer = layer;
				g_cdvdman_pathtbl[g_cache_table].m_cache_hit_count = 0;
				g_cache_path_size += (g_cache_count < g_cdvdman_pathtblsize) ? ptbl_wcache_write_res : 0;
			}
			else
			{
				VERBOSE_KPRINTF(1, "Ptbl_WCache:write %d", ptbl_wcache_write_res);
				g_cdvdman_pathtbl[g_cache_table].m_cache_hit_count = 0;
				g_cdvdman_pathtbl[g_cache_table].m_layer = 0;
				g_cdvdman_pathtbl[g_cache_table].m_nsec = 0;
				g_cdvdman_pathtbl[g_cache_table].m_lsn = 0;
			}
		}
		has_success = 1;
	}
	if ( has_success )
	{
		if ( f )
		{
			return size;
		}
		if ( !sceCdRE(loc, size, buffer, &rmode) )
		{
			return 0;
		}
		sceCdSync(3);
		if ( sceCdGetError() != SCECdErNO )
		{
			return size;
		}
	}
	VERBOSE_KPRINTF(1, "cd_read: error code %x\n", sceCdGetError());
	return 0;
}

static int path_tbl_init(u32 blocks, char *fname, int action)
{
	int num;
	int v;
	char cachedir[512];
	int state;
	u32 blocksbs;

	num = 0;
	v = 0;
	if ( action )
	{
		CpuSuspendIntr(&state);
		g_cdvdman_pathtbl = (cdvdman_pathtbl_t *)AllocSysMemory(ALLOC_LAST, sizeof(cdvdman_pathtbl_t) * blocks, 0);
		if ( !g_cdvdman_pathtbl )
		{
			CpuResumeIntr(state);
			g_cdvdman_pathtblflag = 0;
			return -ENOMEM;
		}
		CpuResumeIntr(state);
		sprintf(cachedir, "%sCache_Path", fname);
		v = open(cachedir, O_TRUNC | O_CREAT | O_RDWR, 0x1ff /* 0o777 */);
		if ( v >= 0 )
		{
			u32 i;

			g_cache_path_fd = v;
			if ( !strncmp(cachedir, "pfs", 3) )
			{
				blocksbs = blocks << 11;
				ioctl2(g_cache_path_fd, PIOCALLOC, &blocksbs, sizeof(blocksbs), 0, 0);
			}
			for ( i = 0; i < blocks; i += 1 )
			{
				v = write(g_cache_path_fd, g_cdvdman_fs_rbuf, sizeof(g_cdvdman_fs_rbuf));
				if ( v < 0 )
				{
					break;
				}
			}
			if ( v >= 0 )
			{
				CpuSuspendIntr(&state);
				g_cdvdman_pathtblsize = blocks;
				for ( i = 0; i < blocks; i += 1 )
				{
					g_cdvdman_pathtbl[i].m_cache_hit_count = 0;
					g_cdvdman_pathtbl[i].m_layer = 0;
					g_cdvdman_pathtbl[i].m_nsec = 0;
					g_cdvdman_pathtbl[i].m_lsn = 0;
					g_cdvdman_pathtbl[i].m_cache_path_sz = 0;
				}
				g_cache_path_size = 0;
				g_cache_count = 0;
				g_cache_table = 0;
				g_cdvdman_pathtblflag = 1;
				CpuResumeIntr(state);
				return 0;
			}
		}
	}
	if ( g_cache_path_fd != -1 )
	{
		num = close(g_cache_path_fd);
		if ( num >= 0 )
		{
			if ( !strncmp(cachedir, "pfs", 3) )
			{
				num = remove(cachedir);
			}
			else if ( !strncmp(cachedir, "host", 4) )
			{
				num = 0;
				remove(cachedir);
			}
		}
	}
	CpuSuspendIntr(&state);
	g_cache_path_fd = -1;
	g_cache_count = 0;
	g_cache_table = 0;
	g_cache_path_size = 0;
	g_cdvdman_pathtblflag = 0;
	g_cdvdman_pathtblsize = 0;
	FreeSysMemory(g_cdvdman_pathtbl);
	g_cdvdman_pathtbl = 0;
	CpuResumeIntr(state);
	if ( v < 0 )
	{
		VERBOSE_KPRINTF(1, "path_tbl_init Error %d\n", v);
	}
	return (!action) ? num : v;
}

// Unofficial: unused obfuscation code was removed

// clang-format off
__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".global optimized_memcpy" "\n"
	"\t" "optimized_memcpy:" "\n"
	"\t" "    srl         $a3, $a2, 2" "\n"
	"\t" "    beqz        $a3, .Loptimized_memcpy_12" "\n"
	"\t" "     or         $a3, $a0, $a1" "\n"
	"\t" "    andi        $a3, $a3, 0x3" "\n"
	"\t" "    bnez        $a3, .Loptimized_memcpy_3" "\n"
	"\t" "     nop" "\n"
	"\t" "    srl         $a3, $a2, 2" "\n"
	"\t" "    addiu       $at, $zero, 0xC" "\n"
	"\t" "    div         $zero, $a3, $at" "\n"
	"\t" "    mflo        $a3" "\n"
	"\t" "    mfhi        $v1" "\n"
	"\t" "    beqz        $v1, .Loptimized_memcpy_2" "\n"
	"\t" "     nop" "\n"
	"\t" ".Loptimized_memcpy_1:" "\n"
	"\t" "    lw          $v0, 0x0($a1)" "\n"
	"\t" "    addiu       $v1, $v1, -0x1" "\n"
	"\t" "    sw          $v0, 0x0($a0)" "\n"
	"\t" "    addiu       $a1, $a1, 0x4" "\n"
	"\t" "    bnez        $v1, .Loptimized_memcpy_1" "\n"
	"\t" "     addiu      $a0, $a0, 0x4" "\n"
	"\t" "    beqz        $a3, .Loptimized_memcpy_12" "\n"
	"\t" "     nop" "\n"
	"\t" ".Loptimized_memcpy_2:" "\n"
	"\t" "    lw          $v0, 0x0($a1)" "\n"
	"\t" "    lw          $v1, 0x4($a1)" "\n"
	"\t" "    lw          $t0, 0x8($a1)" "\n"
	"\t" "    lw          $t1, 0xC($a1)" "\n"
	"\t" "    lw          $t2, 0x10($a1)" "\n"
	"\t" "    lw          $t3, 0x14($a1)" "\n"
	"\t" "    lw          $t4, 0x18($a1)" "\n"
	"\t" "    lw          $t5, 0x1C($a1)" "\n"
	"\t" "    lw          $t6, 0x20($a1)" "\n"
	"\t" "    lw          $t7, 0x24($a1)" "\n"
	"\t" "    lw          $t8, 0x28($a1)" "\n"
	"\t" "    lw          $t9, 0x2C($a1)" "\n"
	"\t" "    addiu       $a3, $a3, -0x1" "\n"
	"\t" "    sw          $v0, 0x0($a0)" "\n"
	"\t" "    sw          $v1, 0x4($a0)" "\n"
	"\t" "    sw          $t0, 0x8($a0)" "\n"
	"\t" "    sw          $t1, 0xC($a0)" "\n"
	"\t" "    sw          $t2, 0x10($a0)" "\n"
	"\t" "    sw          $t3, 0x14($a0)" "\n"
	"\t" "    sw          $t4, 0x18($a0)" "\n"
	"\t" "    sw          $t5, 0x1C($a0)" "\n"
	"\t" "    sw          $t6, 0x20($a0)" "\n"
	"\t" "    sw          $t7, 0x24($a0)" "\n"
	"\t" "    sw          $t8, 0x28($a0)" "\n"
	"\t" "    sw          $t9, 0x2C($a0)" "\n"
	"\t" "    addiu       $a1, $a1, 0x30" "\n"
	"\t" "    bnez        $a3, .Loptimized_memcpy_2" "\n"
	"\t" "     addiu      $a0, $a0, 0x30" "\n"
	"\t" "    j           .Loptimized_memcpy_12" "\n"
	"\t" "     nop" "\n"
	"\t" ".Loptimized_memcpy_3:" "\n"
	"\t" "    andi        $a3, $a0, 0x3" "\n"
	"\t" "    beqz        $a3, .Loptimized_memcpy_6" "\n"
	"\t" "     andi       $a3, $a1, 0x3" "\n"
	"\t" "    beqz        $a3, .Loptimized_memcpy_6" "\n"
	"\t" "     nop" "\n"
	"\t" "    srl         $a3, $a2, 2" "\n"
	"\t" "    addiu       $at, $zero, 0xC" "\n"
	"\t" "    div         $zero, $a3, $at" "\n"
	"\t" "    mflo        $a3" "\n"
	"\t" "    mfhi        $v1" "\n"
	"\t" "    beqz        $v1, .Loptimized_memcpy_5" "\n"
	"\t" "     nop" "\n"
	"\t" ".Loptimized_memcpy_4:" "\n"
	"\t" "    lwl         $v0, 0x3($a1)" "\n"
	"\t" "    lwr         $v0, 0x0($a1)" "\n"
	"\t" "    addiu       $v1, $v1, -0x1" "\n"
	"\t" "    swl         $v0, 0x3($a0)" "\n"
	"\t" "    swr         $v0, 0x0($a0)" "\n"
	"\t" "    addiu       $a1, $a1, 0x4" "\n"
	"\t" "    bnez        $v1, .Loptimized_memcpy_4" "\n"
	"\t" "     addiu      $a0, $a0, 0x4" "\n"
	"\t" "    beqz        $a3, .Loptimized_memcpy_12" "\n"
	"\t" "     nop" "\n"
	"\t" ".Loptimized_memcpy_5:" "\n"
	"\t" "    lwl         $v0, 0x3($a1)" "\n"
	"\t" "    lwr         $v0, 0x0($a1)" "\n"
	"\t" "    lwl         $v1, 0x7($a1)" "\n"
	"\t" "    lwr         $v1, 0x4($a1)" "\n"
	"\t" "    lwl         $t0, 0xB($a1)" "\n"
	"\t" "    lwr         $t0, 0x8($a1)" "\n"
	"\t" "    lwl         $t1, 0xF($a1)" "\n"
	"\t" "    lwr         $t1, 0xC($a1)" "\n"
	"\t" "    lwl         $t2, 0x13($a1)" "\n"
	"\t" "    lwr         $t2, 0x10($a1)" "\n"
	"\t" "    lwl         $t3, 0x17($a1)" "\n"
	"\t" "    lwr         $t3, 0x14($a1)" "\n"
	"\t" "    lwl         $t4, 0x1B($a1)" "\n"
	"\t" "    lwr         $t4, 0x18($a1)" "\n"
	"\t" "    lwl         $t5, 0x1F($a1)" "\n"
	"\t" "    lwr         $t5, 0x1C($a1)" "\n"
	"\t" "    lwl         $t6, 0x23($a1)" "\n"
	"\t" "    lwr         $t6, 0x20($a1)" "\n"
	"\t" "    lwl         $t7, 0x27($a1)" "\n"
	"\t" "    lwr         $t7, 0x24($a1)" "\n"
	"\t" "    lwl         $t8, 0x2B($a1)" "\n"
	"\t" "    lwr         $t8, 0x28($a1)" "\n"
	"\t" "    lwl         $t9, 0x2F($a1)" "\n"
	"\t" "    lwr         $t9, 0x2C($a1)" "\n"
	"\t" "    addiu       $a3, $a3, -0x1" "\n"
	"\t" "    swl         $v0, 0x3($a0)" "\n"
	"\t" "    swr         $v0, 0x0($a0)" "\n"
	"\t" "    swl         $v1, 0x7($a0)" "\n"
	"\t" "    swr         $v1, 0x4($a0)" "\n"
	"\t" "    swl         $t0, 0xB($a0)" "\n"
	"\t" "    swr         $t0, 0x8($a0)" "\n"
	"\t" "    swl         $t1, 0xF($a0)" "\n"
	"\t" "    swr         $t1, 0xC($a0)" "\n"
	"\t" "    swl         $t2, 0x13($a0)" "\n"
	"\t" "    swr         $t2, 0x10($a0)" "\n"
	"\t" "    swl         $t3, 0x17($a0)" "\n"
	"\t" "    swr         $t3, 0x14($a0)" "\n"
	"\t" "    swl         $t4, 0x1B($a0)" "\n"
	"\t" "    swr         $t4, 0x18($a0)" "\n"
	"\t" "    swl         $t5, 0x1F($a0)" "\n"
	"\t" "    swr         $t5, 0x1C($a0)" "\n"
	"\t" "    swl         $t6, 0x23($a0)" "\n"
	"\t" "    swr         $t6, 0x20($a0)" "\n"
	"\t" "    swl         $t7, 0x27($a0)" "\n"
	"\t" "    swr         $t7, 0x24($a0)" "\n"
	"\t" "    swl         $t8, 0x2B($a0)" "\n"
	"\t" "    swr         $t8, 0x28($a0)" "\n"
	"\t" "    swl         $t9, 0x2F($a0)" "\n"
	"\t" "    swr         $t9, 0x2C($a0)" "\n"
	"\t" "    addiu       $a1, $a1, 0x30" "\n"
	"\t" "    bnez        $a3, .Loptimized_memcpy_5" "\n"
	"\t" "     addiu      $a0, $a0, 0x30" "\n"
	"\t" "    j           .Loptimized_memcpy_12" "\n"
	"\t" "     nop" "\n"
	"\t" ".Loptimized_memcpy_6:" "\n"
	"\t" "    andi        $a3, $a0, 0x3" "\n"
	"\t" "    beqz        $a3, .Loptimized_memcpy_9" "\n"
	"\t" "     nop" "\n"
	"\t" "    srl         $a3, $a2, 2" "\n"
	"\t" "    addiu       $at, $zero, 0xC" "\n"
	"\t" "    div         $zero, $a3, $at" "\n"
	"\t" "    mflo        $a3" "\n"
	"\t" "    mfhi        $v1" "\n"
	"\t" "    beqz        $v1, .Loptimized_memcpy_8" "\n"
	"\t" "     nop" "\n"
	"\t" ".Loptimized_memcpy_7:" "\n"
	"\t" "    lw          $v0, 0x0($a1)" "\n"
	"\t" "    addiu       $v1, $v1, -0x1" "\n"
	"\t" "    swl         $v0, 0x3($a0)" "\n"
	"\t" "    swr         $v0, 0x0($a0)" "\n"
	"\t" "    addiu       $a1, $a1, 0x4" "\n"
	"\t" "    bnez        $v1, .Loptimized_memcpy_7" "\n"
	"\t" "     addiu      $a0, $a0, 0x4" "\n"
	"\t" "    beqz        $a3, .Loptimized_memcpy_12" "\n"
	"\t" "     nop" "\n"
	"\t" ".Loptimized_memcpy_8:" "\n"
	"\t" "    lw          $v0, 0x0($a1)" "\n"
	"\t" "    lw          $v1, 0x4($a1)" "\n"
	"\t" "    lw          $t0, 0x8($a1)" "\n"
	"\t" "    lw          $t1, 0xC($a1)" "\n"
	"\t" "    lw          $t2, 0x10($a1)" "\n"
	"\t" "    lw          $t3, 0x14($a1)" "\n"
	"\t" "    lw          $t4, 0x18($a1)" "\n"
	"\t" "    lw          $t5, 0x1C($a1)" "\n"
	"\t" "    lw          $t6, 0x20($a1)" "\n"
	"\t" "    lw          $t7, 0x24($a1)" "\n"
	"\t" "    lw          $t8, 0x28($a1)" "\n"
	"\t" "    lw          $t9, 0x2C($a1)" "\n"
	"\t" "    addiu       $a3, $a3, -0x1" "\n"
	"\t" "    swl         $v0, 0x3($a0)" "\n"
	"\t" "    swr         $v0, 0x0($a0)" "\n"
	"\t" "    swl         $v1, 0x7($a0)" "\n"
	"\t" "    swr         $v1, 0x4($a0)" "\n"
	"\t" "    swl         $t0, 0xB($a0)" "\n"
	"\t" "    swr         $t0, 0x8($a0)" "\n"
	"\t" "    swl         $t1, 0xF($a0)" "\n"
	"\t" "    swr         $t1, 0xC($a0)" "\n"
	"\t" "    swl         $t2, 0x13($a0)" "\n"
	"\t" "    swr         $t2, 0x10($a0)" "\n"
	"\t" "    swl         $t3, 0x17($a0)" "\n"
	"\t" "    swr         $t3, 0x14($a0)" "\n"
	"\t" "    swl         $t4, 0x1B($a0)" "\n"
	"\t" "    swr         $t4, 0x18($a0)" "\n"
	"\t" "    swl         $t5, 0x1F($a0)" "\n"
	"\t" "    swr         $t5, 0x1C($a0)" "\n"
	"\t" "    swl         $t6, 0x23($a0)" "\n"
	"\t" "    swr         $t6, 0x20($a0)" "\n"
	"\t" "    swl         $t7, 0x27($a0)" "\n"
	"\t" "    swr         $t7, 0x24($a0)" "\n"
	"\t" "    swl         $t8, 0x2B($a0)" "\n"
	"\t" "    swr         $t8, 0x28($a0)" "\n"
	"\t" "    swl         $t9, 0x2F($a0)" "\n"
	"\t" "    swr         $t9, 0x2C($a0)" "\n"
	"\t" "    addiu       $a1, $a1, 0x30" "\n"
	"\t" "    bnez        $a3, .Loptimized_memcpy_8" "\n"
	"\t" "     addiu      $a0, $a0, 0x30" "\n"
	"\t" "    j           .Loptimized_memcpy_12" "\n"
	"\t" "     nop" "\n"
	"\t" ".Loptimized_memcpy_9:" "\n"
	"\t" "    srl         $a3, $a2, 2" "\n"
	"\t" "    addiu       $at, $zero, 0xC" "\n"
	"\t" "    div         $zero, $a3, $at" "\n"
	"\t" "    mflo        $a3" "\n"
	"\t" "    mfhi        $v1" "\n"
	"\t" "    beqz        $v1, .Loptimized_memcpy_11" "\n"
	"\t" "     nop" "\n"
	"\t" ".Loptimized_memcpy_10:" "\n"
	"\t" "    lwl         $v0, 0x3($a1)" "\n"
	"\t" "    lwr         $v0, 0x0($a1)" "\n"
	"\t" "    addiu       $v1, $v1, -0x1" "\n"
	"\t" "    sw          $v0, 0x0($a0)" "\n"
	"\t" "    addiu       $a1, $a1, 0x4" "\n"
	"\t" "    bnez        $v1, .Loptimized_memcpy_10" "\n"
	"\t" "     addiu      $a0, $a0, 0x4" "\n"
	"\t" "    beqz        $a3, .Loptimized_memcpy_12" "\n"
	"\t" "     nop" "\n"
	"\t" ".Loptimized_memcpy_11:" "\n"
	"\t" "    lwl         $v0, 0x3($a1)" "\n"
	"\t" "    lwr         $v0, 0x0($a1)" "\n"
	"\t" "    lwl         $v1, 0x7($a1)" "\n"
	"\t" "    lwr         $v1, 0x4($a1)" "\n"
	"\t" "    lwl         $t0, 0xB($a1)" "\n"
	"\t" "    lwr         $t0, 0x8($a1)" "\n"
	"\t" "    lwl         $t1, 0xF($a1)" "\n"
	"\t" "    lwr         $t1, 0xC($a1)" "\n"
	"\t" "    lwl         $t2, 0x13($a1)" "\n"
	"\t" "    lwr         $t2, 0x10($a1)" "\n"
	"\t" "    lwl         $t3, 0x17($a1)" "\n"
	"\t" "    lwr         $t3, 0x14($a1)" "\n"
	"\t" "    lwl         $t4, 0x1B($a1)" "\n"
	"\t" "    lwr         $t4, 0x18($a1)" "\n"
	"\t" "    lwl         $t5, 0x1F($a1)" "\n"
	"\t" "    lwr         $t5, 0x1C($a1)" "\n"
	"\t" "    lwl         $t6, 0x23($a1)" "\n"
	"\t" "    lwr         $t6, 0x20($a1)" "\n"
	"\t" "    lwl         $t7, 0x27($a1)" "\n"
	"\t" "    lwr         $t7, 0x24($a1)" "\n"
	"\t" "    lwl         $t8, 0x2B($a1)" "\n"
	"\t" "    lwr         $t8, 0x28($a1)" "\n"
	"\t" "    lwl         $t9, 0x2F($a1)" "\n"
	"\t" "    lwr         $t9, 0x2C($a1)" "\n"
	"\t" "    addiu       $a3, $a3, -0x1" "\n"
	"\t" "    sw          $v0, 0x0($a0)" "\n"
	"\t" "    sw          $v1, 0x4($a0)" "\n"
	"\t" "    sw          $t0, 0x8($a0)" "\n"
	"\t" "    sw          $t1, 0xC($a0)" "\n"
	"\t" "    sw          $t2, 0x10($a0)" "\n"
	"\t" "    sw          $t3, 0x14($a0)" "\n"
	"\t" "    sw          $t4, 0x18($a0)" "\n"
	"\t" "    sw          $t5, 0x1C($a0)" "\n"
	"\t" "    sw          $t6, 0x20($a0)" "\n"
	"\t" "    sw          $t7, 0x24($a0)" "\n"
	"\t" "    sw          $t8, 0x28($a0)" "\n"
	"\t" "    sw          $t9, 0x2C($a0)" "\n"
	"\t" "    addiu       $a1, $a1, 0x30" "\n"
	"\t" "    bnez        $a3, .Loptimized_memcpy_11" "\n"
	"\t" "     addiu      $a0, $a0, 0x30" "\n"
	"\t" ".Loptimized_memcpy_12:" "\n"
	"\t" "    andi        $v1, $a2, 0x3" "\n"
	"\t" "    beqz        $v1, .Loptimized_memcpy_14" "\n"
	"\t" "     nop" "\n"
	"\t" ".Loptimized_memcpy_13:" "\n"
	"\t" "    lb          $v0, 0x0($a1)" "\n"
	"\t" "    addiu       $v1, $v1, -0x1" "\n"
	"\t" "    sb          $v0, 0x0($a0)" "\n"
	"\t" "    addiu       $a1, $a1, 0x1" "\n"
	"\t" "    bnez        $v1, .Loptimized_memcpy_13" "\n"
	"\t" "     addiu      $a0, $a0, 0x1" "\n"
	"\t" ".Loptimized_memcpy_14:" "\n"
	"\t" "    addu        $v0, $a2, $zero" "\n"
	"\t" "    jr          $ra" "\n"
	"\t" "     nop" "\n"
	"\t" ".set pop" "\n"
);
// clang-format on

#ifdef DEAD_CODE
void hex_dump(u8 *addr_start, int length)
{
	int i;

	KPRINTF("Hex Dump addr %08x\n", addr_start);
	for ( i = 0; i < length; i += 1 )
	{
		if ( !(i & 0xF) && i )
		{
			PRINTF("\n");
		}
		KPRINTF(" %02x", addr_start[i]);
	}
	KPRINTF("\n");
}
#endif

static int cdvdman_initcfg(void)
{
	int i;
	u8 m_version[5];
	u32 eflag;

	eflag = 0;
	for ( i = 0; i <= 100; i += 1 )
	{
		unsigned int mvored;

#ifdef CDVD_VARIANT_XOSD
		if ( !sceCdMV(m_version, &eflag) )
#else
		if ( !sceCdMV(m_version, &eflag) && (eflag & 0x80) )
#endif
		{
			vDelayThread(2000);
			VERBOSE_KPRINTF(1, "_sceCdMV error\n");
		}
		mvored = m_version[3] | (m_version[2] << 8) | (m_version[1] << 16);
		g_cdvdman_emudvd9 = m_version[2] & 1;
		VERBOSE_KPRINTF(1, "MV %02x %02x %02x %02x\n", m_version[0], m_version[1], m_version[2], m_version[3]);
		g_cdvdman_minver_10700 = mvored >= 0x10700;
		g_cdvdman_minver_20200 = mvored >= 0x20200;
		g_cdvdman_minver_20400 = mvored >= 0x20400;
		g_cdvdman_minver_20800 = mvored >= 0x20800;
		g_cdvdman_minver_50000 = mvored >= 0x50000;
		g_cdvdman_minver_50200 = mvored >= 0x50200;
		g_cdvdman_minver_50400 = mvored >= 0x50400;
		g_cdvdman_minver_50600 = mvored >= 0x50600;
#ifdef CDVD_VARIANT_XOSD
		VERBOSE_KPRINTF(0, "M Version %02x %02x %02x %02x\n", m_version[0], m_version[1], m_version[2], m_version[3]);
		if ( cdvdman_get_renewal_date(m_version, &eflag) )
		{
			VERBOSE_KPRINTF(
				0,
				"M Renewal Date 20%02x/%02x/%02x %02x:%02x\n",
				m_version[0],
				m_version[1],
				m_version[2],
				m_version[3],
				m_version[4]);
		}
		g_cdvdman_vernotxxx1x = 0;
		if ( g_cdvdman_minver_50600 )
		{
			// cdvdman_verxxxx1 = (mvored & 0xF) == 1; // not referenced!
			KPRINTF("X_model ");
			switch ( mvored & 0xF0 )
			{
				case 0x00:
					KPRINTF("量産 OR 量産試作機\n");
					break;
				case 0x10:
					KPRINTF("原理試作機\n");
					g_cdvdman_vernotxxx1x = 1;
					break;
				case 0x20:
					KPRINTF("手作り試作機\n");
					break;
				case 0x30:
					KPRINTF("型物試作機\n");
					break;
				default:
					KPRINTF("Unkown\n");
					break;
			}
		}
		else
		{
			KPRINTF("X_model 旧原理試作機 Ver5.6未満 Found\n");
		}
#endif
		g_cdvdman_minver_x_model_15 = (mvored & 0xF) == 1;
		g_cdvdman_minver_60000 = mvored >= 0x60000;
		g_cdvdman_minver_60200 = mvored >= 0x60200;
#ifdef CDVD_VARIANT_OSD
		g_cdvdman_minver_60600 = mvored >= 0x60600;
#endif
		return 1;
	}
	return 0;
}

static int vSetAlarm(iop_sys_clock_t *sys_clock, unsigned int (*alarm_cb)(void *arg), void *arg)
{
	return (QueryIntrContext() ? iSetAlarm : SetAlarm)(sys_clock, alarm_cb, arg);
}

static int vCancelAlarm(unsigned int (*alarm_cb)(void *arg), void *arg)
{
	return (QueryIntrContext() ? iCancelAlarm : CancelAlarm)(alarm_cb, arg);
}

#ifdef DEAD_CODE
s32 vSignalSema(s32 sema_id)
{
	return (QueryIntrContext() ? iSignalSema : SignalSema)(sema_id);
}
#endif

static int vSetEventFlag(int ef, u32 bits)
{
	return (QueryIntrContext() ? iSetEventFlag : SetEventFlag)(ef, bits);
}

static int vClearEventFlag(int ef, u32 bits)
{
	return (QueryIntrContext() ? iClearEventFlag : ClearEventFlag)(ef, bits);
}

static int vReferEventFlagStatus(int ef, iop_event_info_t *info)
{
	return (QueryIntrContext() ? iReferEventFlagStatus : ReferEventFlagStatus)(ef, info);
}

static int vPollEventFlag(int ef, u32 bits, int mode, u32 *efbits)
{
#ifdef CDVD_VARIANT_XOSD
	int retres;
	iop_event_info_t efinfo;

	if ( !QueryIntrContext() )
	{
		return PollEventFlag(ef, bits, mode, efbits);
	}
	retres = iReferEventFlagStatus(ef, &efinfo);
	if ( retres )
	{
		return retres;
	}
	*efbits = efinfo.currBits;
	if ( (mode & 1) ? !(efinfo.currBits & bits) : ((efinfo.currBits & bits) != bits) )
	{
		return KE_EVF_COND;
	}
	if ( mode & 0x10 )
	{
		iClearEventFlag(ef, ~bits);
	}
	return 0;
#else
	return PollEventFlag(ef, bits, mode, efbits);
#endif
}

static int vDelayThread(int usec)
{
	int intrval;
	int state;

	intrval = CpuSuspendIntr(&state);
	CpuResumeIntr(state);
	return (!QueryIntrContext() && !intrval) ? DelayThread(usec) : 0;
}

#ifdef CDVD_VARIANT_XOSD
static int get_cdvd_register(int regnr)
{
	int intrres;
	int IntrContext;
	int regval;
	iop_event_info_t efinfo;
	int state;
	u32 efbits;

	intrres = CpuSuspendIntr(&state);
	CpuResumeIntr(state);
	IntrContext = QueryIntrContext();
	if ( !IntrContext && !intrres )
	{
		WaitEventFlag(g_cdvdman_csys_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
		CpuSuspendIntr(&state);
	}
	else
	{
		if ( IntrContext )
		{
			iReferEventFlagStatus(g_cdvdman_csys_evfid, &efinfo);
		}
		else
		{
			ReferEventFlagStatus(g_cdvdman_csys_evfid, &efinfo);
		}
		if ( !(efinfo.currBits & 1) )
		{
			return -1;
		}
	}
	if ( update_cd_mode_ps2_atapi() != 1 )
	{
		regval = ((vu8 *)(0xBF402000))[regnr];
	}
	else
	{
		if ( !IntrContext && !intrres )
		{
			CpuResumeIntr(state);
			if ( g_cd_atapi_evfid != -1 )
			{
				WaitEventFlag(g_cd_atapi_evfid, 3, WEF_AND | WEF_CLEAR, &efbits);
			}
			if ( g_adma_evfid != -1 )
			{
				WaitEventFlag(g_adma_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
			}
			if ( g_acmd_evfid != -1 )
			{
				WaitEventFlag(g_acmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
			}
			CpuSuspendIntr(&state);
		}
		else
		{
			if ( g_cd_atapi_evfid != -1 )
			{
				if ( IntrContext )
				{
					iReferEventFlagStatus(g_cd_atapi_evfid, &efinfo);
				}
				else
				{
					ReferEventFlagStatus(g_cd_atapi_evfid, &efinfo);
				}
				if ( !(efinfo.currBits & 3) )
				{
					return -1;
				}
			}
			if ( g_adma_evfid != -1 )
			{
				if ( IntrContext )
				{
					iReferEventFlagStatus(g_adma_evfid, &efinfo);
				}
				else
				{
					ReferEventFlagStatus(g_adma_evfid, &efinfo);
				}
				if ( !(efinfo.currBits & 1) )
				{
					return -1;
				}
			}
			if ( g_acmd_evfid != -1 )
			{
				if ( IntrContext )
				{
					iReferEventFlagStatus(g_acmd_evfid, &efinfo);
				}
				else
				{
					ReferEventFlagStatus(g_acmd_evfid, &efinfo);
				}
			}
			if ( !(efinfo.currBits & 1) )
			{
				return -1;
			}
		}
		set_cdvd_dev5_base_addr_atapi(2);
		regval = ((vu8 *)(0xBF402000))[regnr];
		set_cdvd_dev5_base_addr_atapi(1);
		if ( !IntrContext && !intrres )
		{
			if ( g_acmd_evfid != -1 )
			{
				SetEventFlag(g_acmd_evfid, 1);
			}
			if ( g_adma_evfid != -1 )
			{
				SetEventFlag(g_adma_evfid, 1);
			}
			if ( g_cd_atapi_evfid != -1 )
			{
				SetEventFlag(g_cd_atapi_evfid, 3);
			}
		}
	}
	if ( !IntrContext && !intrres )
	{
		CpuResumeIntr(state);
		SetEventFlag(g_cdvdman_csys_evfid, 1);
	}
	return regval;
}
#endif

// cppcheck-suppress constParameterCallback
static unsigned int read_timeout_alarm_cb(void *userdata)
{
	int read_timeout;
	const iop_sys_clock_t *sys_clock;

	sys_clock = (const iop_sys_clock_t *)userdata;
	read_timeout = sys_clock->lo / 0x9000;
	KPRINTF("Read Time Out %d(msec)\n", read_timeout);
	sceCdSC(0xFFFFFFEE, &read_timeout);
	return !sceCdBreak();
}

sceCdCBFunc sceCdCallback(sceCdCBFunc function)
{
	sceCdCBFunc rc;
	u32 efbits;

	if ( sceCdSync(1) || PollEventFlag(g_ncmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND )
	{
		return 0;
	}
	rc = g_cdvdman_user_cb;
	g_cdvdman_user_cb = function;
	vSetEventFlag(g_ncmd_evfid, 1);
	return rc;
}

// cppcheck-suppress funcArgNamesDifferent
void *sceCdPOffCallback(void (*func)(void *userdata), void *userdata)
{
	void *old_cb;
	int state;

	CpuSuspendIntr(&state);
	old_cb = g_cdvdman_power_off_callback;
	g_cdvdman_power_off_callback = func;
	g_cdvdman_power_off_callback_userdata = userdata;
	CpuResumeIntr(state);
	return old_cb;
}

#ifdef CDVD_VARIANT_XOSD
void *sceCdSetAtapiEjectCallback(int (*func)(int reason, void *userdata), void *userdata)
{
	void *old_cb;
	int state;

	CpuSuspendIntr(&state);
	old_cb = g_cdvdman_atapi_eject_bs_power_callback;
	g_cdvdman_atapi_eject_bs_power_callback = func;
	g_cdvdman_atapi_eject_bs_power_callback_userdata = userdata;
	CpuResumeIntr(state);
	return old_cb;
}
#endif

int sceCdstm0Cb(void (*p)(int val))
{
	g_cdvdman_cdstm0cb = p;
	return 0;
}

int sceCdstm1Cb(void (*p)(int val))
{
	g_cdvdman_cdstm1cb = p;
	return 0;
}

static int cdvdman_intr_cb(cdvdman_internal_struct_t *s)
{
	sceCdRMode cdrmode;
	int oldstate;
	int ext_passthrough;
	USE_DEV5_MMIO_HWPORT();

	ext_passthrough = 0;
	s->m_wait_flag = s->m_waf_set_test;
	iSetEventFlag(g_cdvdman_intr_evfid, 0x29);
	DisableIntr(IOP_IRQ_DMA_CDVD, &oldstate);
	if ( s->m_cdvdman_command == SCECdFuncStandby && s->m_last_error == SCECdErTRMOPN )
	{
		s->m_last_error = (sceCdStatus() == SCECdErFAIL) ? SCECdErTRMOPN : SCECdErNO;
	}
	if ( s->m_last_error == SCECdErEOM )
	{
		s->m_last_error = SCECdErNO;
	}
	VERBOSE_KPRINTF(
		1, "Intr call func_num: %d Err= %02x OnTout= %d\n", g_cdvdman_cmdfunc, (u8)s->m_last_error, s->m_last_read_timeout);
	if ( !s->m_scmd_flag )
	{
		cdvdman_write_scmd(s);
	}
	if (
		(((u8)s->m_last_error == SCECdErREAD && g_cdvdman_cmdfunc == SCECdFuncRead)
		 || ((u8)s->m_last_error == SCECdErABRT && s->m_last_read_timeout && g_cdvdman_last_cmdfunc == SCECdFuncRead))
		&& !g_cdvdman_minver_20200 && !s->m_stream_flag && !s->m_dvd_flag && !s->m_recover_status
		&& s->m_read_mode.trycount != 1 )
	{
		s->m_sync_error = 0;
		s->m_interupt_read_state = 0;
		if ( s->m_dec_mode_set )
		{
			s->m_dec_mode_last_set = 1;
		}
		else
		{
			VERBOSE_KPRINTF(1, "dec mode 0x00\n");
			s->m_read_chunk_reprocial_32 = 1 + (0x20 / ((!s->m_read_chunk) ? s->m_read_sectors : s->m_read_chunk));
			s->m_dintrlsn = (s->m_read_lsn < 0x61) ? (s->m_read_lsn + s->m_read_sectors + 48) : (s->m_read_lsn - 80);
			s->m_read_mode.spindlctrl = 16;
			if ( !sceCdRead0_Rty(
						 s->m_dintrlsn,
						 (!s->m_read_chunk) ? s->m_read_sectors : s->m_read_chunk,
						 s->m_read_buf,
						 &s->m_read_mode,
						 (u8)s->m_cdvdman_command,
						 0,
						 0) )
			{
				s->m_last_error = SCECdErREAD;
				s->m_recover_status = 0;
			}
			else
			{
				ext_passthrough = 1;
			}
		}
	}
	if ( !ext_passthrough )
	{
		char dev5_reg_013_masked;

		VERBOSE_KPRINTF(1, "Recover_Stat:%d\n", s->m_recover_status);
		dev5_reg_013_masked = dev5_mmio_hwport->m_dev5_reg_013 & 0xF;
		if ( dev5_reg_013_masked )
		{
			if (
				((u8)s->m_last_error == SCECdErREAD || ((u8)s->m_last_error == SCECdErABRT && s->m_last_read_timeout))
				&& !s->m_recover_status && !s->m_stream_flag && g_cdvdman_cmdfunc != 9 && g_cdvdman_cmdfunc != SCECdFuncReadCDDA
				&& (unsigned int)s->m_read_mode.trycount - 1 >= 4 )
			{
				s->m_sync_error = 0;
				s->m_interupt_read_state = 0;
				if ( s->m_dec_mode_set )
				{
					s->m_dec_mode_last_set = 2;
				}
				else
				{
					VERBOSE_KPRINTF(1, "dec mode 0x01\n");
					cdrmode.trycount = s->m_read_mode.trycount;
					cdrmode.spindlctrl = dev5_reg_013_masked + 13;
					cdrmode.datapattern = s->m_read_mode.datapattern;
					if ( sceCdRead0_Rty(
								 s->m_read_lsn,
								 s->m_read_sectors,
								 s->m_read_buf,
								 &cdrmode,
								 (u8)s->m_cdvdman_command,
								 s->m_read_chunk,
								 s->m_read_callback) )
					{
						s->m_last_error = SCECdErNO;
						return 1;
					}
					s->m_last_error = SCECdErREAD;
				}
			}
		}
		s->m_last_read_timeout = 0;
		switch ( s->m_recover_status )
		{
			case 1:
				s->m_sync_error = 0;
				s->m_interupt_read_state = 0;
				if (
					s->m_last_error != SCECdErNO
					|| !sceCdRead0_Rty(
						s->m_dintrlsn,
						(!s->m_read_chunk) ? s->m_read_sectors : s->m_read_chunk,
						s->m_read_buf,
						&s->m_read_mode,
						(u8)s->m_cdvdman_command,
						0,
						0) )
				{
					s->m_last_error = SCECdErREAD;
					s->m_recover_status = 0;
				}
				else
				{
					ext_passthrough = 1;
				}
				break;
			case 2:
				s->m_sync_error = 0;
				s->m_interupt_read_state = 0;
				if ( sceCdRead0(
							 s->m_read_lsn, s->m_read_sectors, s->m_read_buf, &s->m_read_mode, s->m_read_chunk, s->m_read_callback) )
				{
					s->m_last_error = SCECdErNO;
					s->m_recover_status = 3;
					return 1;
				}
				s->m_last_error = SCECdErREAD;
				s->m_recover_status = 0;
				break;
			case 3:
				s->m_recover_status = 0;
				break;
			default:
				break;
		}
	}
	if ( ext_passthrough )
	{
		s->m_last_error = SCECdErNO;
		s->m_dintrlsn += s->m_read_sectors;
		s->m_read_chunk_reprocial_32 -= 1;
		s->m_recover_status = (!s->m_read_chunk_reprocial_32) ? 2 : 1;
		return 1;
	}
	if ( s->m_dec_state )
	{
		sceCdDecSet(0, 0, 0);
	}
	if ( (s->m_read2_flag == 1 || s->m_read2_flag == 3) && !s->m_use_toc )
	{
		VERBOSE_KPRINTF(1, "call Read2intrCDVD()\n");
		Read2intrCDVD(s->m_read2_flag);
	}
	s->m_sync_error = 0;
	s->m_interupt_read_state = 0;
	if ( s->m_dec_state == 2 )
	{
		s->m_dec_state = 0;
	}
	if ( s->m_stream_flag == 1 && !s->m_use_toc && !s->m_read2_flag )
	{
		if ( g_cdvdman_cdstm0cb )
		{
			g_cdvdman_cdstm0cb(1);
		}
		else
		{
			VERBOSE_KPRINTF(1, "Intr func0 no seting");
		}
	}
	if ( !s->m_read2_flag )
	{
		if ( s->m_stream_flag == 2 && !s->m_use_toc )
		{
			if ( g_cdvdman_cdstm1cb )
			{
				g_cdvdman_cdstm1cb(1);
			}
			else
			{
				VERBOSE_KPRINTF(1, "Intr func1 no seting");
			}
		}
		else
		{
			g_cdvdman_readptr = 0;
		}
	}

	VERBOSE_KPRINTF(
		1, "Intr call user callback func_addr %08x num %d flg %d\n", g_cdvdman_user_cb, g_cdvdman_cmdfunc, s->m_read2_flag);
	if ( g_cdvdman_user_cb && g_cdvdman_cmdfunc && !s->m_read2_flag && !s->m_use_toc )
	{
		int cmdfunc_tmp;

		cmdfunc_tmp = g_cdvdman_cmdfunc;
		g_cdvdman_cmdfunc = 0;
		if ( cmdfunc_tmp == 14 || cmdfunc_tmp == 9 )
		{
			cmdfunc_tmp = SCECdFuncRead;
		}
		g_cdvdman_user_cb(cmdfunc_tmp);
	}
	if ( !g_cdvdman_user_cb )
	{
		g_cdvdman_cmdfunc = 0;
	}
	return 1;
}

static int intrh_cdrom(void *userdata)
{
	int conds1;
	iop_event_info_t efinfo;
#ifdef CDVD_VARIANT_XOSD
	int condaddr5;
#endif
	int intr_cb_res;
	cdvdman_internal_struct_t *s;
	USE_DEV5_MMIO_HWPORT();

	s = (cdvdman_internal_struct_t *)userdata;
	conds1 = 0;
#ifdef CDVD_VARIANT_XOSD
	if ( s->m_cd_mode_ps2_atapi == 1 )
	{
		int spd_intr_stat_cur;
		int atapi_intr_reason;

		VERBOSE_KPRINTF(1, "Atapi Intr Called %d\n", g_cdvdman_debug_atapi_intr++);
		condaddr5 = (unsigned int)GetBaseAddress(5) == 0x1F402000;
		set_cdvd_dev5_base_addr_atapi(1);
		(void)*(vu16 *)0xBF410028;  // r_spd_intr_stat
		set_cdvd_dev5_base_addr_atapi(2);
		atapi_intr_reason = dev5_mmio_hwport->m_dev5_reg_008;
		if ( (atapi_intr_reason & 1) )
		{
			conds1 = 1;
			dev5_mmio_hwport->m_dev5_reg_008 = 1;
			VERBOSE_KPRINTF(1, "ATAPI DRAGON CMDCMPL Interrupt\n");
		}
		if ( atapi_intr_reason & 4 )
		{
			dev5_mmio_hwport->m_dev5_reg_008 = 4;
			conds1 = 1;
			iSetEventFlag(g_cdvdman_intr_evfid, 4);
			iSetEventFlag(g_cdvdman_intr_evfid, 0x10);
			set_cdvd_dev5_base_addr_atapi(1);
			VERBOSE_KPRINTF(1, "ATAPI POFFRDY Interrupt\n");
			if ( g_cdvdman_power_off_callback )
			{
				g_cdvdman_power_off_callback(g_cdvdman_power_off_callback_userdata);
			}
			set_cdvd_dev5_base_addr_atapi(2);
		}
		if ( (atapi_intr_reason & 8) )
		{
			conds1 = 1;
			dev5_mmio_hwport->m_dev5_reg_008 = 8;
			if ( !s->m_medium_removal_state )
			{
				s->m_atapi_disk_ejected = 0;
			}
			set_cdvd_dev5_base_addr_atapi(1);
			VERBOSE_KPRINTF(1, "ATAPI EJECT Interrupt\n");
			if ( s->m_cd_atapi_intr_callback && !s->m_medium_removal_state )
			{
				VERBOSE_KPRINTF(1, "EJECT Stop Call\n");
				s->m_cd_atapi_intr_callback(1);
			}
			iSetEventFlag(g_cdvdman_intr_evfid, 0x40);
			if ( g_cdvdman_atapi_eject_bs_power_callback )
			{
				VERBOSE_KPRINTF(1, "ATAPI EJECT Callback %p\n", g_cdvdman_atapi_eject_bs_power_callback);
				g_cdvdman_atapi_eject_bs_power_callback(1, g_cdvdman_atapi_eject_bs_power_callback_userdata);
			}
			set_cdvd_dev5_base_addr_atapi(2);
		}
		if ( (atapi_intr_reason & 0x10) )
		{
			dev5_mmio_hwport->m_dev5_reg_008 = 0x10;
			conds1 = 1;
			set_cdvd_dev5_base_addr_atapi(1);
			VERBOSE_KPRINTF(1, "ATAPI BS_Power DET Interrupt\n");
			iSetEventFlag(g_cdvdman_intr_evfid, 0x80);
			if ( g_cdvdman_atapi_eject_bs_power_callback )
			{
				VERBOSE_KPRINTF(1, "ATAPI BS_Power Callback %p\n", g_cdvdman_atapi_eject_bs_power_callback);
				g_cdvdman_atapi_eject_bs_power_callback(2, g_cdvdman_atapi_eject_bs_power_callback_userdata);
			}
		}
		set_cdvd_dev5_base_addr_atapi(1);
		if ( g_cdvdman_vernotxxx1x && conds1 )
		{
			*(vu16 *)0xBF41002A &= ~0x100;  // r_spd_intr_mask
			*(vu16 *)0xBF41002A |= 0x100;   // r_spd_intr_mask
			(void)*(vu16 *)0xBF41002A;      // r_spd_intr_mask
		}
		spd_intr_stat_cur = *(vu16 *)0xBF410028;  // r_spd_intr_stat
		VERBOSE_KPRINTF(
			1,
			"Atapi Intr Mode:%d addr:%08x Reason:%02x SPD_INTR_STAT %x\n",
			s->m_cd_mode_ps2_atapi,
			s->m_cd_atapi_intr_callback,
			atapi_intr_reason,
			spd_intr_stat_cur);
		if ( (spd_intr_stat_cur & 3) && (*g_cdvdreg_bf80146c & 4) )
		{
			if ( s->m_cd_atapi_intr_callback )
			{
				s->m_cd_atapi_intr_callback(0);
				VERBOSE_KPRINTF(1, "Atapi Intr called\n");
			}
			if ( !s->m_scmd_flag )
			{
				VERBOSE_KPRINTF(1, "Intr call set_prev_command.\n");
				set_cdvd_dev5_base_addr_atapi(2);
				cdvdman_write_scmd(s);
				set_cdvd_dev5_base_addr_atapi(1);
			}
		}
		else if ( !conds1 )
		{
			if ( (*g_cdvdreg_bf80146c & 4) )
			{
				VERBOSE_KPRINTF(1, "No_Reason Intr\n");
			}
			else
			{
				VERBOSE_KPRINTF(1, "already 9611 Power Off Intr\n");
			}
		}
		if ( condaddr5 )
		{
			set_cdvd_dev5_base_addr_atapi(2);
			VERBOSE_KPRINTF(1, "Cur PS2 IO Intr Called\n");
		}
		return 1;
	}
	VERBOSE_KPRINTF(1, "Dragon Intr Called\n");
	condaddr5 = (unsigned int)GetBaseAddress(5) == 0xBF410000;
	if ( condaddr5 )
	{
		set_cdvd_dev5_base_addr_atapi(2);
	}
#endif
	s->m_waf_set_test = s->m_wait_flag;
	if ( (u8)s->m_last_error != SCECdErABRT )
	{
		s->m_last_error = dev5_mmio_hwport->m_dev5_reg_006;
	}
	if ( (dev5_mmio_hwport->m_dev5_reg_008 & 1) )
	{
		s->m_waf_set_test = (!(dev5_mmio_hwport->m_dev5_reg_005 & 1)) ? 1 : -1;
		dev5_mmio_hwport->m_dev5_reg_008 = 1;
		conds1 = 1;
	}
	if ( (dev5_mmio_hwport->m_dev5_reg_008 & 4) )
	{
		dev5_mmio_hwport->m_dev5_reg_008 = 4;
		iSetEventFlag(g_cdvdman_intr_evfid, 4);
		iSetEventFlag(g_cdvdman_intr_evfid, 0x10);
		// The following Kprintf was added for ioprp300x
		VERBOSE_KPRINTF(1, "PS2 POWER OFF Interrupt\n");
		if ( g_cdvdman_power_off_callback )
		{
			g_cdvdman_power_off_callback(g_cdvdman_power_off_callback_userdata);
		}
		if ( !conds1 )
		{
			return 1;
		}
	}
	else
	{
		s->m_waf_set_test = 1;
		s->m_ncmd_intr_count += 1;
		dev5_mmio_hwport->m_dev5_reg_008 = 2;
	}
#ifdef CDVD_VARIANT_XOSD
	if ( (dev5_mmio_hwport->m_dev5_reg_008 & 8) )
	{
		dev5_mmio_hwport->m_dev5_reg_008 = 8;
		VERBOSE_KPRINTF(1, "PS2 EJECT Interrupt\n");
		iSetEventFlag(g_cdvdman_intr_evfid, 0x40);
		if ( !s->m_medium_removal_state )
		{
			s->m_atapi_disk_ejected = 0;
		}
		if ( g_cdvdman_atapi_eject_bs_power_callback )
		{
			VERBOSE_KPRINTF(1, "PS2 EJECT Callback %p\n", g_cdvdman_atapi_eject_bs_power_callback);
			g_cdvdman_atapi_eject_bs_power_callback(1, g_cdvdman_atapi_eject_bs_power_callback_userdata);
		}
	}
	if ( (dev5_mmio_hwport->m_dev5_reg_008 & 0x10) )
	{
		dev5_mmio_hwport->m_dev5_reg_008 = 0x10;
		VERBOSE_KPRINTF(1, "PS2 BS_Power DET Interrupt\n");
		iSetEventFlag(g_cdvdman_intr_evfid, 0x80);
		if ( g_cdvdman_atapi_eject_bs_power_callback )
		{
			VERBOSE_KPRINTF(1, "PS2 BS_Power Callback %p\n", g_cdvdman_atapi_eject_bs_power_callback);
			g_cdvdman_atapi_eject_bs_power_callback(2, g_cdvdman_atapi_eject_bs_power_callback_userdata);
		}
	}
#endif
	iReferEventFlagStatus(g_cdvdman_intr_evfid, &efinfo);
	if ( !(efinfo.currBits & 0x20) )
	{
		// The following Kprintf was added for ioprp300x
		VERBOSE_KPRINTF(1, "Now DMA Working evfid:%08x\n", efinfo.currBits);
		if ( s->m_last_error == SCECdErNO )
		{
			s->m_drive_interupt_request = 1;
#ifdef CDVD_VARIANT_XOSD
			if ( condaddr5 )
			{
				set_cdvd_dev5_base_addr_atapi(1);
			}
#endif
			return 1;
		}
		if ( s->m_last_error == SCECdErEOM )
		{
			intrh_dma_3(s);
		}
	}
	intr_cb_res = cdvdman_intr_cb(s);
#ifdef CDVD_VARIANT_XOSD
	if ( condaddr5 )
	{
		set_cdvd_dev5_base_addr_atapi(1);
	}
#endif
	return intr_cb_res;
}

static u32 cdvdman_l1start(const u8 *toc)
{
	return toc[23] + (toc[22] << 8) + (toc[21] << 16) - 0x30000 + 1;
}

static int DvdDual_infochk(void)
{
#ifdef CDVD_VARIANT_OSD
	// From DVD Player 3.11
	if ( QueryIntrContext() || !(cdvdman_mediactl(3) || (u8)g_cdvdman_istruct.m_opo_or_para == 0xFF) )
#else
	if ( QueryIntrContext() || !(cdvdman_mediactl(4) || (u8)g_cdvdman_istruct.m_opo_or_para == 0xFF) )
#endif
	{
		return 1;
	}
	g_cdvdman_istruct.m_use_toc = 1;
	if ( !cdvdman_gettoc(g_cdvdman_ptoc) )
	{
		g_cdvdman_istruct.m_use_toc = 0;
		g_cdvdman_istruct.m_opo_or_para = -1;
		return 0;
	}
	g_cdvdman_istruct.m_use_toc = 0;
	g_cdvdman_istruct.m_layer_1_lsn = cdvdman_l1start(g_cdvdman_ptoc);
	g_cdvdman_istruct.m_opo_or_para = ((g_cdvdman_ptoc[14] & 0x60)) ? (((g_cdvdman_ptoc[14] & 0x10)) ? 2 : 1) : 0;
	if ( g_cdvdman_istruct.m_dual_layer_emulation )
	{
		VERBOSE_KPRINTF(1, "CDVD:DualEmuON\n");
		g_cdvdman_istruct.m_layer_1_lsn = g_cdvdman_istruct.m_current_dvd_lsn;
		g_cdvdman_istruct.m_opo_or_para = 0;
	}
	VERBOSE_KPRINTF(
		1,
		"DvdDual_info: %02x\tLayer1_LSN:%d opo_or_para %d\n",
		g_cdvdman_ptoc[14],
		g_cdvdman_istruct.m_layer_1_lsn,
		(u8)g_cdvdman_istruct.m_opo_or_para);
	return 1;
}

static u32 sceCdLsnDualChg(u32 lsn)
{
	int layer_disk_needed;
	u32 change_lsn;
	sceCdRMode cdrmode;
	int has_change_lsn;

	layer_disk_needed = 2;
	has_change_lsn = 0;
	if ( cdvdman_isdvd() && DvdDual_infochk() )
	{
		if ( g_cdvdman_istruct.m_dual_layer_emulation )
		{
			if ( !g_cdvdman_istruct.m_current_dvd && lsn >= g_cdvdman_istruct.m_current_dvd_lsn )
			{
				layer_disk_needed = 1;
			}
			if ( g_cdvdman_istruct.m_current_dvd && lsn < g_cdvdman_istruct.m_current_dvd_lsn )
			{
				layer_disk_needed = 0;
			}
			if ( layer_disk_needed == 2 )
			{
				change_lsn = lsn - ((g_cdvdman_istruct.m_current_dvd) ? g_cdvdman_istruct.m_current_dvd_lsn : 0);
				has_change_lsn = 1;
			}
			else if ( !QueryIntrContext() )
			{
				u32 traychk;

				VERBOSE_KPRINTF(0, "CDVD: Exchange it for the Layer_%d_Disk Please.\n", layer_disk_needed);
				while ( g_cdvdman_minver_60200 ? !cdvdman_scmd_sender_3B(0) : !sceCdTrayReq(SCECdTrayOpen, &traychk) )
				{
					;
				}
				cdrmode.trycount = 0;
				cdrmode.spindlctrl = SCECdSpinStm;
				cdrmode.datapattern = SCECdSecS2048;
				g_cdvdman_istruct.m_use_toc = 1;
				while ( layer_disk_needed != 2 )
				{
					if ( cdvdman_isdvd() )
					{
						int read0_result;

						read0_result = sceCdRead0(0xE, 1, g_cdvdman_ptoc, &cdrmode, 0, 0);
						sceCdSync(3);
						if ( g_cdvdman_istruct.m_last_error == SCECdErNO || read0_result )
						{
							int i;

							for ( i = 0; i < 20; i += 1 )
							{
								if ( g_cdvdman_ptoc[i + 104] != g_masterdisc_header[i] )
								{
									break;
								}
							}
							if ( i == 20 && g_cdvdman_ptoc[131] == 2 && (g_cdvdman_ptoc[132] & 2) )
							{
								if ( layer_disk_needed == g_cdvdman_ptoc[133] )
								{
									g_cdvdman_istruct.m_current_dvd = layer_disk_needed;
									layer_disk_needed = 2;
								}
								else
								{
									VERBOSE_KPRINTF(0, "CDVD: Layer_%d Disk not Found\n", layer_disk_needed);
									VERBOSE_KPRINTF(0, "CDVD: Exchange it for the Layer_%d_Disk Please.\n", layer_disk_needed);
									if ( !g_cdvdman_istruct.m_current_dvd && lsn >= g_cdvdman_istruct.m_current_dvd_lsn )
									{
										layer_disk_needed = 1;
									}
									while ( g_cdvdman_minver_60200 ? !cdvdman_scmd_sender_3B(0) : !sceCdTrayReq(SCECdTrayOpen, &traychk) )
									{
										;
									}
								}
							}
							else
							{
								VERBOSE_KPRINTF(0, "CDVD: Not Master Disk %s\n", (const char *)&g_cdvdman_ptoc[i + 104]);
								while ( g_cdvdman_minver_60200 ? !cdvdman_scmd_sender_3B(0) : !sceCdTrayReq(SCECdTrayOpen, &traychk) )
								{
									;
								}
							}
						}
						else
						{
							VERBOSE_KPRINTF(1, "CDVD: LsnDualChg Read Error %02x, %d\n", (u8)g_cdvdman_istruct.m_last_error, 0);
						}
					}
					else
					{
						vDelayThread(16000);
					}
				}
				change_lsn = lsn - ((g_cdvdman_istruct.m_current_dvd) ? g_cdvdman_istruct.m_current_dvd_lsn : 0);
				g_cdvdman_istruct.m_use_toc = 0;
				has_change_lsn = 1;
			}
		}
		else
		{
			change_lsn =
				lsn - ((g_cdvdman_istruct.m_opo_or_para && (lsn >= (u32)g_cdvdman_istruct.m_layer_1_lsn)) ? 0x10 : 0);
			has_change_lsn = 1;
		}
	}
	if ( has_change_lsn )
	{
		VERBOSE_KPRINTF(1, "CDVD: sceCdLsnDualChg lsn %d: change lsn %d\n", lsn, change_lsn);
	}
	return has_change_lsn ? change_lsn : lsn;
}

int sceCdReadDvdDualInfo(int *on_dual, unsigned int *layer1_start)
{
	int read0_result;
	int i;
	sceCdRMode cdrmode;

	*on_dual = 0;
	*layer1_start = 0;
	g_cdvdman_istruct.m_dual_layer_emulation = 0;
#ifndef CDVD_VARIANT_XOSD
	if ( !cdvdman_isdvd() )
	{
		return 1;
	}
#endif
	if ( !g_cdvdman_emudvd9 )
	{
		if ( !DvdDual_infochk() )
		{
			return 0;
		}
		*on_dual = !!g_cdvdman_istruct.m_opo_or_para;
		*layer1_start = g_cdvdman_istruct.m_layer_1_lsn;
		return 1;
	}
#ifdef CDVD_VARIANT_OSD
	if ( !cdvdman_isdvd() )
	{
		return 1;
	}
#endif
#ifndef CDVD_VARIANT_XOSD
	if ( g_cdvdman_mmode != SCECdMmodeDvd && g_cdvdman_mmode != 0xFF )
	{
		return 0;
	}
#endif
	cdrmode.trycount = 0;
	cdrmode.spindlctrl = SCECdSpinStm;
	cdrmode.datapattern = SCECdSecS2048;
	read0_result = sceCdRead0(0xE, 1, g_cdvdman_ptoc, &cdrmode, 0, 0);
	sceCdSync(3);
	if ( g_cdvdman_istruct.m_last_error != SCECdErNO && !read0_result )
	{
		VERBOSE_KPRINTF(1, "CDVD: ReadDvdDualInfo Read Error %02x, %d\n", (u8)g_cdvdman_istruct.m_last_error, 0);
		return 0;
	}
	for ( i = 0; i < 20; i += 1 )
	{
		if ( g_cdvdman_ptoc[i + 104] != g_masterdisc_header[i] )
		{
			break;
		}
	}
	if ( i != 20 )
	{
		if ( !DvdDual_infochk() )
		{
			return 0;
		}
		*on_dual = !!g_cdvdman_istruct.m_opo_or_para;
		*layer1_start = g_cdvdman_istruct.m_layer_1_lsn;
		return 1;
	}
	if ( g_cdvdman_ptoc[131] != 2 || !(g_cdvdman_ptoc[132] & 2) )
	{
		return 1;
	}
	g_cdvdman_istruct.m_current_dvd = g_cdvdman_ptoc[133];
	g_cdvdman_istruct.m_current_dvd_lsn =
		g_cdvdman_ptoc[134] + (g_cdvdman_ptoc[135] << 8) + (g_cdvdman_ptoc[136] << 16) + (g_cdvdman_ptoc[137] << 24) + 1;
	g_cdvdman_istruct.m_opo_or_para = 0;
	g_cdvdman_istruct.m_layer_1_lsn = g_cdvdman_istruct.m_current_dvd_lsn;
	g_cdvdman_istruct.m_dual_layer_emulation = 1;
	*on_dual = 1;
	*layer1_start = g_cdvdman_istruct.m_layer_1_lsn;
	VERBOSE_KPRINTF(
		1,
		"sceCdReadDvdDualInfo():Cur_Disk %d layer1_start %d\n",
		(u8)g_cdvdman_istruct.m_current_dvd,
		g_cdvdman_istruct.m_current_dvd_lsn);
	return 1;
}

int sceCdSC(int code, int *param)
{
	void *poffarg_tmp;
	int *BootMode;
	int state;
	u32 efbits;
#ifdef CDVD_VARIANT_XOSD
	u8 regval;
#endif

	switch ( code )
	{
#ifdef CDVD_VARIANT_XOSD
		case 0xFFFFFFD6:
			CpuSuspendIntr(&state);
			set_cdvd_dev5_base_addr_atapi(2);
			regval = ((vu8 *)(0xBF402000))[*param];
			set_cdvd_dev5_base_addr_atapi(1);
			CpuResumeIntr(state);
			return regval;
		case 0xFFFFFFD7:
			CpuSuspendIntr(&state);
			*param = (int)&g_cdvdman_istruct.m_chgsys_writer_drive_shell_is_open;
			CpuResumeIntr(state);
			return 1;
		case 0xFFFFFFD8:
			return sc_ffffffd8((u32 *)param);
		case 0xFFFFFFD9:
			CpuSuspendIntr(&state);
			*param = (int)&g_cdvdman_istruct.m_atapi_disk_ejected;
			CpuResumeIntr(state);
			return 1;
		case 0xFFFFFFDA:
			return get_disk_type_ex();
		case 0xFFFFFFDB:
			CpuSuspendIntr(&state);
			*(&g_cdvdman_istruct.m_var_sc_ffffffdb) = (int)param;
			CpuResumeIntr(state);
			return 1;
		case 0xFFFFFFDC:
			return g_cdvdman_vernotxxx1x;
		case 0xFFFFFFDD:
			return get_cdvd_register(*param);
		case 0xFFFFFFDF:
			g_adma_evfid = *param;
			return 1;
		case 0xFFFFFFE0:
			g_acmd_evfid = *param;
			return 1;
		case 0xFFFFFFE1:
			CpuSuspendIntr(&state);
			g_cdvdman_istruct.m_chgsys_callback = (void *)(int)param;
			CpuResumeIntr(state);
			return 1;
		case 0xFFFFFFE4:
			g_cd_atapi_evfid = *param;
			return 1;
		case 0xFFFFFFE5:
			CpuSuspendIntr(&state);
			g_cdvdman_istruct.m_cd_atapi_intr_callback = (void *)(int)param;
			CpuResumeIntr(state);
			return 1;
#endif
		case 0xFFFFFFE6:
			CpuSuspendIntr(&state);
			*param = (int)g_cdvdman_power_off_callback;
			poffarg_tmp = g_cdvdman_power_off_callback_userdata;
			CpuResumeIntr(state);
			return (int)poffarg_tmp;
		case 0xFFFFFFE7:
			return g_scmd_evfid;
		case 0xFFFFFFE9:
			return sceCdLsnDualChg(*param);
		case 0xFFFFFFEA:
			return DvdDual_infochk();
		case 0xFFFFFFEE:
			g_cdvdman_istruct.m_last_read_timeout = *param;
			return 0;
		case 0xFFFFFFEF:
			return g_cdvdman_stream_timeout;
		case 0xFFFFFFF0:
			*param = (int)&g_verbose_level;
			return 0xFF;
		case 0xFFFFFFF1:
			return g_cdvdman_sync_timeout;
		case 0xFFFFFFF2:
			*param = (int)&g_cdvdman_istruct;
			return 0xFF;
		case 0xFFFFFFF3:
			BootMode = QueryBootMode(4);
			switch ( BootMode ? *(u8 *)BootMode : 0xFF )
			{
				case 0:
				case 1:
				case 2:
					g_cdvdman_mmode = 0xFF;
					break;
				case 3:
					g_cdvdman_mmode = SCECdMmodeCd;
					break;
				default:
					break;
			}
			return 1;
		case 0xFFFFFFF4:
#ifdef CDVD_VARIANT_OSD
			return 1;
#else
			// The following call to sceCdGetDiskType was inlined
			switch ( sceCdGetDiskType() )
			{
				case SCECdPSCD:
				case SCECdPSCDDA:
				case SCECdPS2CD:
				case SCECdPS2CDDA:
					return g_cdvdman_mmode == SCECdMmodeCd || g_cdvdman_mmode == 0xFF;
				case SCECdPS2DVD:
					return g_cdvdman_mmode == SCECdMmodeDvd || g_cdvdman_mmode == 0xFF;
				case SCECdCDDA:
					return g_cdvdman_mmode == 0xFF;
				default:
					return 0;
			}
#endif
		case 0xFFFFFFF5:
			return g_cdvdman_intr_evfid;
		case 0xFFFFFFF6:
			if ( *param )
			{
				WaitEventFlag(g_fio_fsv_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
			}
			else
			{
				SetEventFlag(g_fio_fsv_evfid, 1);
			}
			g_cdvdman_ee_rpc_fno = *param;
			VERBOSE_KPRINTF(1, "EE_ncmd_working code= %d\n", *param);
			return g_cdvdman_ee_rpc_fno;
		case 0xFFFFFFF7:
			return (u16)_irx_id.v;
		case 0xFFFFFFF8:
			g_cdvdman_spinctl = *param;
			return 1;
		case 0xFFFFFFFC:
			return g_cdvdman_cd36key;
		case 0xFFFFFFFD:
			return g_cdvdman_istruct.m_read2_flag;
		case 0xFFFFFFFE:
			g_cdvdman_istruct.m_last_error = *(u8 *)param;
			return (u8)g_cdvdman_istruct.m_last_error;
		case 0xFFFFFFFF:
		case 0:
		case 1:
		case 2:
			*param = (u8)g_cdvdman_istruct.m_last_error;
			if ( code != -1 )
			{
				g_cdvdman_istruct.m_stream_flag = code;
			}
			return g_cdvdman_istruct.m_stream_flag;
		default:
			VERBOSE_KPRINTF(1, "sceCdSC func_num Not found %d\n", code);
			return 0;
	}
}

static void cdvdman_init(void)
{
	int *BootMode;
	unsigned int i;
	int scres_unused;
	u32 argres;
#ifdef CDVD_VARIANT_XOSD
	int chgsys_tmp;
#endif
	USE_DEV5_MMIO_HWPORT();

#ifdef CDVD_VARIANT_XOSD
	{
		int state;
		CpuSuspendIntr(&state);
		g_cdvdman_istruct.m_cd_mode_ps2_atapi = 2;
		cdvdman_change_drive(2);
		CpuResumeIntr(state);
	}
#endif
	g_cdvdman_user_cb = 0;
	g_cdvdman_power_off_callback = 0;
	g_cdvdman_cmdfunc = 0;
	g_cdvdman_istruct.m_drive_interupt_request = 0;
	RegisterIntrHandler(IOP_IRQ_CDVD, 1, intrh_cdrom, &g_cdvdman_istruct);
	RegisterIntrHandler(IOP_IRQ_DMA_CDVD, 1, intrh_dma_3, &g_cdvdman_istruct);
	EnableIntr(IOP_IRQ_CDVD);
	sceCdSC(0xFFFFFFF3, &scres_unused);
	dmac_set_dpcr(dmac_get_dpcr() | 0x8000);
	dmac_ch_set_chcr(3, 0);
#ifdef CDVD_VARIANT_XOSD
	if ( (dev5_mmio_hwport->m_dev5_reg_008 & 0x10) )
	{
		dev5_mmio_hwport->m_dev5_reg_008 = 0x10;
	}
	if ( (dev5_mmio_hwport->m_dev5_reg_008 & 8) )
	{
		dev5_mmio_hwport->m_dev5_reg_008 = 8;
	}
#endif
	if ( (dev5_mmio_hwport->m_dev5_reg_008 & 4) )
	{
		dev5_mmio_hwport->m_dev5_reg_008 = 4;
	}
	if ( (dev5_mmio_hwport->m_dev5_reg_008 & 1) )
	{
		dev5_mmio_hwport->m_dev5_reg_008 = 1;
	}
#ifndef CDVD_VARIANT_XOSD
	g_cdvdman_clk_flg = sceCdReadClock(&g_cdvdman_clock) ? (!g_cdvdman_clock.stat) : 0;
#endif
	g_cdvdman_istruct.m_tray_is_open = !(dev5_mmio_hwport->m_dev5_reg_00A & SCECdStatShellOpen);
#ifdef CDVD_VARIANT_XOSD
	g_cdvdman_clk_flg = sceCdReadClock(&g_cdvdman_clock) ? (!g_cdvdman_clock.stat) : 0;
#endif
	cdvdman_initcfg();
	BootMode = QueryBootMode(6);
	g_cdvdman_istruct.m_no_dec_flag = BootMode ? ((*(u16 *)BootMode & 0xFFFC) == 0x60) : 0;
	for ( i = 0; i <= 60 && (!sceCdCancelPOffRdy(&argres) || argres); i += 1 )
	{
		DelayThread(16000);
	}
#ifdef CDVD_VARIANT_XOSD
	{
		u32 medium_res;
		int MediumRemoval;

		for ( i = 0; i <= 60 && (!sceCdSetMediumRemoval(0, &argres) || argres); i += 1 )
		{
			DelayThread(16000);
		}
		for ( i = 0; i <= 60 && (!(MediumRemoval = sceCdGetMediumRemoval(&medium_res, &argres)) || argres); i += 1 )
		{
			DelayThread(16000);
		}
		g_cdvdman_istruct.m_medium_removal_state = (MediumRemoval && !argres) ? medium_res : 0;
		{
			int state;

			CpuSuspendIntr(&state);
			chgsys_tmp = (dev5_mmio_hwport->m_dev5_reg_00A & SCECdStatShellOpen);
			g_cdvdman_istruct.m_cd_mode_ps2_atapi =
				(chgsys_tmp || (!(dev5_mmio_hwport->m_dev5_reg_015 & 0x80) && dev5_mmio_hwport->m_dev5_reg_00F != 6)) ? 2 : 1;
			cdvdman_change_drive(g_cdvdman_istruct.m_cd_mode_ps2_atapi);
			CpuResumeIntr(state);
		}
		if ( chgsys_tmp )
		{
			sceCdChgSys(1);
		}
	}
#endif
}

int sceCdInit(int mode)
{
	VERBOSE_PRINTF(1, "sceCdInit called mode= %d\n", mode);
	if ( mode == SCECdEXIT )
	{
		int oldstate;

		g_cdvdman_istruct.m_cd_inited = 0;
#ifndef CDVD_VARIANT_XOSD
		sceCdBreak();
		sceCdSync(3);
		cdvdman_ncmd_sender_06();
#endif
		sceCdSync(0);
		VERBOSE_PRINTF(1, "Cdvdman Exit\n");
		DisableIntr(IOP_IRQ_CDVD, &oldstate);
		ReleaseIntrHandler(IOP_IRQ_CDVD);
		DisableIntr(IOP_IRQ_DMA_CDVD, &oldstate);
		ReleaseIntrHandler(IOP_IRQ_DMA_CDVD);
	}
	else
	{
		VERBOSE_PRINTF(1, "Cdvdman Init\n");
		g_cdvdman_istruct.m_read2_flag = 0;
		g_cdvdman_istruct.m_dec_shift = 0;
		g_cdvdman_istruct.m_check_version = 0;
		g_cdvdman_istruct.m_dec_state = 0;
#ifdef CDVD_VARIANT_XOSD
		if ( update_cd_mode_ps2_atapi() != 1 )
#endif
		{
			sceCdDecSet(0, 0, 0);
		}
		cdvdman_init();
		g_cdvdman_istruct.m_cd_inited = 1;
	}
	if ( mode == SCECdINIT )
	{
		u8 ready_status_tmp;
		u8 ready_status;

		ready_status_tmp = 0;
		VERBOSE_PRINTF(1, "sceCdInit Ready check start.\n");
		ready_status = 0;
		while ( (ready_status & 0xC0) != 0x40 )
		{
#ifdef CDVD_VARIANT_XOSD
			if ( update_cd_mode_ps2_atapi() == 1 || sceCdGetDiskType() == 6 )
				break;
			g_cdvdman_istruct.m_field_0DC = 0;
			if ( sceCdGetDiskType() == SCECdNODISC )
				break;
#endif
			ready_status = sceCdDiskReady(8);
			vDelayThread(10000);
			if ( ready_status != ready_status_tmp )
			{
				ready_status_tmp = ready_status;
				VERBOSE_PRINTF(1, "sceCdInit Dev5 Status %x\n", ready_status);
			}
			// The following Kprintf was added for ioprp300x
			VERBOSE_PRINTF(1, "sceCdInit Ready check %x\n", ready_status);
		}
		VERBOSE_PRINTF(1, "sceCdInit Ready check end.\n");
	}
	g_cdvdman_istruct.m_wait_flag = 1;
	g_cdvdman_istruct.m_scmd_flag = 1;
	g_cdvdman_istruct.m_last_error = SCECdErNO;
	g_cdvdman_istruct.m_last_read_timeout = 0;
	g_cdvdman_spinctl = -1;
	SetEventFlag(g_cdvdman_intr_evfid, 0x29);
	SetEventFlag(g_ncmd_evfid, 1);
	SetEventFlag(g_scmd_evfid, 1);
	SetEventFlag(g_sfile_evfid, 1);
	return 1;
}

static int set_prev_command(int cmd, const char *sdata, int sdlen, char *rdata, int rdlen, int check_sef)
{
	int i;
	int delaybackoff;
	int j;
	u32 efbits;
#ifdef CDVD_VARIANT_XOSD
	int suspendres;
#endif

#ifdef CDVD_VARIANT_XOSD
	{
		int state;

		suspendres = CpuSuspendIntr(&state);
		CpuResumeIntr(state);
		suspendres = QueryIntrContext() || suspendres;
	}
	VERBOSE_KPRINTF(1, "set_prev_command cmd %02x call\n", cmd);
	if ( !suspendres )
	{
		WaitEventFlag(g_cdvdman_csys_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	}
#endif
	if ( check_sef == 1 && vPollEventFlag(g_scmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND )
	{
		// The following Kprintf was added for ioprp300x
		VERBOSE_KPRINTF(1, "set_prev_command Double Booking\n");
#ifdef CDVD_VARIANT_XOSD
		if ( !suspendres )
		{
			SetEventFlag(g_cdvdman_csys_evfid, 1);
		}
#endif
		return 0;
	}
	g_cdvdman_istruct.m_scmd = cmd;
	g_cdvdman_istruct.m_sdlen = sdlen;
	g_cdvdman_istruct.m_rdlen = rdlen;
	for ( i = 0; i < sdlen; i += 1 )
	{
		g_cdvdman_istruct.m_scmd_sd[i] = sdata[i];
	}
#ifdef CDVD_VARIANT_XOSD
	if ( update_cd_mode_ps2_atapi() != 1 )
#endif
	{
		if ( g_cdvdman_istruct.m_wait_flag )
		{
			g_cdvdman_istruct.m_scmd_flag = 1;
			cdvdman_write_scmd(&g_cdvdman_istruct);
		}
		else if ( QueryIntrContext() )
		{
			while ( (dmac_ch_get_chcr(3) & 0x1000000) && !g_cdvdman_istruct.m_wait_flag )
			{
				VERBOSE_KPRINTF(1, "set_prev_command: DMA Wait\n");
			}
			g_cdvdman_istruct.m_scmd_flag = 1;
			cdvdman_write_scmd(&g_cdvdman_istruct);
		}
		else
		{
			g_cdvdman_istruct.m_scmd_flag = 0;
		}
	}
#ifdef CDVD_VARIANT_XOSD
	else
	{
		if ( !suspendres )
		{
			if ( g_cd_atapi_evfid != -1 )
			{
				WaitEventFlag(g_cd_atapi_evfid, 3, WEF_AND | WEF_CLEAR, &efbits);
			}
			if ( g_adma_evfid != -1 )
			{
				WaitEventFlag(g_adma_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
			}
			if ( g_acmd_evfid != -1 )
			{
				WaitEventFlag(g_acmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
			}
		}
		{
			int state;

			CpuSuspendIntr(&state);
			set_cdvd_dev5_base_addr_atapi(2);
			g_cdvdman_istruct.m_scmd_flag = 1;
			VERBOSE_KPRINTF(1, "do_set_prev_command %02x\n", cmd);
			cdvdman_write_scmd(&g_cdvdman_istruct);
			set_cdvd_dev5_base_addr_atapi(1);
			CpuResumeIntr(state);
		}
		if ( !suspendres )
		{
			if ( g_acmd_evfid != -1 )
			{
				SetEventFlag(g_acmd_evfid, 1);
			}
			if ( g_adma_evfid != -1 )
			{
				SetEventFlag(g_adma_evfid, 1);
			}
			if ( g_cd_atapi_evfid != -1 )
			{
				SetEventFlag(g_cd_atapi_evfid, 3);
			}
		}
	}
#endif
	delaybackoff = 1;
	for ( i = 0; i < 500; i += delaybackoff )
	{
		if ( g_cdvdman_istruct.m_scmd_flag )
		{
			for ( j = 0; j < rdlen; j += 1 )
			{
				rdata[j] = g_cdvdman_istruct.m_scmd_rd[j];
			}
			if ( check_sef == 1 )
			{
				vSetEventFlag(g_scmd_evfid, 1);
			}
#ifdef CDVD_VARIANT_XOSD
			VERBOSE_KPRINTF(1, "set_prev_command end :%02x\n", (u8)g_cdvdman_istruct.m_scmd);
			if ( !suspendres )
			{
				SetEventFlag(g_cdvdman_csys_evfid, 1);
			}
#endif
			return (u8)g_cdvdman_istruct.m_scmd;
		}
		vDelayThread(1000 * delaybackoff);
		if ( (i & 1) && delaybackoff < 16 )
		{
			delaybackoff *= 2;
		}
	}
	g_cdvdman_istruct.m_scmd_flag = 1;
	if ( check_sef == 1 )
	{
		vSetEventFlag(g_scmd_evfid, 1);
	}
	return 0;
}

static void cdvdman_write_scmd(cdvdman_internal_struct_t *s)
{
	int i;
	unsigned int j;
	unsigned int rdcnt;
	char rdptr1[64];
	USE_DEV5_MMIO_HWPORT();

	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(1, "in:%d out_size:%d \n", (u8)s->m_sdlen, (u8)s->m_rdlen);
	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(1, "cmd:%02x pram:%02x\n", (u8)s->m_scmd, (u8)s->m_scmd_sd[0]);
	for ( i = 0; i <= 0; i += 1 )
	{
		int overflowcond;
#ifdef CDVD_VARIANT_XOSD
		int suspendres;
#endif

		if ( (dev5_mmio_hwport->m_dev5_reg_017 & 0x80) )
		{
			s->m_scmd_flag = 1;
			s->m_scmd = 0;
			// The following Kprintf was added for ioprp300x
			VERBOSE_KPRINTF(1, "do_set_prev_command BUSY ABNORM END %08x\n", dev5_mmio_hwport->m_dev5_reg_017);
			return;
		}
		while ( !(dev5_mmio_hwport->m_dev5_reg_017 & 0x40) )
		{
			;
		}
		for ( j = 0; j < (u8)s->m_sdlen; j += 1 )
		{
			dev5_mmio_hwport->m_dev5_reg_017 = s->m_scmd_sd[j];
		}
		dev5_mmio_hwport->m_dev5_reg_016 = s->m_scmd;
#ifdef CDVD_VARIANT_XOSD
		{
			int state;
			suspendres = CpuSuspendIntr(&state);
			CpuResumeIntr(state);
			suspendres = QueryIntrContext() || suspendres;
		}
#endif
#ifdef CDVD_VARIANT_XOSD
		if ( suspendres )
#else
		if ( QueryIntrContext() )
#endif
		{
			for ( j = 0; dev5_mmio_hwport->m_dev5_reg_017 & 0x80; j += 1 )
			{
				if ( j > 12500000 )
				{
					s->m_scmd_flag = 1;
					s->m_scmd = 0;
					return;
				}
			}
		}
		else
		{
			for ( j = 0; dev5_mmio_hwport->m_dev5_reg_017 & 0x80; j += 1 )
			{
				DelayThread(100);
				if ( j > 50000 )
				{
					// The following Kprintf was added for ioprp300x
					VERBOSE_KPRINTF(1, "Mecacon Scmd TIMEOUT CMD= %02x\n", (u8)s->m_scmd);
					s->m_scmd_flag = 1;
					s->m_scmd = 0;
					return;
				}
			}
		}
		overflowcond = 0;
		for ( j = 0; j < (u8)s->m_rdlen; j += 1 )
		{
			if ( (dev5_mmio_hwport->m_dev5_reg_017 & 0x40) )
			{
				break;
			}
			rdptr1[j] = dev5_mmio_hwport->m_dev5_reg_018;
		}
		if ( j >= (u8)s->m_rdlen )
		{
			overflowcond = 1;
			VERBOSE_KPRINTF(1, "Prev Cmd Result Over Flow\n", rdptr1);
		}
		rdcnt = j;
		if ( (!overflowcond && j >= (u8)s->m_rdlen) || s->m_rdlen == 16 )
		{
			break;
		}
		VERBOSE_KPRINTF(1, "Prev Cmd Result Illegal Size Try count:%d\n", i);
	}
	if ( i == 1 )
	{
		s->m_scmd_flag = 1;
		s->m_scmd = 0;
	}
	else
	{
		for ( j = 0; j < (sizeof(s->m_scmd_rd) / sizeof(s->m_scmd_rd[0])); j += 1 )
		{
			s->m_scmd_rd[j] = 0;
		}
		if ( s->m_rdlen != (sizeof(s->m_scmd_rd) / sizeof(s->m_scmd_rd[0])) )
		{
			rdcnt = (u8)s->m_rdlen;
		}
		for ( j = 0; j < rdcnt; j += 1 )
		{
			s->m_scmd_rd[j] = rdptr1[j];
		}
		s->m_scmd_flag = 1;
		s->m_scmd = 1;
	}
}

#ifdef CDVD_VARIANT_XOSD
static int cdvdman_write_scmd_swap_dev5(char cmd, const char *wdata, int sdlen, char *rdata, int rdlen, int check_sef)
{
	int atapi_check;
	int i;
	int j;
	char rdstart[64];
	u32 efbits;
	int state;
	USE_DEV5_MMIO_HWPORT();

	if ( check_sef == 1 && vPollEventFlag(g_scmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND )
	{
		return 0;
	}
	CpuSuspendIntr(&state);
	atapi_check = update_cd_mode_ps2_atapi() == 1;
	if ( atapi_check )
	{
		set_cdvd_dev5_base_addr_atapi(2);
	}
	for ( i = 0; i <= 0; i += 1 )
	{
		int cmdresoverflow;

		if ( (dev5_mmio_hwport->m_dev5_reg_017 & 0x80) )
		{
			if ( atapi_check )
			{
				set_cdvd_dev5_base_addr_atapi(1);
			}
			CpuResumeIntr(state);
			if ( check_sef == 1 )
			{
				vSetEventFlag(g_scmd_evfid, 1);
			}
			return 0;
		}
		while ( !(dev5_mmio_hwport->m_dev5_reg_017 & 0x40) )
		{
			;
		}
		for ( j = 0; j < sdlen; j += 1 )
		{
			dev5_mmio_hwport->m_dev5_reg_017 = wdata[j];
		}
		dev5_mmio_hwport->m_dev5_reg_016 = cmd;
		while ( (dev5_mmio_hwport->m_dev5_reg_017 & 0x80) )
		{
			;
		}
		cmdresoverflow = 0;
		for ( j = 0; !(dev5_mmio_hwport->m_dev5_reg_017 & 0x40); j += 1 )
		{
			if ( j >= rdlen )
			{
				cmdresoverflow = 1;
				VERBOSE_KPRINTF(1, "Prev Cmd2 Result Over Flow\n");
				break;
			}
			rdstart[j] = dev5_mmio_hwport->m_dev5_reg_018;
		}
		if ( (!cmdresoverflow && j >= rdlen) || rdlen == 16 )
		{
			break;
		}
		VERBOSE_KPRINTF(1, "Prev Cmd2 Result Illegal Size Try count:%d\n", i);
		while ( !(dev5_mmio_hwport->m_dev5_reg_017 & 0x20) )
		{
			;
		}
		for ( j = 0; j < 16 - rdlen; j += 1 )
		{
		}
	}
	if ( i == 1 )
	{
		if ( atapi_check )
		{
			set_cdvd_dev5_base_addr_atapi(1);
		}
		CpuResumeIntr(state);
		if ( check_sef == 1 )
		{
			vSetEventFlag(g_scmd_evfid, 1);
		}
		return 0;
	}
	for ( j = 0; j < rdlen; j += 1 )
	{
		rdata[j] = rdstart[j];
	}
	if ( atapi_check )
	{
		set_cdvd_dev5_base_addr_atapi(1);
	}
	CpuResumeIntr(state);
	if ( check_sef == 1 )
	{
		vSetEventFlag(g_scmd_evfid, 1);
	}
	return 1;
}
#endif

static int cdvdman_send_scmd2(int cmd, const void *sdata, int sdlen, void *rdata, int rdlen, int check_sef)
{
	int i;
	int j;
	char rdstart[64];
	u32 efbits;
	USE_DEV5_MMIO_HWPORT();

	if ( check_sef == 1 && vPollEventFlag(g_scmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND )
	{
		return 0;
	}
	for ( i = 0; i <= 0; i += 1 )
	{
		int cmdresoverflow;

		if ( (dev5_mmio_hwport->m_dev5_reg_017 & 0x80) )
		{
			if ( check_sef == 1 )
			{
				vSetEventFlag(g_scmd_evfid, 1);
			}
			return 0;
		}
		while ( !(dev5_mmio_hwport->m_dev5_reg_017 & 0x40) )
		{
			;
		}
		for ( j = 0; j < sdlen; j += 1 )
		{
			dev5_mmio_hwport->m_dev5_reg_017 = ((u8 *)sdata)[j];
		}
		dev5_mmio_hwport->m_dev5_reg_016 = cmd;
		while ( (dev5_mmio_hwport->m_dev5_reg_017 & 0x80) )
		{
#ifndef CDVD_VARIANT_XOSD
			DelayThread(100);
#endif
		}
		cmdresoverflow = 0;
		for ( j = 0; !(dev5_mmio_hwport->m_dev5_reg_017 & 0x40); j += 1 )
		{
			if ( j >= rdlen )
			{
				cmdresoverflow = 1;
				// The following Kprintf was modified for ioprp300x
				VERBOSE_KPRINTF(1, "Prev Cmd2 Result Over Flow\n");
				break;
			}
			rdstart[j] = dev5_mmio_hwport->m_dev5_reg_018;
		}
		if ( (!cmdresoverflow && j >= rdlen) || rdlen == 16 )
		{
			break;
		}
		// The following Kprintf was modified for ioprp300x
		VERBOSE_KPRINTF(1, "Prev Cmd2 Result Illegal Size Try count:%d\n", i);
		while ( !(dev5_mmio_hwport->m_dev5_reg_017 & 0x20) )
		{
			;
		}
		for ( j = 0; j < 16 - rdlen; j += 1 )
			;
	}
	if ( i == 1 )
	{
		if ( check_sef == 1 )
		{
			vSetEventFlag(g_scmd_evfid, 1);
		}
		return 0;
	}
	for ( i = 0; i < rdlen; i += 1 )
	{
		((char *)rdata)[i] = rdstart[i];
	}
	if ( check_sef == 1 )
	{
		vSetEventFlag(g_scmd_evfid, 1);
	}
	return 1;
}

#ifdef CDVD_VARIANT_OSD
static unsigned int signal_sema_timeout_callback(void *userdata)
{
	int sema_id;

	sema_id = (int)(u32)(uiptr)userdata;
	iSignalSema(sema_id);
	return 0;
}
#endif

int sceCdApplySCmd(u8 cmdNum, const void *inBuff, u16 inBuffSize, void *outBuff)
{
	int i;

#ifdef CDVD_VARIANT_OSD
	if ( g_cdvdman_minver_50000 )
	{
		WaitSema(g_cdvdman_apply_scmd_sema);
	}
#endif
	for ( i = 0; i <= 2500; i += 1 )
	{
		if ( set_prev_command(cmdNum, (const char *)inBuff, inBuffSize, (char *)outBuff, 16, 1) )
		{
#ifdef CDVD_VARIANT_OSD
			if ( g_cdvdman_minver_50000 )
			{
				iop_sys_clock_t sysclk;

				sysclk.hi = 0;
				sysclk.lo = 0x9000;
				SetAlarm(&sysclk, signal_sema_timeout_callback, (void *)g_cdvdman_apply_scmd_sema);
			}
			else
#endif
			{
				DelayThread(2000);
			}
			return 1;
		}
		DelayThread(2000);
	}
	KPRINTF("CDVD: set_prev_command TIMEOUT 5(SEC)\n");
	return 0;
}

int sceCdApplySCmd2(u8 cmdNum, const void *inBuff, unsigned long int inBuffSize, void *outBuff)
{
#ifdef CDVD_VARIANT_XOSD
	return sceCdApplySCmd(cmdNum, inBuff, inBuffSize, outBuff);
#else
	int i;

	for ( i = 0; i <= 2500; i += 1 )
	{
		if ( cdvdman_send_scmd2(cmdNum, inBuff, inBuffSize, outBuff, 16, 1) )
		{
			return 1;
		}
		DelayThread(2000);
	}
	KPRINTF("CDVD: set_prev_command TIMEOUT 5(SEC)\n");
	return 0;
#endif
}

#ifdef CDVD_VARIANT_OSD
int sceCdApplySCmd3(u8 cmdNum, const void *inBuff, unsigned long int inBuffSize, void *outBuff)
{
	int i;

	for ( i = 0; i <= 2500; i += 1 )
	{
		DelayThread(2000);
		if ( set_prev_command((u8)cmdNum, inBuff, inBuffSize, outBuff, 16, 1) )
		{
			DelayThread(2000);
			return 1;
		}
	}
	KPRINTF("CDVD: set_prev_command TIMEOUT 5(SEC)\n");
	return 0;
}
#endif

int sceCdBreak(void)
{
	u32 efbits;
	int state;
	int oldstate;
	USE_DEV5_MMIO_HWPORT();

	if ( PollEventFlag(g_ncmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND )
	{
		return 0;
	}
	CpuSuspendIntr(&state);
	// The following Kprintf was modified for ioprp300x
	VERBOSE_KPRINTF(
		1,
		"Break call: read2_flg= %d func= %d lsn= %d csec= %d nsec= %d %rsec= %d addr= %p waitflg= %d, DriveIntrReq= %d "
		"stmflg= %d recover= %d\n",
		g_cdvdman_istruct.m_read2_flag,
		g_cdvdman_cmdfunc,
		g_cdvdman_istruct.m_cdvdman_lsn,
		g_cdvdman_istruct.m_cdvdman_csec,
		g_cdvdman_istruct.m_cdvdman_nsec,
		g_cdvdman_istruct.m_cdvdman_rsec,
		g_cdvdman_istruct.m_read_buf,
		g_cdvdman_istruct.m_wait_flag,
		g_cdvdman_istruct.m_drive_interupt_request,
		g_cdvdman_istruct.m_stream_flag,
		g_cdvdman_istruct.m_recover_status);
	if ( g_cdvdman_istruct.m_last_read_timeout )
	{
		g_cdvdman_istruct.m_read2_flag = 0;
	}
	g_cdvdman_istruct.m_last_error = SCECdErABRT;
	g_cdvdman_istruct.m_thread_id = GetThreadId();
	g_cdvdman_istruct.m_break_cdvdfsv_readchain = 1;
	if ( g_cdvdman_istruct.m_dec_state )
	{
		g_cdvdman_istruct.m_dec_shift = 0;
		g_cdvdman_istruct.m_check_version = 0;
		g_cdvdman_istruct.m_dec_state = 0;
		sceCdDecSet(0, 0, 0);
	}
	g_cdvdman_istruct.m_recover_status = 0;
	if ( QueryIntrContext() )
	{
		iSetEventFlag(g_cdvdman_intr_evfid, 0x29);
		iCancelAlarm(read_timeout_alarm_cb, &g_cdvdman_read_alarm_cb_timeout);
	}
	else
	{
		SetEventFlag(g_cdvdman_intr_evfid, 0x29);
		CancelAlarm(read_timeout_alarm_cb, &g_cdvdman_read_alarm_cb_timeout);
	}
	if ( !g_cdvdman_istruct.m_wait_flag || g_cdvdman_istruct.m_last_read_timeout )
	{
		// The following direct register access was replaced with call for ioprp300x
		if ( (sceCdDiskReady(8) & 0xC0) == 0x40 )
		{
			VERBOSE_KPRINTF(1, "cdvd: NonInter END\n");
			g_cdvdman_istruct.m_wait_flag = 1;
		}
		g_cdvdman_last_cmdfunc = g_cdvdman_cmdfunc;
		g_cdvdman_cmdfunc = SCECdFuncBreak;
		dev5_mmio_hwport->m_dev5_reg_007 = 1;
		if ( g_cdvdman_istruct.m_last_read_timeout )
		{
			DisableIntr(IOP_IRQ_DMA_CDVD, &oldstate);
		}
		g_cdvdman_istruct.m_drive_interupt_request = 0;
		VERBOSE_KPRINTF(1, "cdvd: Abort command On\n");
	}
	vSetEventFlag(g_ncmd_evfid, 1);
	CpuResumeIntr(state);
	return 1;
}

static unsigned int ncmd_timeout_alarm_cb(void *userdata)
{
	iop_sys_clock_t *sys_clock;

	sys_clock = (iop_sys_clock_t *)userdata;
	KPRINTF("Cmd Time Out %d(msec)\n", sys_clock->lo / 0x9000);
	sys_clock->lo = 0;
	return 0;
}

static int intrh_dma_3(void *userdata)
{
	int dmacbres;
	int oldstate;
	cdvdman_internal_struct_t *s;

	s = (cdvdman_internal_struct_t *)userdata;
	s->m_dma3_param.m_dma3_msectors -= s->m_dma3_param.m_dma3_csectors;
	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(
		1,
		"_sceCdDmaIntr wk %d: func_num: %d nsec %d cnt %d cdid:%08x\n",
		s->m_wait_flag,
		g_cdvdman_cmdfunc,
		s->m_dma3_param.m_dma3_msectors,
		s->m_cdvdman_dma3sec,
		&g_cdvdman_istruct);
	dmacbres = s->m_dma3_param.m_dma3_callback ? s->m_dma3_param.m_dma3_callback() : 1;
	s->m_cdvdman_dma3sec += s->m_dma3_param.m_dma3_csectors;
	s->m_dma3_param.m_dma3_csectors = ((u32)s->m_read_chunk > (u32)s->m_dma3_param.m_dma3_msectors) ?
																			(u32)s->m_dma3_param.m_dma3_msectors :
																			(u32)s->m_read_chunk;
	if ( dmacbres )
	{
		if ( s->m_dma3_param.m_dma3_msectors )
		{
			dmac_ch_set_chcr(3, 0);
			dmac_ch_get_chcr(3);
			dmac_ch_set_madr(3, (u32)s->m_dma3_param.m_dma3_maddress);
			dmac_ch_set_bcr(
				3,
				((s->m_dma3_param.m_dma3_blkcount * s->m_dma3_param.m_dma3_csectors) << 16) | s->m_dma3_param.m_dma3_blkwords);
			dmac_ch_set_chcr(3, 0x41000200);
			dmac_ch_get_chcr(3);
			iClearEventFlag(g_cdvdman_intr_evfid, ~0x20);
		}
		else
		{
			DisableIntr(IOP_IRQ_DMA_CDVD, &oldstate);
			iSetEventFlag(g_cdvdman_intr_evfid, 0x20);
		}
	}
	if ( !s->m_dma3_param.m_dma3_msectors && s->m_drive_interupt_request )
	{
		cdvdman_intr_cb(s);
		s->m_drive_interupt_request = 0;
	}
	return 1;
}

static int cdvdman_setdma3(cdvdman_dma3_parameter_t *dma3_param)
{
	USE_DEV5_MMIO_HWPORT();

	if ( (dmac_ch_get_chcr(3) & 0x1000000) )
	{
		dev5_mmio_hwport->m_dev5_reg_007 = 1;
	}
	g_cdvdman_istruct.m_drive_interupt_request = 0;
	g_cdvdman_istruct.m_dma3_param.m_dma3_blkwords = dma3_param->m_dma3_blkwords;
	g_cdvdman_istruct.m_dma3_param.m_dma3_blkcount = dma3_param->m_dma3_blkcount;
	g_cdvdman_istruct.m_dma3_param.m_dma3_maddress = dma3_param->m_dma3_maddress;
	g_cdvdman_istruct.m_dma3_param.m_dma3_callback = dma3_param->m_dma3_callback;
	g_cdvdman_istruct.m_dma3_param.m_dma3_csectors = dma3_param->m_dma3_csectors;
	g_cdvdman_istruct.m_dma3_param.m_cdvdreg_howto = dma3_param->m_cdvdreg_howto;
	g_cdvdman_istruct.m_dma3_param.m_dma3_msectors = dma3_param->m_dma3_msectors;
	g_cdvdman_istruct.m_cdvdman_dma3sec = 0;
	dmac_ch_set_chcr(3, 0);
	dmac_ch_get_chcr(3);
	if ( dma3_param->m_dma3_csectors )
	{
		vClearEventFlag(g_cdvdman_intr_evfid, ~0x20);
		EnableIntr(IOP_IRQ_DMA_CDVD);
	}
	dev5_mmio_hwport->m_dev5_reg_006 = dma3_param->m_cdvdreg_howto;
	dmac_ch_set_madr(3, (u32)dma3_param->m_dma3_maddress);
	dmac_ch_set_bcr(
		3,
		(dma3_param->m_dma3_blkcount * (dma3_param->m_dma3_csectors ? dma3_param->m_dma3_csectors : 1)) << 16
			| dma3_param->m_dma3_blkwords);
	dmac_ch_set_chcr(3, 0x41000200);
	return dmac_ch_get_chcr(3);
}

#ifdef CDVD_VARIANT_XOSD
static int update_cd_mode_ps2_atapi(void)
{
	int is_intr_context;
	int saved_cd_mode_ps2_atapi;
	int state;

	is_intr_context = !QueryIntrContext();
	if ( is_intr_context )
	{
		CpuSuspendIntr(&state);
	}
	saved_cd_mode_ps2_atapi = g_cdvdman_istruct.m_cd_mode_ps2_atapi;
	if ( saved_cd_mode_ps2_atapi == 1 )
	{
		if ( !(*g_cdvdreg_bf801460 & 2) || (unsigned int)GetBaseAddress(5) != (unsigned int)0xBF410000 )
		{
			set_cdvd_dev5_base_addr_atapi(1);
		}
	}
	else
	{
		if ( (*g_cdvdreg_bf801460 & 2) || (unsigned int)GetBaseAddress(5) != (unsigned int)0x1F402000 )
		{
			set_cdvd_dev5_base_addr_atapi(2);
		}
	}
	if ( is_intr_context )
	{
		CpuResumeIntr(state);
	}
	return saved_cd_mode_ps2_atapi;
}
#endif

#ifdef CDVD_VARIANT_XOSD
static int set_cdvd_dev5_base_addr_atapi(int mode)
{
	int is_intr_context;
	int state;

	is_intr_context = !QueryIntrContext();
	if ( is_intr_context )
	{
		CpuSuspendIntr(&state);
	}
	if ( mode == 1 )
	{
		SetBaseAddress(5, 0xBF410000);
		VERBOSE_KPRINTF(1, "DEV5-addr : 0x%08lx\n", GetBaseAddress(5));
		SetDelay(5, 0xEF101043);
		VERBOSE_KPRINTF(1, "DEV5-dely : 0x%08lx\n", GetDelay(5));
		*g_cdvdreg_bf801464 |= 0xC;
		*g_cdvdreg_bf801460 |= 0x40;
		*g_cdvdreg_bf801460 |= 2;
	}
	else
	{
		SetBaseAddress(5, 0x1F402000);
		VERBOSE_KPRINTF(1, "DEV5-addr : 0x%08lx\n", GetBaseAddress(5));
		SetDelay(5, 0x6F060011);
		VERBOSE_KPRINTF(1, "DEV5-dely : 0x%08lx\n", GetDelay(5));
		*g_cdvdreg_bf801460 &= ~2;
		*g_cdvdreg_bf801460 &= ~0x40;
		*g_cdvdreg_bf801464 &= 0xF3;
	}
	if ( is_intr_context )
	{
		CpuResumeIntr(state);
	}
	return 1;
}
#endif

#ifdef CDVD_VARIANT_XOSD
static int cdvdman_change_drive(int mode)
{
	ClearEventFlag(g_cdvdman_csys_evfid, ~1);
	if ( mode == 1 )
	{
		VERBOSE_KPRINTF(1, "Init Go SCECdChgWriDrv\n");
		ClearEventFlag(g_fio_fsv_evfid, ~1);
		ClearEventFlag(g_sfile_evfid, ~1);
		ClearEventFlag(g_scmd_evfid, ~1);
		ClearEventFlag(g_ncmd_evfid, ~1);
		set_cdvd_dev5_base_addr_atapi(1);
		if ( g_cd_atapi_evfid != -1 )
		{
			SetEventFlag(g_cd_atapi_evfid, 3);
		}
		if ( g_adma_evfid != -1 )
		{
			SetEventFlag(g_adma_evfid, 1);
		}
		if ( g_acmd_evfid != -1 )
		{
			SetEventFlag(g_acmd_evfid, 1);
		}
		SetEventFlag(g_scmd_evfid, 1);
		VERBOSE_KPRINTF(1, "Init Go SCECdChgWriDrv End.\n");
		SetEventFlag(g_fio_fsv_evfid, 1);
	}
	else
	{
		VERBOSE_KPRINTF(1, "Init Go SCECdChgPs2Drv\n");
		ClearEventFlag(g_scmd_evfid, ~1);
		if ( g_acmd_evfid != -1 )
		{
			ClearEventFlag(g_acmd_evfid, ~1);
		}
		if ( g_adma_evfid != -1 )
		{
			ClearEventFlag(g_adma_evfid, ~1);
		}
		if ( g_cd_atapi_evfid != -1 )
		{
			ClearEventFlag(g_cd_atapi_evfid, ~3);
		}
		set_cdvd_dev5_base_addr_atapi(mode);
		SetEventFlag(g_ncmd_evfid, 1);
		SetEventFlag(g_scmd_evfid, 1);
		SetEventFlag(g_sfile_evfid, 1);
		SetEventFlag(g_fio_fsv_evfid, 1);
		VERBOSE_KPRINTF(1, "Init Go SCECdChgPs2Drv End.\n");
	}
	SetEventFlag(g_cdvdman_csys_evfid, 1);
	return 1;
}
#endif

#ifdef CDVD_VARIANT_XOSD
// cppcheck-suppress funcArgNamesDifferent
int sceCdChgSys(u32 use_writer_drive)
{
	int current_mode;
	u32 efbits;
	USE_DEV5_MMIO_HWPORT();

	VERBOSE_KPRINTF(1, "sceCdChgSys(%d) start.\n", use_writer_drive);
	if ( QueryIntrContext() )
	{
		return 0;
	}
	WaitEventFlag(g_cdvdman_csys_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
	current_mode = update_cd_mode_ps2_atapi();
	VERBOSE_KPRINTF(
		1, "sceCdChgSys %d call mode:%d->%d\n", use_writer_drive, current_mode, g_cdvdman_istruct.m_cd_mode_ps2_atapi);
	if ( !use_writer_drive )
	{
		g_cdvdman_istruct.m_cd_mode_ps2_atapi = current_mode;
	}
	if (
		use_writer_drive
		&& (use_writer_drive != (u32)current_mode || g_cdvdman_istruct.m_cd_mode_ps2_atapi != current_mode) )
	{
		int disc_type;

		if ( use_writer_drive == 1 )
		{
			VERBOSE_KPRINTF(1, "Go SCECdChgWriDrv\n");
			VERBOSE_KPRINTF(1, "fio_fsv_evfid\n");
			WaitEventFlag(g_fio_fsv_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
			VERBOSE_KPRINTF(1, "sfile_evfid\n");
			WaitEventFlag(g_sfile_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
			VERBOSE_KPRINTF(1, "sceCdSync\n");
			sceCdSync(0);
			VERBOSE_KPRINTF(1, "scmd_evfid\n");
			WaitEventFlag(g_scmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
			VERBOSE_KPRINTF(1, "ncmd_evfid\n");
			WaitEventFlag(g_ncmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
			set_cdvd_dev5_base_addr_atapi(2);
			disc_type = dev5_mmio_hwport->m_dev5_reg_00F;
			if ( g_cdvdman_vernotxxx1x )
			{
				if ( (dev5_mmio_hwport->m_dev5_reg_00A & SCECdStatShellOpen) )
				{
					g_cdvdman_istruct.m_chgsys_writer_drive_shell_is_open = 1;
				}
				if (
					(!(dev5_mmio_hwport->m_dev5_reg_015 & 0x80) && !(dev5_mmio_hwport->m_dev5_reg_00A & SCECdStatShellOpen))
					|| (dev5_mmio_hwport->m_dev5_reg_00A & SCECdStatShellOpen) )
				{
					int last_err;

					last_err = 2;
					while ( last_err != SCECdErNO && last_err != SCECdErABRT )
					{
						VERBOSE_KPRINTF(1, "GO->Atapi sceCdChgSpdlCtrl Call %d\n", 0);
						while ( !sceCdChgSpdlCtrl(0) )
						{
							DelayThread(16000);
						}
						sceCdSync(3);
						last_err = sceCdGetError();
						if ( last_err != SCECdErNO )
						{
							VERBOSE_KPRINTF(1, "** sceCdChgSpdlCtrl ERR 0x%02x\n", last_err);
							DelayThread(250000);
						}
					}
					VERBOSE_KPRINTF(1, "** sceCdChgSpdlCtrl OK\n", last_err);
				}
			}
			set_cdvd_dev5_base_addr_atapi(1);
			g_cdvdman_istruct.m_cd_mode_ps2_atapi = 1;
			if (
				g_cdvdman_vernotxxx1x && disc_type != SCECdNODISC && g_cdvdman_istruct.m_chgsys_callback_next_disktype_last
				&& g_cdvdman_istruct.m_chgsys_callback )
			{
				int i;

				for ( i = 0;
							i < 10 && g_cdvdman_istruct.m_chgsys_callback(&g_cdvdman_istruct.m_chgsys_callback_next_disktype, 1);
							i += 1 )
				{
					DelayThread(16000);
				}
				g_cdvdman_istruct.m_chgsys_callback_next_disktype_last = SCECdNODISC;
			}
			if ( g_cd_atapi_evfid != -1 )
			{
				SetEventFlag(g_cd_atapi_evfid, 3);
			}
			if ( g_adma_evfid != -1 )
			{
				SetEventFlag(g_adma_evfid, 1);
			}
			if ( g_acmd_evfid != -1 )
			{
				SetEventFlag(g_acmd_evfid, 1);
			}
			SetEventFlag(g_scmd_evfid, 1);
			SetEventFlag(g_fio_fsv_evfid, 1);
			VERBOSE_KPRINTF(1, "Go SCECdChgWriDrv End.\n");
		}
		else
		{
			VERBOSE_KPRINTF(1, "Go SCECdChgPs2Drv\n");
			VERBOSE_KPRINTF(1, "scmd_evfid\n");
			WaitEventFlag(g_scmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
			if ( g_acmd_evfid != -1 )
			{
				VERBOSE_KPRINTF(1, "g_acmd_evfid Wait\n");
				WaitEventFlag(g_acmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
			}
			if ( g_adma_evfid != -1 )
			{
				VERBOSE_KPRINTF(1, "g_adma_evfid Wait\n");
				WaitEventFlag(g_adma_evfid, 1, WEF_AND | WEF_CLEAR, &efbits);
			}
			if ( g_cd_atapi_evfid != -1 )
			{
				VERBOSE_KPRINTF(1, "g_cd_atapi_evfid Wait\n");
				WaitEventFlag(g_cd_atapi_evfid, 3, WEF_AND | WEF_CLEAR, &efbits);
			}
			VERBOSE_KPRINTF(1, "evflg Ok\n");
			set_cdvd_dev5_base_addr_atapi(1);
			g_cdvdman_istruct.m_chgsys_callback_next_disktype = SCECdDETCTDVDD;
			if ( g_cdvdman_vernotxxx1x )
			{
				set_cdvd_dev5_base_addr_atapi(2);
				disc_type = dev5_mmio_hwport->m_dev5_reg_00F;
				set_cdvd_dev5_base_addr_atapi(1);
				VERBOSE_KPRINTF(1, "GO->PS2 STOP ATAPI SPINDL\n");
				if ( disc_type == 6 && g_cdvdman_istruct.m_chgsys_callback )
				{
					int i;

					for ( i = 0;
								i < 10 && g_cdvdman_istruct.m_chgsys_callback(&g_cdvdman_istruct.m_chgsys_callback_next_disktype, 0);
								i += 1 )
					{
						DelayThread(16000);
					}
					if ( i == 10 )
					{
						g_cdvdman_istruct.m_chgsys_callback_next_disktype = SCECdDETCTDVDD;
					}
					VERBOSE_KPRINTF(1, "Atapi Set -> Dragon Media %d\n", g_cdvdman_istruct.m_chgsys_callback_next_disktype);
					if ( g_cdvdman_istruct.m_chgsys_callback_next_disktype == 256 )
					{
						g_cdvdman_istruct.m_chgsys_callback_next_disktype_last = g_cdvdman_istruct.m_chgsys_callback_next_disktype;
						g_cdvdman_istruct.m_chgsys_callback_next_disktype = SCECdDETCTDVDD;
					}
				}
			}
			set_cdvd_dev5_base_addr_atapi(use_writer_drive);
			g_cdvdman_istruct.m_cd_mode_ps2_atapi = use_writer_drive;
			disc_type = dev5_mmio_hwport->m_dev5_reg_00F;
			if ( g_cdvdman_vernotxxx1x )
			{
				if (
					((dev5_mmio_hwport->m_dev5_reg_015 & 0x80) && !(dev5_mmio_hwport->m_dev5_reg_00A & SCECdStatShellOpen))
					|| (dev5_mmio_hwport->m_dev5_reg_00A & SCECdStatShellOpen) )
				{
					int last_err;

					last_err = 2;
					while ( last_err != SCECdErNO && last_err != SCECdErABRT && last_err != SCECdErPRM )
					{
						if ( (dev5_mmio_hwport->m_dev5_reg_00A & SCECdStatShellOpen) )
						{
							g_cdvdman_istruct.m_chgsys_callback_next_disktype = SCECdDETCT;
						}
						VERBOSE_KPRINTF(
							1,
							"GO->DRAGON sceCdChgSpdlCtrl Call %d Ps2MediaType %02x\n",
							g_cdvdman_istruct.m_chgsys_callback_next_disktype,
							disc_type);
						while ( !sceCdChgSpdlCtrl(g_cdvdman_istruct.m_chgsys_callback_next_disktype) )
						{
							DelayThread(16000);
						}
						sceCdSync(3);
						last_err = sceCdGetError();
						if ( last_err != SCECdErNO )
						{
							VERBOSE_KPRINTF(1, "** sceCdChgSpdlCtrl ERR 0x%02x\n", last_err);
							DelayThread(250000);
						}
					}
					VERBOSE_KPRINTF(1, "** sceCdChgSpdlCtrl OK\n");
				}
			}
			else
			{
				VERBOSE_KPRINTF(1, " -> Ps2 May be No media %08x\n", disc_type);
			}
			SetEventFlag(g_ncmd_evfid, 1);
			SetEventFlag(g_scmd_evfid, 1);
			SetEventFlag(g_sfile_evfid, 1);
			SetEventFlag(g_fio_fsv_evfid, 1);
			VERBOSE_KPRINTF(1, "Go SCECdChgPs2Drv End.\n");
		}
	}
	SetEventFlag(g_cdvdman_csys_evfid, 1);
	return current_mode;
}
#endif

static int
cdvdman_send_ncmd(int ncmd, const void *ndata, int ndlen, int func, cdvdman_dma3_parameter_t *dma3_param, int check_cb)
{
	int i;
	u32 efbits;
	USE_DEV5_MMIO_HWPORT();

	if ( check_cb == 1 && PollEventFlag(g_ncmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND )
	{
		return -1;
	}
	if (
		// The following direct register access was replaced with call for ioprp300x
		(sceCdDiskReady(8) & 0xC0) != 0x40 || !g_cdvdman_istruct.m_wait_flag
		|| !(g_cdvdman_istruct.m_read2_flag != 1 || ncmd == 8) || !(g_cdvdman_istruct.m_read2_flag != 2 || ncmd == 6) )
	{
		if ( check_cb == 1 )
		{
			vSetEventFlag(g_ncmd_evfid, 1);
		}
		// The following direct register access was replaced with call for ioprp300x
		VERBOSE_KPRINTF(1, "set_cd_commnad Error\tstat %02x\n", (u8)sceCdDiskReady(8));
		return -1;
	}
	g_cdvdman_iocache = 0;
	if ( dma3_param )
	{
		cdvdman_setdma3(dma3_param);
	}
	g_cdvdman_cmdfunc = func;
	// The following call to sceCdGetDiskType was inlined
	if (
		!g_cdvdman_minver_10700 && g_cdvdman_ncmd == 0x06 && ncmd && ncmd != g_cdvdman_ncmd && ncmd != 0x07 && ncmd != 0x0E
		&& ncmd != 0x08
#ifdef CDVD_VARIANT_XOSD
		&& ncmd != 0x0C
#endif
		&& (sceCdGetDiskType() != SCECdCDDA || ncmd == 0x03) )
	{
		g_cdvdman_ncmd_timeout.hi = 0;
		g_cdvdman_ncmd_timeout.lo = 0x6978000;
		vSetAlarm(&g_cdvdman_ncmd_timeout, ncmd_timeout_alarm_cb, &g_cdvdman_ncmd_timeout);
		while ( dev5_mmio_hwport->m_dev5_reg_00A != SCECdStatPause )
		{
			VERBOSE_KPRINTF(1, "Read Pause 1 chk status 0x%02x\n", dev5_mmio_hwport->m_dev5_reg_00A);
			if ( !g_cdvdman_ncmd_timeout.lo )
			{
				g_cdvdman_ncmd = ncmd;
				if ( check_cb == 1 )
				{
					vSetEventFlag(g_ncmd_evfid, 1);
				}
				KPRINTF("Time Out Pause WAIT set_cd_commnad\n");
				return -1;
			}
			vDelayThread(1000);
		}
		vCancelAlarm(ncmd_timeout_alarm_cb, &g_cdvdman_ncmd_timeout);
	}
	g_cdvdman_ncmd = ncmd;
	if ( g_cdvdman_istruct.m_dec_state )
	{
		sceCdDecSet(!!g_cdvdman_istruct.m_dec_shift, 1, g_cdvdman_istruct.m_dec_shift);
	}
	g_cdvdman_istruct.m_last_read_timeout = 0;
	g_cdvdman_istruct.m_cdvdman_command = ncmd;
	g_cdvdman_istruct.m_last_error = SCECdErNO;
	g_cdvdman_istruct.m_wait_flag = 0;
	g_cdvdman_istruct.m_thread_id = GetThreadId();
	if ( QueryIntrContext() )
	{
		iClearEventFlag(g_cdvdman_intr_evfid, ~1);
	}
	else
	{
		ClearEventFlag(g_cdvdman_intr_evfid, ~1);
	}
	for ( i = 0; i < ndlen; i += 1 )
	{
		dev5_mmio_hwport->m_dev5_reg_005 = ((u8 *)ndata)[i];
	}
	dev5_mmio_hwport->m_dev5_reg_004 = ncmd;
	if ( check_cb == 1 )
	{
		vSetEventFlag(g_ncmd_evfid, 1);
	}
	return 0;
}

int sceCdApplyNCmd(u8 cmdNum, const void *inBuff, u16 inBuffSize)
{
	VERBOSE_KPRINTF(1, "Apply NCmd call cmd= 0x%02x\n", cmdNum);
	while ( cdvdman_send_ncmd(cmdNum, inBuff, inBuffSize, 0, 0, 1) < 0 )
	{
		vDelayThread(2000);
	}
	sceCdSync(4);
	return 1;
}

int sceCdCheckCmd(void)
{
	return g_cdvdman_istruct.m_wait_flag;
}

static int cdvdman_mediactl(int code)
{
	int reg_00B_tmp_1;
	int retval;
	u32 efbits;
	char rdata[1];
#ifndef CDVD_VARIANT_XOSD
	USE_DEV5_MMIO_HWPORT();
#endif

	rdata[0] = 0;
	if ( vPollEventFlag(g_scmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND )
	{
		return 0;
	}
#ifdef CDVD_VARIANT_XOSD
	reg_00B_tmp_1 = get_cdvd_register(0x0B) & SCECdStatShellOpen;
#else
	reg_00B_tmp_1 = dev5_mmio_hwport->m_dev5_reg_00B & SCECdStatShellOpen;
#endif
	if ( reg_00B_tmp_1 == g_cdvdman_chmedia )
	{
		retval = 0;
		if ( g_cdvdman_chflags[code] )
		{
			g_cdvdman_chflags[code] = 0;
			retval = 1;
		}
	}
	else
	{
		unsigned int i;

		for ( i = 0; i < (sizeof(g_cdvdman_chflags) / sizeof(g_cdvdman_chflags[0])); i += 1 )
		{
			g_cdvdman_chflags[i] = i != (unsigned int)code;
		}
		retval = 1;
	}
#ifdef CDVD_VARIANT_XOSD
	if ( ((get_cdvd_register(0x0A)) & SCECdStatShellOpen) != reg_00B_tmp_1 )
#else
	if ( ((dev5_mmio_hwport->m_dev5_reg_00A) & SCECdStatShellOpen) != reg_00B_tmp_1 )
#endif
	{
		while ( !set_prev_command(0x05, NULL, 0, rdata, sizeof(rdata), 0) || rdata[0] )
		{
			vDelayThread(4000);
		}
	}
#ifdef CDVD_VARIANT_XOSD
	g_cdvdman_chmedia = get_cdvd_register(0x0B) & 1;
#else
	g_cdvdman_chmedia = dev5_mmio_hwport->m_dev5_reg_00B & SCECdStatShellOpen;
#endif
	vSetEventFlag(g_scmd_evfid, 1);
	return retval;
}

int sceCdGetError(void)
{
	if ( g_cdvdman_istruct.m_last_error != SCECdErNO )
	{
		VERBOSE_KPRINTF(1, "sceCdGetError: 0x%02x\n", (u8)g_cdvdman_istruct.m_last_error);
	}
	return (u8)g_cdvdman_istruct.m_last_error;
}

#ifdef DEAD_CODE
int cdvdman_get_last_command(void)
{
	return (u8)g_cdvdman_istruct.m_cdvdman_command;
}
#endif

int sceCdNop(void)
{
	return cdvdman_send_ncmd(0x00, NULL, 0, 0, 0, 1) >= 0;
}

#ifdef DEAD_CODE
int cdvdman_ncmd_sender_01(void)
{
	return cdvdman_send_ncmd(0x01, NULL, 0, 0, 0, 1) >= 0;
}
#endif

#ifndef CDVD_VARIANT_XOSD
static int cdvdman_ncmd_sender_06(void)
{
	int i;
	cdvdman_dma3_parameter_t dma3_param;
	char ndata[11];

	// The following call to sceCdGetDiskType was inlined
	if ( sceCdGetDiskType() == SCECdNODISC )
	{
		return 1;
	}
	for ( i = 0; i < 48; i += 8 )
	{
		// The following Kprintf was removed for ioprp300x
		KPRINTF("CMD_READP call\n");
		ndata[0] = i + 17;
		ndata[3] = 0;
		ndata[2] = 0;
		ndata[1] = 0;
		ndata[4] = 8;
		ndata[7] = 0;
		ndata[6] = 0;
		ndata[5] = 0;
		ndata[8] = 0;
		ndata[9] = 1;
		ndata[10] = 0;
		dma3_param.m_cdvdreg_howto = 128;
		dma3_param.m_dma3_blkwords = 32;
		dma3_param.m_dma3_blkcount = 128;
		dma3_param.m_dma3_csectors = 0;
		dma3_param.m_dma3_msectors = 0;
		dma3_param.m_dma3_callback = 0;
		dma3_param.m_dma3_maddress = g_cdvdman_ptoc;
		if ( cdvdman_send_ncmd(0x06, ndata, sizeof(ndata), 5, &dma3_param, 1) < 0 )
		{
			return 0;
		}
		sceCdSync(3);
	}
	return 1;
}
#endif

int sceCdStandby(void)
{
	cdvdman_dma3_parameter_t dma3_param;
	char ndata[11];

#ifdef CDVD_VARIANT_XOSD
	switch ( get_disk_type_ex() )
#else
	// The following call to sceCdGetDiskType was inlined
	switch ( sceCdGetDiskType() )
#endif
	{
		case SCECdPSCD:
		case SCECdPSCDDA:
		case SCECdPS2CD:
		case SCECdPS2CDDA:
		case SCECdPS2DVD:
			ndata[0] = 16;
			ndata[4] = 1;
			ndata[9] = 1;
			dma3_param.m_cdvdreg_howto = 128;
			dma3_param.m_dma3_blkwords = 32;
			dma3_param.m_dma3_blkcount = 16;
			ndata[3] = 0;
			ndata[2] = 0;
			ndata[1] = 0;
			ndata[7] = 0;
			ndata[6] = 0;
			ndata[5] = 0;
			ndata[8] = 0;
			ndata[10] = 0;
			dma3_param.m_dma3_csectors = 0;
			dma3_param.m_dma3_msectors = 0;
			dma3_param.m_dma3_callback = 0;
			dma3_param.m_dma3_maddress = g_cdvdman_ptoc;
			return cdvdman_send_ncmd(0x06, ndata, sizeof(ndata), 5, &dma3_param, 1) >= 0;
		default:
			return cdvdman_send_ncmd(0x02, NULL, 0, 5, 0, 1) >= 0;
	}
}

int sceCdStop(void)
{
	return cdvdman_send_ncmd(0x03, NULL, 0, 6, 0, 1) >= 0;
}

int sceCdPause(void)
{
	return cdvdman_send_ncmd(0x04, NULL, 0, 7, 0, 1) >= 0;
}

#ifdef DEAD_CODE
int cdvdman_ncmd_sender_0B(void)
{
	char ndata[1];

	ndata[0] = 1;
	return cdvdman_send_ncmd(0x0B, ndata, sizeof(ndata), 0, 0, 1) >= 0;
}
#endif

static unsigned int readtoc_timeout_alarm_cb(void *userdata)
{
	iop_sys_clock_t *sys_clock;
	USE_DEV5_MMIO_HWPORT();

	sys_clock = (iop_sys_clock_t *)userdata;
	KPRINTF("Cmd Time Out %d(msec)\n", sys_clock->lo / 0x9000);
	dev5_mmio_hwport->m_dev5_reg_007 = 1;
	sys_clock->lo = 0;
	return 0;
}

static int cdvdman_readtoc(u8 *toc, int param, int func)
{
	int last_err;
	cdvdman_dma3_parameter_t dma3_param;
	iop_sys_clock_t sysclk;
	char ndata[1];

#ifdef CDVD_VARIANT_XOSD
	switch ( get_disk_type_ex() )
#else
	// The following call to sceCdGetDiskType was inlined
	switch ( sceCdGetDiskType() )
#endif
	{
		case SCECdPS2DVD:
		case SCECdDVDVR:
		case SCECdDVDV:
			dma3_param.m_cdvdreg_howto = 132;
			dma3_param.m_dma3_blkwords = 4;
			dma3_param.m_dma3_blkcount = 129;
			dma3_param.m_dma3_maddress = toc;
			dma3_param.m_dma3_msectors = 0;
			dma3_param.m_dma3_csectors = 0;
			dma3_param.m_dma3_callback = 0;
			ndata[0] = param;
			break;
		case SCECdPSCD:
		case SCECdPSCDDA:
		case SCECdPS2CD:
		case SCECdPS2CDDA:
		case SCECdCDDA:
			dma3_param.m_cdvdreg_howto = 128;
			dma3_param.m_dma3_blkwords = 32;
			dma3_param.m_dma3_blkcount = 8;
			dma3_param.m_dma3_maddress = toc;
			dma3_param.m_dma3_msectors = 0;
			dma3_param.m_dma3_csectors = 0;
			dma3_param.m_dma3_callback = 0;
			ndata[0] = 0;
			break;
		default:
			return 0;
	}
	if ( cdvdman_send_ncmd(0x09, ndata, sizeof(ndata), func, &dma3_param, 1) < 0 )
	{
		return 0;
	}
	sysclk.hi = 0;
	sysclk.lo = 0x15F90000;
	vSetAlarm(&sysclk, readtoc_timeout_alarm_cb, &sysclk);
	sceCdSync(3);
	vCancelAlarm(readtoc_timeout_alarm_cb, &sysclk);
	last_err = sceCdGetError();
	if ( g_cdvdman_minver_10700 && !sceCdPause() )
	{
		return 0;
	}
	sceCdSync(3);
	return last_err == SCECdErNO;
}

static int cdvdman_gettoc(u8 *toc)
{
	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(1, "sceCdReadToc() call 0x%p\n", toc);
	return cdvdman_readtoc(toc, 0, 3);
}

#ifdef CDVD_VARIANT_OSD
int sceCdGetToc2(u8 *toc, int param)
{
	cdvdman_dma3_parameter_t dma3_param;
	char ndata[1];

	dma3_param.m_cdvdreg_howto = 140;
	dma3_param.m_dma3_blkwords = 12;
	dma3_param.m_dma3_blkcount = 43;
	dma3_param.m_dma3_maddress = toc;
	dma3_param.m_dma3_msectors = 0;
	dma3_param.m_dma3_csectors = 0;
	dma3_param.m_dma3_callback = 0;
	ndata[0] = param;
	return cdvdman_send_ncmd(0x09, ndata, sizeof(ndata), 0, &dma3_param, 1) >= 0;
}
#endif

u32 sceCdGetReadPos(void)
{
	int sector_sizes[4];

	sector_sizes[0] = 0x800;
	sector_sizes[1] = 0x918;
	sector_sizes[2] = 0x924;
	if ( g_cdvdman_istruct.m_recover_status && g_cdvdman_istruct.m_recover_status != 3 )
	{
		return 0;
	}
	if ( g_cdvdman_cmdfunc == SCECdFuncReadCDDA || g_cdvdman_cmdfunc == 12 )
	{
		return dmac_ch_get_madr(3) - (uiptr)g_cdvdman_readbuf;
	}
	if ( g_cdvdman_istruct.m_read2_flag )
	{
		return g_cdvdman_readptr * sector_sizes[g_cdvdman_istruct.m_cdvdman_pattern];
	}
	if ( g_cdvdman_cmdfunc == SCECdFuncRead )
	{
		return dmac_ch_get_madr(3) - (uiptr)g_cdvdman_readbuf;
	}
	return 0;
}

static int cdvdman_speedctl(u32 spindlctrl, int dvdflag, u32 maxlsn)
{
	u32 maxlsn_chk;

	switch ( spindlctrl )
	{
		case SCECdSpinStm:
			return dvdflag ? 2 : 4;
		case SCECdSpinNom:
			if ( !dvdflag )
			{
				return 133;
			}
			if ( g_cdvdman_minver_10700 )
			{
				return 131;
			}
#ifdef CDVD_VARIANT_OSD
			// From DVD Player 3.11
			maxlsn_chk = 0x1C9000;
#else
			maxlsn_chk = 0x128000;
#endif
			if ( g_cdvdman_istruct.m_opo_or_para )
			{
				maxlsn -= (maxlsn >= (u32)g_cdvdman_istruct.m_layer_1_lsn) ? g_cdvdman_istruct.m_layer_1_lsn : 0;
#ifdef CDVD_VARIANT_OSD
				// From DVD Player 3.11
				maxlsn_chk = 0x197000;
#else
				maxlsn_chk = 0x165000;
#endif
			}
			if ( maxlsn >= maxlsn_chk )
			{
				VERBOSE_KPRINTF(1, "Kprob Spd D lsn= %d\n", maxlsn);
				return 130;
			}
			return 133;
		case SCECdSpinX1:
		case 0xE:
			return 1;
		case SCECdSpinX2:
			return 2;
		case SCECdSpinX4:
			return dvdflag ? 2 : 131;
		case SCECdSpinX12:
			return dvdflag ? 3 : 4;
		case SCECdSpinNm2:
			return 64;
		case 0xC:
			return dvdflag ? 4 : 2;
		case 0xF:
			return 130;
		case 0x10:
			return dvdflag ? 130 : 131;
		case 0x11:
			return dvdflag ? 130 : 132;
		case 0x12:
			return dvdflag ? 1 : 131;
		case SCECdSpinMx:
			return dvdflag ? 3 : 5;
		default:
			return dvdflag ? 131 : 133;
	}
}

static int cdvdman_isdvd(void)
{
	// The following call to sceCdGetDiskType was inlined
#ifdef CDVD_VARIANT_XOSD
	switch ( get_disk_type_ex() )
#else
	switch ( sceCdGetDiskType() )
#endif
	{
		case SCECdPSCD:
		case SCECdPSCDDA:
		case SCECdPS2CD:
		case SCECdPS2CDDA:
		case SCECdCDDA:
			g_cdvdman_istruct.m_tray_is_open = 1;
			return 0;
		case SCECdPS2DVD:
		case SCECdDVDVR:
		case SCECdDVDV:
			g_cdvdman_istruct.m_tray_is_open = 1;
			return 1;
		default:
			return 0;
	}
}

static int sceCdRead0_Rty(u32 lsn, u32 nsec, void *buf, const sceCdRMode *mode, int ncmd, int dintrsec, void *func)
{
	cdvdman_dma3_parameter_t dma3_param;
	char ndata[11];

	g_cdvdman_readbuf = buf;
	VERBOSE_KPRINTF(1, "sceCdRead0_Rty Lsn:%d nsec:%d dintrnsec %d func %08x\n", lsn, nsec, dintrsec, func);
	*(u32 *)ndata = lsn;
	*(u32 *)&ndata[4] = nsec;
	ndata[8] = mode->trycount;
	ndata[9] = cdvdman_speedctl(mode->spindlctrl, cdvdman_isdvd(), lsn + nsec);
	dma3_param.m_dma3_csectors = dintrsec;
	dma3_param.m_dma3_callback = (int (*)(void))func;
	dma3_param.m_dma3_msectors = nsec;
	dma3_param.m_dma3_maddress = buf;
	dma3_param.m_dma3_blkcount = (!(u16)dintrsec) ? nsec : 1;
	switch ( ncmd )
	{
		case 0x06:
			ndata[10] = mode->datapattern;
			switch ( mode->datapattern )
			{
				case SCECdSecS2328:
					dma3_param.m_dma3_blkwords = 6;
					dma3_param.m_dma3_blkcount *= 97;
					dma3_param.m_cdvdreg_howto = 134;
					break;
				case SCECdSecS2340:
					dma3_param.m_dma3_blkwords = 15;
					dma3_param.m_dma3_blkcount *= 39;
					dma3_param.m_cdvdreg_howto = 143;
					break;
				case SCECdSecS2048:
				default:
					dma3_param.m_dma3_blkwords = 32;
					dma3_param.m_dma3_blkcount *= 16;
					dma3_param.m_cdvdreg_howto = 128;
					break;
			}
			break;
		case 0x08:
			dma3_param.m_dma3_blkwords = 12;
			dma3_param.m_dma3_blkcount *= 43;
			dma3_param.m_cdvdreg_howto = 140;
			ndata[10] = 0;
			break;
		default:
			return 0;
	}
	return cdvdman_send_ncmd(ncmd, ndata, sizeof(ndata), ncmd == 0x06 ? 1 : 14, &dma3_param, 0) >= 0;
}

int sceCdRead0(u32 lsn, u32 sectors, void *buffer, sceCdRMode *mode, int csec, void *callback)
{
	cdvdman_dma3_parameter_t dma3_param;
	char ndata[11];
	u32 efbits;

	if ( PollEventFlag(g_ncmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND )
	{
		return 0;
	}
	VERBOSE_KPRINTF(
		1,
		"DVD/CD sceCdRead0 sec %d num %d spin %d trycnt %d dptn %d adr %08x\n",
		lsn,
		sectors,
		mode->spindlctrl,
		mode->trycount,
		mode->datapattern,
		buffer);
	g_cdvdman_readbuf = buffer;
	dma3_param.m_dma3_csectors = (csec && (sectors < (u32)csec)) ? sectors : (u32)csec;
	dma3_param.m_dma3_callback = (int (*)(void))callback;
	dma3_param.m_dma3_msectors = sectors;
	dma3_param.m_dma3_blkcount = (!csec) ? sectors : 1;
	switch ( mode->datapattern )
	{
		case SCECdSecS2328:
			dma3_param.m_dma3_blkwords = 6;
			dma3_param.m_dma3_blkcount *= 97;
			dma3_param.m_cdvdreg_howto = 134;
			break;
		case SCECdSecS2340:
			dma3_param.m_dma3_blkwords = 15;
			dma3_param.m_dma3_blkcount *= 39;
			dma3_param.m_cdvdreg_howto = 143;
			break;
		case SCECdSecS2048:
		default:
			dma3_param.m_dma3_blkwords = 32;
			dma3_param.m_dma3_blkcount *= 16;
			dma3_param.m_cdvdreg_howto = 128;
			break;
	}
		// The following call to sceCdGetDiskType was inlined
#ifdef CDVD_VARIANT_OSD
	g_cdvdman_istruct.m_dvd_flag = cdvdman_isdvd();
#else
	switch ( sceCdGetDiskType() )
	{
		case SCECdPSCD:
		case SCECdPSCDDA:
		case SCECdPS2CD:
		case SCECdPS2CDDA:
			if ( g_cdvdman_mmode != SCECdMmodeCd && g_cdvdman_mmode != 0xFF )
			{
				vSetEventFlag(g_ncmd_evfid, 1);
				return 0;
			}
			g_cdvdman_istruct.m_dvd_flag = 0;
			break;
		case SCECdPS2DVD:
			if ( g_cdvdman_mmode != SCECdMmodeDvd && g_cdvdman_mmode != 0xFF )
			{
				vSetEventFlag(g_ncmd_evfid, 1);
				return 0;
			}
			g_cdvdman_istruct.m_dvd_flag = 1;
			break;
		default:
			vSetEventFlag(g_ncmd_evfid, 1);
			return 0;
	}
#endif
	g_cdvdman_istruct.m_read_mode = *mode;
	g_cdvdman_istruct.m_read_callback = callback;
	g_cdvdman_istruct.m_read_chunk = dma3_param.m_dma3_csectors;
	g_cdvdman_istruct.m_read_lsn = lsn;
	g_cdvdman_istruct.m_read_sectors = sectors;
	*(u32 *)ndata = lsn;
	*(u32 *)&ndata[4] = sectors;
	ndata[8] = mode->trycount;
	ndata[9] = cdvdman_speedctl(mode->spindlctrl, g_cdvdman_istruct.m_dvd_flag, lsn + sectors);
	g_cdvdman_istruct.m_read_buf = buffer;
	ndata[10] = mode->datapattern;
	dma3_param.m_dma3_maddress = buffer;
	VERBOSE_KPRINTF(1, "Read Command call\n");
	if ( cdvdman_send_ncmd(0x06, ndata, sizeof(ndata), 1, &dma3_param, 0) < 0 )
	{
		vSetEventFlag(g_ncmd_evfid, 1);
		return 0;
	}
	vSetEventFlag(g_ncmd_evfid, 1);
	return 1;
}

static int read_cdvd_cb(cdvdman_internal_struct_t *common)
{
	int sblock;
	int i;
	u32 cdreadlsn;
	int syncdec_res_1;
	sceCdlLOCCD cdrloc;

	sblock = 0;
	for ( i = 0; i < common->m_dma3_param.m_dma3_csectors; i += 1 )
	{
		char syncdec_res_4;
		int errlsn;

		syncdec_res_4 = 0;
		if ( common->m_read2_flag == 3 )
		{
			sblock = 0x924;
			cdrloc.minute = cdvdman_syncdec(
				common->m_dec_state, common->m_check_version, common->m_dec_shift, g_cdvdman_ptoc[(i * sblock)]);
			cdrloc.second = cdvdman_syncdec(
				common->m_dec_state, common->m_check_version, common->m_dec_shift, g_cdvdman_ptoc[(i * sblock) + 1]);
			cdrloc.sector = cdvdman_syncdec(
				common->m_dec_state, common->m_check_version, common->m_dec_shift, g_cdvdman_ptoc[(i * sblock) + 2]);
			cdreadlsn = sceCdPosToInt(&cdrloc);
		}
		else
		{
			sblock = 0x810;
			syncdec_res_1 = (u8)cdvdman_syncdec(
				common->m_dec_state, common->m_check_version, common->m_dec_shift, g_cdvdman_ptoc[(i * sblock) + 3]);
			syncdec_res_1 +=
				(u8)cdvdman_syncdec(
					common->m_dec_state, common->m_check_version, common->m_dec_shift, g_cdvdman_ptoc[(i * sblock) + 2])
				<< 8;
			syncdec_res_1 +=
				(u8)cdvdman_syncdec(
					common->m_dec_state, common->m_check_version, common->m_dec_shift, g_cdvdman_ptoc[(i * sblock) + 1])
				<< 16;
			syncdec_res_4 = cdvdman_syncdec(
				common->m_dec_state, common->m_check_version, common->m_dec_shift, g_cdvdman_ptoc[(i * sblock)]);
			if ( !common->m_cdvdman_dma3sec && !common->m_interupt_read_state )
			{
				common->m_interupt_read_state = (syncdec_res_4 & 0xC) | (((syncdec_res_4 & 0xC) && i) ? 0x80 : 0);
			}
			cdreadlsn = (syncdec_res_1 - 0x30000) + (( common->m_opo_or_para && ((unsigned int)(common->m_cdvdman_lsn + common->m_cdvdman_csec + i) >= common->m_layer_1_lsn && common->m_opo_or_para == 1) ) ? common->m_layer_1_lsn : 0);
		}
		errlsn = common->m_cdvdman_lsn + common->m_cdvdman_csec + common->m_cdvdman_dma3sec + i;
		if ( cdreadlsn != (u32)errlsn )
		{
			VERBOSE_KPRINTF(
				1, "Read_IOP Sector_ID error lsn= %d readlsn= %d layer= %d\n", errlsn, cdreadlsn, (syncdec_res_4 & 1));
			break;
		}
	}
	if ( i == common->m_dma3_param.m_dma3_csectors )
	{
		unsigned int size;

		size = 0;
		switch ( common->m_cdvdman_pattern )
		{
			case 0:
				size = 0x800;
				break;
			case 1:
			default:
				size = 0x918;
				break;
			case 2:
				optimized_memcpy(
					&((char *)(common->m_cdvdman_rbuffer))[0x924 * common->m_cdvdman_dma3sec],
					(const char *)g_cdvdman_ptoc,
					0x924 * i);
				break;
		}
		if ( size )
		{
			for ( i = 0; i < common->m_dma3_param.m_dma3_csectors; i += 1 )
			{
				optimized_memcpy(
					&((char *)(common->m_cdvdman_rbuffer))[(common->m_cdvdman_dma3sec + i) * size],
					(const char *)&g_cdvdman_ptoc[12 + (i * sblock)],
					size);
			}
		}
		g_cdvdman_readptr = common->m_cdvdman_csec + common->m_cdvdman_dma3sec;
	}
	else
	{
		common->m_sync_error += 1;
	}
	return 1;
}

static int cdvdman_read(u32 lsn, u32 sectors, void *buf, sceCdRMode *mode, int decflag, int shift, int ef1, int ef2)
{
	int read_res;
	int state;
	int scres_unused;
	int dvd;
	int ready;

	dvd = cdvdman_isdvd();
	if ( dvd )
	{
		if ( !DvdDual_infochk() )
		{
			if ( ef1 )
			{
				vSetEventFlag(ef1, ef2);
			}
			return 0;
		}
		lsn = sceCdLsnDualChg(lsn);
	}
	else if ( mode->datapattern == SCECdSecS2328 || (g_cdvdman_cd36key && !g_cdvdman_istruct.m_dec_state) )
	{
		int read0_res;

		if ( g_cdvdman_cd36key && !g_cdvdman_istruct.m_dec_state && mode->spindlctrl == SCECdSpinNom )
		{
			mode->spindlctrl = SCECdSpinStm;
		}
		CpuSuspendIntr(&state);
		read0_res = sceCdRead0(lsn, sectors, buf, mode, 0, 0);
		if ( ef1 )
		{
			vSetEventFlag(ef1, ef2);
		}
		CpuResumeIntr(state);
		return read0_res;
	}
	CpuSuspendIntr(&state);
	ready = sceCdDiskReady(8) & 0xC0;
	if ( ready != 0x40 || g_cdvdman_istruct.m_read2_flag )
	{
		// The following Kprintf was modified for ioprp300x
		VERBOSE_KPRINTF(
			1,
			"sceCdRead: Double Booking error r2f= %d waf= %d rdy= %02x\n",
			g_cdvdman_istruct.m_read2_flag,
			g_cdvdman_istruct.m_wait_flag,
			ready);
		if ( ef1 )
		{
			vSetEventFlag(ef1, ef2);
		}
		CpuResumeIntr(state);
		return 0;
	}
	if ( decflag )
	{
		g_cdvdman_istruct.m_dec_shift = shift;
		g_cdvdman_istruct.m_dec_state = 1;
	}
	g_cdvdman_readbuf = buf;
	g_cdvdman_readptr = 0;
	g_cdvdman_istruct.m_cdvdman_lsn = lsn;
	g_cdvdman_istruct.m_cdvdman_csec = 0;
	g_cdvdman_istruct.m_cdvdman_nsec = sectors;
	g_cdvdman_istruct.m_cdvdman_rbuffer = (int)buf;
	g_cdvdman_istruct.m_cdvdman_pattern = dvd ? SCECdSecS2048 : mode->datapattern;
	g_cdvdman_istruct.m_cdvdman_cdrmode.trycount = mode->trycount;
	g_cdvdman_istruct.m_cdvdman_cdrmode.spindlctrl = mode->spindlctrl;
	g_cdvdman_istruct.m_cdvdman_cdrmode.datapattern = dvd ? SCECdSecS2048 : SCECdSecS2340;
	g_cdvdman_istruct.m_read2_flag = dvd ? 1 : 3;
	g_cdvdman_istruct.m_sync_error = 0;
	g_cdvdman_istruct.m_interupt_read_state = 0;
	g_cdvdman_istruct.m_cdvdman_rsec = (sectors >= 0x41) ? (((lsn & 0xF)) ? (0x10 - (lsn & 0xF)) : 0x40) : sectors;
	g_cdvdman_read_alarm_cb_timeout.hi = 0;
	g_cdvdman_read_alarm_cb_timeout.lo = 0x9000 * sceCdSC(0xFFFFFFF1, &scres_unused);
	vSetAlarm(&g_cdvdman_read_alarm_cb_timeout, read_timeout_alarm_cb, &g_cdvdman_read_alarm_cb_timeout);
	read_res = (dvd ? sceCdRV : sceCdRead0)(
		lsn,
		g_cdvdman_istruct.m_cdvdman_rsec,
		g_cdvdman_ptoc,
		dvd ? mode : &g_cdvdman_istruct.m_cdvdman_cdrmode,
		g_cdvdman_cache_sector_count,
		read_cdvd_cb);
	if ( !read_res )
	{
		g_cdvdman_istruct.m_last_error = SCECdErREADCFR;
		g_cdvdman_istruct.m_cdvdman_rsec = 0;
		g_cdvdman_istruct.m_read2_flag = 0;
		if ( g_cdvdman_istruct.m_dec_state )
		{
			g_cdvdman_istruct.m_dec_shift = 0;
			g_cdvdman_istruct.m_check_version = 0;
			g_cdvdman_istruct.m_dec_state = 0;
			sceCdDecSet(0, 0, 0);
		}
		vCancelAlarm(read_timeout_alarm_cb, &g_cdvdman_read_alarm_cb_timeout);
	}
	if ( ef1 )
	{
		vSetEventFlag(ef1, ef2);
	}
	CpuResumeIntr(state);
	return !!read_res;
}

int sceCdRE(unsigned int lsn, unsigned int sectors, void *buf, sceCdRMode *mode)
{
	return cdvdman_read(lsn, sectors, buf, mode, 0, 0, 0, 0);
}

int sceCdRead(u32 lbn, u32 sectors, void *buffer, sceCdRMode *mode)
{
	iop_event_info_t efinfo;
	int state;

	// Unofficial: initialize to 0
	memset(&efinfo, 0, sizeof(efinfo));
	CpuSuspendIntr(&state);
	vReferEventFlagStatus(g_fio_fsv_evfid, &efinfo);
	if ( !(efinfo.currBits & 1) )
	{
		CpuResumeIntr(state);
		return 0;
	}
	vClearEventFlag(g_fio_fsv_evfid, ~1);
	CpuResumeIntr(state);
	return cdvdman_read(lbn, sectors, buffer, mode, 0, 0, g_fio_fsv_evfid, 1);
}

static int cdvdman_syncdec(int decflag, int decxor, int shift, u32 data)
{
	return decflag ? ((u8)(((u8)data << (shift % 8)) | ((u8)data >> (8 - shift % 8))) ^ (u8)decxor) : (u8)data;
}

static void Read2intrCDVD(int read2_flag)
{
	iCancelAlarm(read_timeout_alarm_cb, &g_cdvdman_read_alarm_cb_timeout);
	if ( g_cdvdman_istruct.m_last_error != SCECdErNO || g_cdvdman_retries >= 5 )
	{
		if ( g_cdvdman_istruct.m_last_error == SCECdErNO )
		{
			g_cdvdman_istruct.m_last_error = SCECdErREADCF;
		}
		g_cdvdman_istruct.m_read2_flag = 0;
		g_cdvdman_retries = 0;
		g_cdvdman_rtindex = 0;
		g_cdvdman_readptr = 0;
		if ( g_cdvdman_istruct.m_dec_state )
		{
			g_cdvdman_istruct.m_dec_shift = 0;
			g_cdvdman_istruct.m_check_version = 0;
			g_cdvdman_istruct.m_dec_state = 0;
		}
		g_cdvdman_istruct.m_interupt_read_state = 0;
	}
	else if ( !g_cdvdman_istruct.m_interupt_read_state || g_cdvdman_istruct.m_cdvdman_csec )
	{
		int scres_unused;

		g_cdvdman_istruct.m_interupt_read_state = 0;
		if ( g_cdvdman_istruct.m_sync_error )
		{
			u32 lsn_tmp;

			if ( !g_cdvdman_rtindex )
			{
				g_cdvdman_rtindex = 3;
				g_cdvdman_retries += 1;
			}
			g_cdvdman_istruct.m_sync_error = 0;
			lsn_tmp = g_cdvdman_istruct.m_cdvdman_lsn + g_cdvdman_istruct.m_cdvdman_csec;
			if ( lsn_tmp >= 0x30 )
			{
				lsn_tmp -= 0x10 * (g_cdvdman_rtindex - 1);
			}
			else
			{
				lsn_tmp += 0x10 * (g_cdvdman_rtindex - 1);
			}
			if ( ((read2_flag == 3) ? sceCdRead0 : sceCdRV)(
						 lsn_tmp,
						 g_cdvdman_istruct.m_cdvdman_rsec,
						 g_cdvdman_ptoc,
						 &g_cdvdman_istruct.m_cdvdman_cdrmode,
						 g_cdvdman_cache_sector_count,
						 read_cdvd_cb) )
			{
				g_cdvdman_read_alarm_cb_timeout.hi = 0;
				g_cdvdman_read_alarm_cb_timeout.lo = 0x9000 * sceCdSC(0xFFFFFFF1, &scres_unused);
				iSetAlarm(&g_cdvdman_read_alarm_cb_timeout, read_timeout_alarm_cb, &g_cdvdman_read_alarm_cb_timeout);
			}
			else
			{
				VERBOSE_KPRINTF(1, "Retry Read Fatal Error\n");
				g_cdvdman_istruct.m_last_error = SCECdErNORDY;
				g_cdvdman_istruct.m_read2_flag = 0;
				g_cdvdman_retries = 0;
				g_cdvdman_rtindex = 0;
				g_cdvdman_readptr = 0;
				if ( g_cdvdman_istruct.m_dec_state )
				{
					g_cdvdman_istruct.m_dec_shift = 0;
					g_cdvdman_istruct.m_check_version = 0;
					g_cdvdman_istruct.m_dec_state = 0;
				}
			}
			g_cdvdman_rtindex -= !!g_cdvdman_rtindex;
		}
		else
		{
			int cdsectorsz;

			g_cdvdman_retries = 0;
			switch ( g_cdvdman_istruct.m_cdvdman_pattern )
			{
				case 0:
					cdsectorsz = 0x800;
					break;
				case 1:
				default:
					cdsectorsz = 0x918;
					break;
				case 2:
					cdsectorsz = 0x924;
					break;
			}
			g_cdvdman_istruct.m_cdvdman_rbuffer += cdsectorsz * g_cdvdman_istruct.m_cdvdman_rsec;
			g_cdvdman_istruct.m_cdvdman_csec += g_cdvdman_istruct.m_cdvdman_rsec;
			if ( (unsigned int)g_cdvdman_istruct.m_cdvdman_csec < (unsigned int)g_cdvdman_istruct.m_cdvdman_nsec )
			{
				g_cdvdman_istruct.m_cdvdman_rsec =
					((unsigned int)(g_cdvdman_istruct.m_cdvdman_csec + 0x40) < (unsigned int)g_cdvdman_istruct.m_cdvdman_nsec) ?
						0x40 :
						(g_cdvdman_istruct.m_cdvdman_nsec - g_cdvdman_istruct.m_cdvdman_csec);
				if ( ((read2_flag == 3) ? sceCdRead0 : sceCdRV)(
							 g_cdvdman_istruct.m_cdvdman_lsn + g_cdvdman_istruct.m_cdvdman_csec,
							 g_cdvdman_istruct.m_cdvdman_rsec,
							 g_cdvdman_ptoc,
							 &g_cdvdman_istruct.m_cdvdman_cdrmode,
							 g_cdvdman_cache_sector_count,
							 read_cdvd_cb) )
				{
					g_cdvdman_read_alarm_cb_timeout.hi = 0;
					g_cdvdman_read_alarm_cb_timeout.lo = 0x9000 * sceCdSC(0xFFFFFFF1, &scres_unused);
					iSetAlarm(&g_cdvdman_read_alarm_cb_timeout, read_timeout_alarm_cb, &g_cdvdman_read_alarm_cb_timeout);
				}
				else
				{
					g_cdvdman_istruct.m_last_error = SCECdErNORDY;
					g_cdvdman_istruct.m_read2_flag = 0;
					g_cdvdman_readptr = 0;
					if ( g_cdvdman_istruct.m_dec_state )
					{
						g_cdvdman_istruct.m_dec_shift = 0;
						g_cdvdman_istruct.m_check_version = 0;
						g_cdvdman_istruct.m_dec_state = 0;
					}
				}
			}
			else
			{
				g_cdvdman_istruct.m_read2_flag = 0;
				g_cdvdman_readptr = 0;
				if ( g_cdvdman_istruct.m_dec_state )
				{
					g_cdvdman_istruct.m_dec_shift = 0;
					g_cdvdman_istruct.m_check_version = 0;
					g_cdvdman_istruct.m_dec_state = 0;
				}
			}
		}
	}
	else
	{
		g_cdvdman_istruct.m_last_error = ((g_cdvdman_istruct.m_interupt_read_state & 0x80)) ? SCECdErREADCF : SCECdErIPI;
		g_cdvdman_istruct.m_interupt_read_state = 0;
		VERBOSE_KPRINTF(1, "IPIerr emu Hit Dummy Err %02x\n", (u8)g_cdvdman_istruct.m_last_error);
		g_cdvdman_istruct.m_read2_flag = 0;
		g_cdvdman_retries = 0;
		g_cdvdman_rtindex = 0;
		g_cdvdman_readptr = 0;
		if ( g_cdvdman_istruct.m_dec_state )
		{
			g_cdvdman_istruct.m_dec_shift = 0;
			g_cdvdman_istruct.m_check_version = 0;
			g_cdvdman_istruct.m_dec_state = 0;
		}
	}
}

int sceCdReadChain(sceCdRChain *tag, sceCdRMode *mode)
{
	(void)tag;
	(void)mode;

	return 0;
}

static int cdvdman_readfull(u32 lsn, u32 sectors, void *buf, const sceCdRMode *mode, int flag)
{
	cdvdman_dma3_parameter_t dma3_param;
	char ndata[11];

	VERBOSE_KPRINTF(1, "lsn:%d nsec:%d buf:% cmdmode:%d\n", lsn, sectors, buf, flag);
	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(1, "DA Read lsn= %d nsec= %d datapattern= %d\n", lsn, sectors, mode->datapattern);
	g_cdvdman_readbuf = buf;
#ifdef CDVD_VARIANT_XOSD
	dma3_param.m_dma3_csectors = sectors;
	dma3_param.m_dma3_msectors = (u16)sectors;
#else
	dma3_param.m_dma3_csectors = 0;
	dma3_param.m_dma3_msectors = 0;
#endif
	dma3_param.m_dma3_callback = 0;
	dma3_param.m_dma3_blkcount = sectors;
	switch ( mode->datapattern )
	{
		case SCECdSecS2328:
			dma3_param.m_dma3_blkwords = 8;
			dma3_param.m_dma3_blkcount *= 74;
			dma3_param.m_cdvdreg_howto = 136;
			break;
		case SCECdSecS2340:
			dma3_param.m_dma3_blkwords = 12;
			dma3_param.m_dma3_blkcount *= 51;
			dma3_param.m_cdvdreg_howto = 140;
			break;
		case SCECdSecS2048:
		default:
			dma3_param.m_dma3_blkwords = 12;
			dma3_param.m_dma3_blkcount *= 49;
			dma3_param.m_cdvdreg_howto = 140;
			break;
	}
		// The following call to sceCdGetDiskType() was inlined
#ifdef CDVD_VARIANT_XOSD
	switch ( get_disk_type_ex() )
#else
	switch ( sceCdGetDiskType() )
#endif
	{
		case SCECdPSCDDA:
		case SCECdPS2CDDA:
		case SCECdCDDA:
			break;
		default:
			return 0;
	}
#ifndef CDVD_VARIANT_OSD
	if ( g_cdvdman_mmode != SCECdMmodeCd && g_cdvdman_mmode != 0xFF )
	{
		return 0;
	}
#endif
	*(u32 *)ndata = lsn;
	*(u32 *)&ndata[4] = sectors;
	ndata[8] = mode->trycount;
	ndata[9] = cdvdman_speedctl(mode->spindlctrl, 0, lsn + sectors);
	dma3_param.m_dma3_maddress = buf;
	ndata[10] = mode->datapattern;
	return cdvdman_send_ncmd((!flag) ? 0x07 : 0x0E, ndata, sizeof(ndata), (!flag) ? 2 : 12, &dma3_param, 1) >= 0;
}

// cppcheck-suppress constParameterPointer
int sceCdReadCDDA(u32 lbn, u32 sectors, void *buffer, sceCdRMode *mode)
{
	return cdvdman_readfull(lbn, sectors, buffer, mode, 0);
}

#ifdef CDVD_VARIANT_OSD
// cppcheck-suppress constParameterPointer
int sceCdReadFull(unsigned int lsn, unsigned int sectors, void *buf, sceCdRMode *mode)
{
	return g_cdvdman_minver_20800 ? cdvdman_readfull(lsn, sectors, buf, mode, 1) :
																	cdvdman_readfull(lsn, sectors, buf, mode, 0);
}
#endif

int sceCdRV(u32 lsn, u32 sectors, void *buf, sceCdRMode *mode, int arg5, void *cb)
{
	cdvdman_dma3_parameter_t dma3_param;
	char ndata[11];
	u32 efbits;

	if (
#ifdef CDVD_VARIANT_XOSD
		get_disk_type_ex() != SCECdPS2DVD
#else
		// The following call to sceCdGetDiskType was inlined
		sceCdGetDiskType() != SCECdPS2DVD
#endif
#ifndef CDVD_VARIANT_OSD
		|| (g_cdvdman_mmode != SCECdMmodeDvd && g_cdvdman_mmode != 0xFF)
#endif
		|| (PollEventFlag(g_ncmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND) )
	{
		return 0;
	}
	VERBOSE_KPRINTF(
		1, "RV read: sec %d num %d spin %d trycnt %d  addr %08x\n", lsn, sectors, mode->spindlctrl, mode->trycount, buf);
	g_cdvdman_readbuf = buf;
	g_cdvdman_istruct.m_dvd_flag = cdvdman_isdvd();
	g_cdvdman_istruct.m_read_mode = *mode;
	g_cdvdman_istruct.m_read_lsn = lsn;
	g_cdvdman_istruct.m_read_sectors = sectors;
	*(u32 *)ndata = lsn;
	*(u32 *)&ndata[4] = sectors;
	ndata[8] = mode->trycount;
	ndata[9] = cdvdman_speedctl(mode->spindlctrl, 1, lsn + sectors);
	ndata[10] = 0;
	dma3_param.m_dma3_csectors = (arg5 && (sectors < (u32)arg5)) ? sectors : (u32)arg5;
	g_cdvdman_istruct.m_read_chunk = dma3_param.m_dma3_csectors;
	dma3_param.m_cdvdreg_howto = 140;
	dma3_param.m_dma3_blkwords = 12;
	g_cdvdman_istruct.m_read_buf = buf;
	dma3_param.m_dma3_blkcount = (!arg5) ? sectors : 1;
	dma3_param.m_dma3_blkcount *= 43;
	dma3_param.m_dma3_msectors = sectors;
	dma3_param.m_dma3_callback = (int (*)(void))cb;
	g_cdvdman_istruct.m_read_callback = cb;
	dma3_param.m_dma3_maddress = buf;
	if ( cdvdman_send_ncmd(0x08, ndata, sizeof(ndata), 14, &dma3_param, 0) < 0 )
	{
		vSetEventFlag(g_ncmd_evfid, 1);
		return 0;
	}
	vSetEventFlag(g_ncmd_evfid, 1);
	return 1;
}

#ifdef CDVD_VARIANT_OSD
// cppcheck-suppress constParameterPointer
int sceCdReadDVDV(u32 lbn, u32 sectors, void *buffer, sceCdRMode *mode)
{
	cdvdman_dma3_parameter_t dma3_param;
	char ndata[11];

#ifdef CDVD_VARIANT_XOSD
	switch ( get_disk_type_ex() )
#else
	// The following call to sceCdGetDiskType was inlined
	switch ( sceCdGetDiskType() )
#endif
	{
		case SCECdPS2DVD:
		case SCECdDVDVR:
		case SCECdDVDV:
			break;
		default:
			return 0;
	}
	switch ( mode->spindlctrl )
	{
		case SCECdSpinMax:
		case SCECdSpinX4:
			ndata[9] = 3;
			break;
		case SCECdSpinX1:
			ndata[9] = 1;
			break;
		case SCECdSpinX2:
		case 0xC:
			ndata[9] = 4;
			break;
		case 0xA:
			ndata[9] = 0x40;
			break;
		case 0xB:
			ndata[9] = 2;
			break;
		default:
			ndata[9] = 0x83;
			break;
	}
	*(u32 *)ndata = lbn;
	*(u32 *)&ndata[4] = sectors;
	dma3_param.m_cdvdreg_howto = 140;
	dma3_param.m_dma3_blkwords = 12;
	dma3_param.m_dma3_blkcount = 43 * sectors;
	ndata[10] = 0;
	dma3_param.m_dma3_maddress = buffer;
	dma3_param.m_dma3_msectors = 0;
	dma3_param.m_dma3_csectors = 0;
	dma3_param.m_dma3_callback = 0;
	ndata[8] = mode->trycount;
	return cdvdman_send_ncmd(0x08, ndata, sizeof(ndata), 9, &dma3_param, 1) >= 0;
}
#endif

int sceCdSeek(u32 lbn)
{
	char ndata[4];

	*(u32 *)ndata = lbn;
	if ( cdvdman_isdvd() )
	{
		if ( !DvdDual_infochk() )
		{
			return 0;
		}
		*(u32 *)ndata = sceCdLsnDualChg(lbn);
	}
	return cdvdman_send_ncmd(0x05, ndata, sizeof(ndata), 4, 0, 1) >= 0;
}

#ifdef CDVD_VARIANT_XOSD
static unsigned int sceCdChgSpdlCtrl(int mode)
{
	unsigned int retval;
	char ndata[1];

	ndata[0] = mode;
	retval = ~cdvdman_send_ncmd(0x0F, ndata, sizeof(ndata), 0xF, 0, 0);
	VERBOSE_KPRINTF(1, "Called sceCdChgSpdlCtrl mode:%d ret= %d\n", mode & 0xFF, retval >> 31);
	return retval >> 31;
}
#endif

// cppcheck-suppress funcArgNamesDifferent
int sceCdRI(u8 *buffer, u32 *status)
{
	int retval;
	char rdata[9];

	retval = set_prev_command(0x12, NULL, 0, rdata, sizeof(rdata), 1);
	*status = rdata[0];
	memcpy(buffer, &rdata[1], sizeof(rdata) - 1);
	return retval;
}

int sceCdRM(char *buffer, u32 *status)
{
	int retval;
	int cmd_tmp2;
	char rdata[9];
	char wdata[1];
	u32 efbits;

	*status = SCECdErNO;
	if ( sceCdMV((u8 *)rdata, status) != 1 || (unsigned int)(rdata[3] | (rdata[2] << 8) | (rdata[1] << 16)) < 0x10500 )
	{
		strcpy(buffer, "M_NAME_UNKNOWN");
		buffer[15] = 0;
		*status |= 0x40;
		return 1;
	}
	if ( vPollEventFlag(g_scmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND )
	{
		return 0;
	}
	DelayThread(2000);
	wdata[0] = 0;
	retval = set_prev_command(0x17, wdata, sizeof(wdata), rdata, sizeof(rdata), 0);
	*status = rdata[0];
	memcpy(buffer, &rdata[1], sizeof(rdata) - 1);
	DelayThread(2000);
	wdata[0] = 8;
	cmd_tmp2 = set_prev_command(0x17, wdata, sizeof(wdata), rdata, sizeof(rdata), 0);
	*status |= rdata[0];
	memcpy(&buffer[8], &rdata[1], sizeof(rdata) - 1);
	vSetEventFlag(g_scmd_evfid, 1);
	return retval ? (!!cmd_tmp2) : 0;
}

#ifdef CDVD_VARIANT_OSD
// Not implemented XOSD or DVD Player 3.11
// cppcheck-suppress funcArgNamesDifferent
int sceCdWI(const u8 *buffer, u32 *status)
{
	int retres;

	DelayThread(16000);
	*status = SCECdErNO;
	retres = set_prev_command(0x13, (const char *)buffer, 8, (char *)status, 1, 1);
	DelayThread(16000);
	return retres;
}

// Not implemented XOSD or DVD Player 3.11
int sceCdWM(const char *buffer, u32 *status)
{
	int retres;
	char rdata[1];
	char wdata[9];

	*status = SCECdErNO;
	if ( sceCdMV((u8 *)wdata, status) != 1 || (unsigned int)(wdata[3] | (wdata[2] << 8) | (wdata[1] << 16)) < 0x10500 )
	{
		*status |= 0x40;
		return 1;
	}
	DelayThread(2000);
	wdata[0] = 0;
	// The following was inlined
	memcpy(&wdata[1], &buffer[0], sizeof(wdata) - 1);
	retres = set_prev_command(0x18, wdata, sizeof(wdata), rdata, sizeof(rdata), 1);
	*status = rdata[0];
	DelayThread(16000);
	wdata[0] = 8;
	// The following was inlined
	memcpy(&wdata[1], &buffer[8], sizeof(wdata) - 1);
	retres = set_prev_command(0x18, wdata, sizeof(wdata), rdata, sizeof(rdata), 1) && retres;
	*status |= rdata[0];
	DelayThread(16000);
	return !!retres;
}

// cppcheck-suppress funcArgNamesDifferent
int sceCdReadConsoleID(u8 *buffer, u32 *status)
{
	int retval;
	char rdata[9];
	char wdata[1];

	wdata[0] = 0x45;
	retval = set_prev_command(0x03, wdata, sizeof(wdata), rdata, sizeof(rdata), 1);
	*status = rdata[0];
	memcpy(buffer, &rdata[1], sizeof(rdata) - 1);
	return retval;
}

// Not implemented XOSD or DVD Player 3.11
int sceCdWriteConsoleID(const u8 *buffer, u32 *status)
{
	int retval;
	char wdata[9];

	DelayThread(16000);
	*status = SCECdErNO;
	wdata[0] = 0x44;
	memcpy(&wdata[1], buffer, 8);
	retval = set_prev_command(0x03, wdata, sizeof(wdata), (char *)status, 1, 1);
	DelayThread(16000);
	return retval;
}
#endif

int sceCdMV(u8 *buffer, u32 *status)
{
	int retval;
	char rdata[4];
	char wdata[1];

	wdata[0] = 0x00;
	retval = set_prev_command(0x03, wdata, sizeof(wdata), rdata, sizeof(rdata), 1);
#ifdef CDVD_VARIANT_XOSD
	*status = SCECdErNO;
#else
	*status = rdata[0] & 0x80;
#endif
	VERBOSE_KPRINTF(1, "MV 0x%02x,0x%02x,0x%02x,0x%02x\n", (u8)rdata[0], (u8)rdata[1], (u8)rdata[2], (u8)rdata[3]);
#ifndef CDVD_VARIANT_XOSD
	rdata[0] &= ~0x80;
#endif
	memcpy(buffer, rdata, sizeof(rdata));
	return retval;
}

#ifdef CDVD_VARIANT_XOSD
static int cdvdman_get_renewal_date(u8 *buffer, u32 *status)
{
	int retval;
	char rdata[6];
	char wdata[1];

	*status = SCECdErNO;
	wdata[0] = 0xFD;
	retval = set_prev_command(0x03, wdata, sizeof(wdata), rdata, sizeof(rdata), 1);
	if ( retval )
	{
		*status = (u8)rdata[0];
	}
	memcpy(buffer, &rdata[1], sizeof(rdata) - 1);
	return retval;
}
#endif

#ifndef CDVD_VARIANT_XOSD
static int cdvdman_scmd_sender_03_30(u8 *buf, u32 *status)
{
	int retval;
	char rdata[2];
	char wdata[2];

	if ( g_cdvdman_minver_50000 )
	{
		return 0;
	}
	wdata[0] = 0x30;
	wdata[1] = 2;
	retval = set_prev_command(0x03, wdata, sizeof(wdata), rdata, sizeof(rdata), 1);
	*status = (u8)rdata[0];
	*buf = rdata[1];
	return retval;
}
#endif

#ifdef CDVD_VARIANT_OSD
int sceCdReadSUBQ(void *buffer, u32 *status)
{
	int retval;
	int i;
	int dev5_reg_00E;
	int dev5_reg_00D;
	int dev5_reg_00C;
	char rdata[11];
	u32 efbits;
	USE_DEV5_MMIO_HWPORT();

	if ( vPollEventFlag(g_scmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND )
	{
		return 0;
	}
	dev5_mmio_hwport->m_dev5_reg_009 = 0;
	retval = cdvdman_send_scmd2(0x02, NULL, 0, rdata, sizeof(rdata), 0);
	memcpy(buffer, &rdata[1], sizeof(rdata) - 1);
	for ( i = 0; i < 10; i += 1 )
	{
		int tmp00E;
		int tmp00D;
		int tmp00C;

		dev5_reg_00E = dev5_mmio_hwport->m_dev5_reg_00E;
		dev5_reg_00D = dev5_mmio_hwport->m_dev5_reg_00D;
		dev5_reg_00C = dev5_mmio_hwport->m_dev5_reg_00C;
		tmp00E = dev5_mmio_hwport->m_dev5_reg_00E;
		tmp00D = dev5_mmio_hwport->m_dev5_reg_00D;
		tmp00C = dev5_mmio_hwport->m_dev5_reg_00C;
		if ( dev5_reg_00E == tmp00E && dev5_reg_00D == tmp00D && dev5_reg_00C == tmp00C )
		{
			break;
		}
	}
	if ( i != 10 )
	{
		*((u8 *)buffer + 9) = dev5_reg_00E;
		*((u8 *)buffer + 8) = dev5_reg_00D;
		*((u8 *)buffer + 7) = dev5_reg_00C;
	}
	*status = rdata[0];
	vSetEventFlag(g_scmd_evfid, 1);
	return retval;
}

// cppcheck-suppress funcArgNamesDifferent
int sceCdForbidDVDP(u32 *status)
{
	int retval;
	char rdata[1];

	// Unofficial: NULL for wdata if length is 0
	retval = set_prev_command(0x15, NULL, 0, rdata, sizeof(rdata), 1);
	*status = (u8)rdata[0];
	return retval;
}
#endif

int sceCdMmode(int media)
{
	g_cdvdman_mmode = media;
	return 1;
}

#ifdef CDVD_VARIANT_OSD
// cppcheck-suppress funcArgNamesDifferent
int sceCdForbidRead(u32 *status)
{
	*status = SCECdErNO;
	// Unofficial: NULL for wdata if length is 0
	return set_prev_command(0x19, NULL, 0, (char *)status, 1, 1);
}

int sceCdBootCertify(const u8 *romname)
{
	char rdata[1];

	VERBOSE_KPRINTF(1, "BC %d %d %d %d\n", *romname, romname[1], romname[2], romname[3]);
	return set_prev_command(0x1A, (const char *)romname, 4, rdata, sizeof(rdata), 1);
}
#endif

// cppcheck-suppress funcArgNamesDifferent
int sceCdCancelPOffRdy(u32 *status)
{
	*status = SCECdErNO;
	// Unofficial: NULL for wdata if length is 0
	return g_cdvdman_minver_20400 ? set_prev_command(0x1B, NULL, 0, (char *)status, 1, 1) : 1;
}

#ifdef CDVD_VARIANT_OSD
// cppcheck-suppress funcArgNamesDifferent
int sceCdBlueLEDCtl(u8 control, u32 *status)
{
	char wdata[1];

	*status = SCECdErNO;
	wdata[0] = control;
	return set_prev_command(0x1C, wdata, sizeof(wdata), (char *)status, 1, 1);
}
#endif

static unsigned int power_off_alarm_cb(void *userdata)
{
	cdvdman_internal_struct_t *s;

	s = (cdvdman_internal_struct_t *)userdata;
	s->m_power_flag = 0;
	return 0;
}

// cppcheck-suppress funcArgNamesDifferent
int sceCdPowerOff(u32 *status)
{
	int retval;

	*status = SCECdErNO;
	VERBOSE_KPRINTF(1, "sceCdPowerOff Call\n");
#ifndef CDVD_VARIANT_XOSD
	if ( !g_cdvdman_minver_x_model_15 )
	{
		while ( (sceCdStatus() & SCECdStatShellOpen) )
		{
			u32 traychk;

			sceCdTrayReq(SCECdTrayClose, &traychk);
			vDelayThread(250000);
		}
	}
#endif
	retval = set_prev_command(0x0F, NULL, 0, (char *)status, 1, 1);
	if ( !retval )
	{
		g_cdvdman_istruct.m_power_flag = 0;
		return 0;
	}
	KPRINTF("PowerOff Start...\n");
	g_cdvdman_istruct.m_power_flag = 1;
	g_cdvdman_power_off_timeout.hi = 0;
	g_cdvdman_power_off_timeout.lo = 0xAFC8000;
	vSetAlarm(&g_cdvdman_power_off_timeout, power_off_alarm_cb, &g_cdvdman_istruct);
	return retval;
}

int sceCdCtrlADout(int mode, u32 *status)
{
	char wdata[1];

	*status = SCECdErNO;
	DelayThread(2000);
	VERBOSE_KPRINTF(1, "Audio Digital Out: Set param %d\n", (u8)mode);
	wdata[0] = mode;
	return set_prev_command(0x14, wdata, sizeof(wdata), (char *)status, 1, 1);
}

#ifdef CDVD_VARIANT_OSD
// cppcheck-suppress funcArgNamesDifferent
int sceCdAutoAdjustCtrl(int mode, u32 *status)
{
	char wdata[1];

	*status = SCECdErNO;
	DelayThread(2000);
	VERBOSE_KPRINTF(1, "Auto Adjust Ctrl: Set param %d\n", (u8)mode);
	wdata[0] = mode;
	return set_prev_command(0x16, wdata, sizeof(wdata), (char *)status, 1, 1);
}

// TODO clock argument not const
int sceCdWriteWakeUpTime(const sceCdCLOCK *clock, u16 userdata, int flags)
{
	unsigned int clkmonth;
	unsigned int clkday;
	char wdata[8];

	if ( !g_cdvdman_minver_50000 )
	{
		return 0;
	}
	clkmonth = 10 * (clock->month >> 4) + (clock->month & 0xF);
	clkday = 10 * (clock->day >> 4) + (clock->day & 0xF);
	if (
		(10 * (clock->second >> 4) + (clock->second & 0xF) >= 0x3C
		 || 10 * (clock->minute >> 4) + (clock->minute & 0xF) >= 0x3C
		 || 10 * (clock->hour >> 4) + (clock->hour & 0xF) >= 0x18 || 10 * (clock->year >> 4) + (clock->year & 0xF) >= 0x64
		 || clkmonth >= 0xD || !clkmonth || clkday >= 0x20 || !clkday)
		&& flags != 255 && !(clock->second & 0x80) && !(clock->minute & 0x80) )
	{
		return 0;
	}
	wdata[0] = (flags == 255) ? 255 : clock->second;
	wdata[1] = clock->minute | ((flags == 1) ? 0x80 : 0);
	wdata[2] = clock->hour;
	wdata[3] = clock->day;
	wdata[4] = clock->month;
	wdata[5] = clock->year;
	wdata[6] = userdata & 0xFF;
	wdata[7] = (userdata >> 8) & 0xFF;
	return set_prev_command(0x21, wdata, sizeof(wdata), (char *)&clock->stat, sizeof(clock->stat), 1);
}

int sceCdReadWakeUpTime(sceCdCLOCK *clock, u16 *userdata, u32 *wakeupreason, int *flags)
{
	int retval;
	char rdata[10];

	clock->year = 0;
	clock->month = 0;
	clock->day = 0;
	clock->pad = 0;
	clock->hour = 0;
	clock->minute = 0;
	clock->second = 0;
	clock->stat = 0;
	*userdata = 0;
	*wakeupreason = 0;
	*flags = 0;
	if ( !g_cdvdman_minver_50000 )
	{
		*wakeupreason = 256;
		return 1;
	}
	retval = set_prev_command(0x22, NULL, 0, rdata, sizeof(rdata), 1);
	if ( !retval )
	{
		return 0;
	}
	clock->stat = rdata[0];
	clock->second = (u8)rdata[2] == 255 ? -1 : (rdata[2] & 0x7F);
	clock->minute = rdata[3] & 0x7F;
	clock->pad = 0;
	clock->hour = rdata[4];
	clock->day = rdata[5];
	clock->month = rdata[6];
	clock->year = rdata[7];
	*userdata = (u8)rdata[8] | ((u8)rdata[9] << 8);
	*wakeupreason = (u8)rdata[1];
	*flags = (((u8)rdata[2] >> 7) << 1) | (!!(rdata[3] & 0x80));
	return retval;
}

int sceCdSendSCmd1D(int *arg1, unsigned int *arg2, unsigned int *arg3, u32 *status)
{
	int retval;
	char rdata[5];
	u32 rdata2tmp;

	*arg3 = 0;
	*arg2 = 0;
	*arg1 = 0;
	if ( !g_cdvdman_minver_50000 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	retval = set_prev_command(0x1D, NULL, 0, rdata, sizeof(rdata), 1);
	if ( retval != 1 )
	{
		return retval;
	}
	*status = (u8)rdata[0];
	*arg1 = (u8)rdata[1];
	rdata2tmp = rdata[4] | (rdata[3] << 8) | (rdata[2] << 16);
	switch ( *arg1 )
	{
		case 12:
			*arg2 = (rdata2tmp >> 17) & 0x7F;
			*arg3 = (rdata2tmp >> 12) & 0x1F;
			return 1;
		case 15:
			*arg2 = (rdata2tmp >> 17) & 0x7F;
			*arg3 = (rdata2tmp >> 9) & 0xFF;
			return 1;
		case 20:
			*arg2 = (rdata2tmp >> 17) & 0x7F;
			*arg3 = (rdata2tmp >> 4) & 0x1FFF;
			return 1;
		default:
			return 0;
	}
}

int sceRemote2_7(u16 param, u32 *status)
{
	char wdata[2];

	if ( !g_cdvdman_minver_50000 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	wdata[0] = param & 0xFF;
	wdata[1] = (param >> 8) & 0xFF;
	return set_prev_command(0x1F, wdata, sizeof(wdata), (char *)status, 1, 1);
}

int sceRemote2_7Get(u32 *param, u32 *status)
{
	int retval;
	char rdata[3];

	*param = 0;
	if ( !g_cdvdman_minver_50000 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	retval = set_prev_command(0x26, NULL, 0, rdata, sizeof(rdata), 1);
	*param = ((u8)rdata[2] << 8) | (u8)rdata[1];
	*status = (u8)rdata[0];
	return retval;
}

// cppcheck-suppress funcArgNamesDifferent
int sceCdReadPS1BootParam(u8 *out, u32 *status)
{
	int retval;
	char rdata[13];

	memset(out, 0, 11);
	if ( !g_cdvdman_minver_50200 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	retval = set_prev_command(0x27, NULL, 0, (char *)rdata, sizeof(rdata), 1);
	memcpy(out, &rdata[1], sizeof(rdata) - 2);
	*status = rdata[0];
	return retval;
}

// cppcheck-suppress funcArgNamesDifferent
int sceCdSetFanProfile(u8 param, u32 *status)
{
	int retval;
	char rdata[1];
	char wdata[1];

	if ( !g_cdvdman_minver_50400 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	wdata[0] = param;
	retval = set_prev_command(0x28, wdata, sizeof(wdata), rdata, sizeof(rdata), 1);
	*status = (u8)rdata[0];
	return retval;
}

int cdvdman_152_get_temperature(u32 *param, u32 *status)
{
	int retval;
	char rdata[3];
	char wdata[1];

	if ( !g_cdvdman_minver_50400 )
	{
		*status = 256;
		*param = 0;
		return 1;
	}
	*status = SCECdErNO;
	wdata[0] = 0xEF;
	retval = set_prev_command(0x03, wdata, sizeof(wdata), rdata, sizeof(rdata), 1);
	*param = 3125 * (((u8)rdata[2] | ((u8)rdata[1] << 8)) << 16 >> 18) / 100;
	*status = (u8)rdata[0];
	return retval;
}
#endif

#ifdef CDVD_VARIANT_XOSD
// cppcheck-suppress funcArgNamesDifferent
int sceCdNoticeGameStart(u8 arg1, u32 *status)
{
	int retval;
	char rdata[1];
	char wdata[1];

	if ( !g_cdvdman_minver_50400 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	wdata[0] = arg1;
	retval = set_prev_command(0x29, wdata, sizeof(wdata), rdata, sizeof(rdata), 1);
	*status = (u8)rdata[0];
	if ( retval && !rdata[0] )
	{
		g_cdvdman_istruct.m_medium_removal_state = 0;
	}
	return retval;
}
#endif

#ifdef CDVD_VARIANT_OSD
int sceCdRcBypassCtl(int mode, u32 *status)
{
	char wdata[1];
	USE_IOP_MMIO_HWPORT();

	if ( mfc0(PRID) >= 0x23 )
	{
		if ( mode & 0xFF )
		{
			/* 0xBF808284 */ *(u32 *)&iop_mmio_hwport->sio2.unused[0] |= 1;
		}
		else
		{
			/* 0xBF808284 */ *(u32 *)&iop_mmio_hwport->sio2.unused[0] &= ~1;
		}
		*status = SCECdErNO;
		return 1;
	}
	if ( !g_cdvdman_minver_50000 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	wdata[0] = mode;
	return set_prev_command(0x24, wdata, sizeof(wdata), (char *)status, 1, 1);
}

// cppcheck-suppress funcArgNamesDifferent
int sceCdReadRegionParams(u32 *param, u32 *status)
{
	int retval;
	char rdata[15];

	memset(param, 0, 15);
	if ( !g_cdvdman_minver_60000 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	retval = set_prev_command(0x36, NULL, 0, rdata, sizeof(rdata), 1);
	memcpy(param, &rdata[1], sizeof(rdata) - 1);
	*status = rdata[0];
	return retval;
}

// Not implemented XOSD or DVD Player 3.11
// TODO const arg2 arg3
// cppcheck-suppress funcArgNamesDifferent
int sceCdWriteRegionParams(u8 arg1, u32 *arg2, u8 *arg3, u32 *status)
{
	char wdata[15];

	*status = SCECdErNO;
	memset(wdata, 0, sizeof(wdata));
	if ( !g_cdvdman_minver_60600 )
	{
		*status = 256;
		return 1;
	}
	wdata[0] = arg1;
	// The following was inlined
	memcpy(&wdata[1], arg2, 12);
	// The following was inlined
	memcpy(&wdata[13], arg3, 2);
	return set_prev_command(0x3E, wdata, sizeof(wdata), (char *)status, 1, 1);
}

// cppcheck-suppress funcArgNamesDifferent
int sceCdSetLEDsMode(u32 param, u32 *status)
{
	char wdata[1];

	if ( !g_cdvdman_minver_50000 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	wdata[0] = param;
	return set_prev_command(0x25, wdata, sizeof(wdata), (char *)status, 1, 1);
}
#endif

int sceCdReadClock(sceCdCLOCK *clock)
{
	int retval;

	retval = set_prev_command(0x08, NULL, 0, (char *)clock, 8, 1);
	clock->pad = 0;
	clock->month &= 0x7F;
	if ( retval && !clock->stat )
	{
		memcpy(&g_cdvdman_clock, clock, sizeof(g_cdvdman_clock));
		g_cdvdman_clk_flg = 1;
	}
	else if ( g_cdvdman_clk_flg )
	{
		memcpy(clock, &g_cdvdman_clock, sizeof(g_cdvdman_clock));
	}
	else
	{
		clock->month = 3;
		clock->day = 4;
		clock->hour = 5;
		clock->minute = 6;
		clock->year = 0;
		clock->second = 7;
	}
	return retval;
}

int sceCdRC(sceCdCLOCK *clock)
{
	return set_prev_command(0x08, NULL, 0, (char *)clock, 8, 1);
}

#ifdef CDVD_VARIANT_OSD
int sceCdWriteClock(sceCdCLOCK *clock)
{
	return set_prev_command(
		0x09, (const char *)&clock->second, sizeof(clock) - 1, (char *)&clock->stat, sizeof(clock->second), 1);
}

int sceCdReadNVM(u32 address, u16 *data, u8 *result)
{
	int retval;
	char rdata[3];
	char wdata[2];

	wdata[0] = (address >> 8) & 0xFF;
	wdata[1] = address & 0xFF;
	retval = set_prev_command(0x0A, wdata, sizeof(wdata), rdata, sizeof(rdata), 1);
	*result = rdata[0];
	*data = (rdata[1] << 8) | rdata[2];
	VERBOSE_KPRINTF(1, "RN_call addr= 0x%04x data= 0x%04x stat= 0x%02x\n", address, *data, *result);
	return retval;
}

int sceCdWriteNVM(u32 address, u16 data, u8 *result)
{
	int retval;
	char wdata[4];

	wdata[0] = (address >> 8) & 0xFF;
	wdata[1] = address & 0xFF;
	wdata[2] = (data >> 8) & 0xFF;
	wdata[3] = data & 0xFF;
	retval = set_prev_command(0x0B, wdata, sizeof(wdata), (char *)result, 1, 1);
	VERBOSE_KPRINTF(1, "WN_call addr= 0x%04x data= 0x%04x stat= 0x%02x\n", address, data, *result);
	return retval;
}
#endif

int sceCdTrayReq(int param, u32 *traychk)
{
	char wdata[1];
	char rdata[1];
#ifdef CDVD_VARIANT_XOSD
	int state;
#endif

	if ( param == SCECdTrayCheck )
	{
		*traychk = cdvdman_mediactl(1);
		VERBOSE_KPRINTF(1, "Tray Req test = %d\n", *traychk);
		return 1;
	}
	if ( g_cdvdman_minver_x_model_15 && param == SCECdTrayClose )
	{
		return 1;
	}
	wdata[0] = param;
	g_cdvdman_iocache = 0;
#ifdef CDVD_VARIANT_XOSD
	CpuSuspendIntr(&state);
#endif
	rdata[0] = !!set_prev_command(0x06, wdata, sizeof(wdata), rdata, sizeof(rdata), 1) && !rdata[0];
#ifdef CDVD_VARIANT_XOSD
	CpuResumeIntr(state);
#endif
	if ( rdata[0] )
	{
#ifdef CDVD_VARIANT_XOSD
		if ( !param )
		{
			g_cdvdman_istruct.m_medium_removal_state = 0;
			g_cdvdman_istruct.m_atapi_disk_ejected = 0;
		}
#endif
		vDelayThread(11000);
	}
	return rdata[0];
}

static int cdvdman_scmd_sender_3B(int arg1)
{
	char wdata[1];
	char rdata[1];

	if ( g_cdvdman_minver_x_model_15 && arg1 == 1 )
	{
		return 1;
	}
	g_cdvdman_iocache = 0;
	wdata[0] = arg1;
	if ( set_prev_command(0x3B, wdata, sizeof(wdata), rdata, sizeof(rdata), 1) && !rdata[0] )
	{
		vDelayThread(11000);
		return 1;
	}
	return 0;
}

#ifdef CDVD_VARIANT_DNAS
int sceCdReadDiskID(unsigned int *id)
{
	sceCdRMode rmode;
	char sectbuf[0x800];
	u32 efbits;
	USE_DEV5_MMIO_HWPORT();

	// The following was inlined
	memset(id, 0, 5);
#ifdef CDVD_VARIANT_XOSD
	switch ( get_disk_type_ex() )
#else
	switch ( sceCdGetDiskType() )
#endif
	{
		case SCECdPS2CD:
		case SCECdPS2CDDA:
		case SCECdPS2DVD:
			break;
		default:
			return 0;
	}
	rmode.spindlctrl = 0x12;
	rmode.datapattern = SCECdSecS2048;
	rmode.trycount = 0;
	sceCdRead0(0x4B, 1, sectbuf, &rmode, 0, 0);
	sceCdSync(3);
	if ( !cdvdman_ncmd_sender_0C(0, 0, 0x4B) )
	{
		return 0;
	}
	sceCdSync(3);
	if ( g_cdvdman_istruct.m_last_error != SCECdErNO )
	{
		return 0;
	}
	WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
	if ( !(dev5_mmio_hwport->m_dev5_reg_038 & 4) )
	{
		vSetEventFlag(g_scmd_evfid, 1);
		return 0;
	}
	*(u8 *)id = dev5_mmio_hwport->m_dev5_reg_030 ^ dev5_mmio_hwport->m_dev5_reg_039;
	*((u8 *)id + 1) = dev5_mmio_hwport->m_dev5_reg_031 ^ dev5_mmio_hwport->m_dev5_reg_039;
	*((u8 *)id + 2) = dev5_mmio_hwport->m_dev5_reg_032 ^ dev5_mmio_hwport->m_dev5_reg_039;
	*((u8 *)id + 3) = dev5_mmio_hwport->m_dev5_reg_033 ^ dev5_mmio_hwport->m_dev5_reg_039;
	*((u8 *)id + 4) = dev5_mmio_hwport->m_dev5_reg_034 ^ dev5_mmio_hwport->m_dev5_reg_039;
	vSetEventFlag(g_scmd_evfid, 1);
	return 1;
}

int sceCdDoesUniqueKeyExist(u32 *status)
{
	int is_cd;
	u8 dev5_reg_038;
	sceCdRMode rmode;
	char ndata[7];
	int state;
	u32 efbits;
	USE_DEV5_MMIO_HWPORT();

	if ( !g_cdvdman_istruct.m_cd_inited )
	{
		*status = SCECdErCUD;
		return 0;
	}
	*status = SCECdErNO;
#ifdef CDVD_VARIANT_XOSD
	switch ( get_disk_type_ex() )
#else
	switch ( sceCdGetDiskType() )
#endif
	{
		case SCECdPS2CD:
		case SCECdPS2CDDA:
			is_cd = 1;
			break;
		case SCECdPS2DVD:
			is_cd = 0;
			break;
		default:
			*status = SCECdErCUD;
			return 0;
	}
	CpuSuspendIntr(&state);
	if ( g_cdvdman_istruct.m_stream_flag || g_cdvdman_istruct.m_read2_flag )
	{
		*status = SCECdErREADCF;
		CpuResumeIntr(state);
		return 0;
	}
	if ( (sceCdStatus() & SCECdStatSpin) )
	{
		CpuResumeIntr(state);
	}
	else
	{
		dev5_mmio_hwport->m_dev5_reg_007 = 1;
		CpuResumeIntr(state);
		sceCdSync(3);
	}
	CpuSuspendIntr(&state);
	rmode.spindlctrl = 0x12;
	rmode.datapattern = SCECdSecS2048;
	rmode.trycount = 0;
	if ( is_cd )
	{
		unsigned int i;

		for ( i = 0; i < 20; i += 1 )
		{
			sceCdRead0(0x4B + (0x10 * i), 0x10, g_cdvdman_ptoc, &rmode, 0, 0);
			CpuResumeIntr(state);
			sceCdSync(3);
			CpuSuspendIntr(&state);
		}
		CpuResumeIntr(state);
	}
	else
	{
		sceCdRead0(0x4B, 1, g_cdvdman_ptoc, &rmode, 0, 0);
		CpuResumeIntr(state);
		sceCdSync(3);
	}
	WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
	CpuSuspendIntr(&state);
	if ( g_cdvdman_istruct.m_stream_flag || g_cdvdman_istruct.m_read2_flag )
	{
		*status = SCECdErREADCF;
		CpuResumeIntr(state);
		vSetEventFlag(g_scmd_evfid, 1);
		return 0;
	}
	// The following was inlined
	memset(ndata, 0, sizeof(ndata));
	ndata[3] = 0x4B;
	if ( cdvdman_send_ncmd(0x0C, ndata, sizeof(ndata), 0, 0, 1) < 0 )
	{
		*status = SCECdErREADCF;
		CpuResumeIntr(state);
		vSetEventFlag(g_scmd_evfid, 1);
		return 0;
	}
	CpuResumeIntr(state);
	sceCdSync(3);
	if ( g_cdvdman_istruct.m_last_error != SCECdErNO )
	{
		*status = (u8)g_cdvdman_istruct.m_last_error;
		vSetEventFlag(g_scmd_evfid, 1);
		return 0;
	}
	dev5_reg_038 = dev5_mmio_hwport->m_dev5_reg_038;
	vSetEventFlag(g_scmd_evfid, 1);
	return (dev5_reg_038 & 5) == 5;
}
#endif

#ifdef CDVD_VARIANT_XOSD
int sceCdDeobfuscateUsingUniqueKey(u8 *buffer, unsigned int shiftval, int xorval, u32 *status)
{
	int is_cd;
	int retval;
	int i;
	u8 xor_val_1_stk;
	u8 xor_val_2_stk;
	u8 xor_val_3_stk;
	u8 xor_val_4_stk;
	u8 xor_val_1_xstk;
	u8 xor_val_2_xstk;
	char xorarea[16];
	char ndata[7];
	sceCdRMode rmode;
	int state;
	u32 efbits;
	USE_DEV5_MMIO_HWPORT();

	memset(xorarea, 0, sizeof(xorarea));
	memset(buffer, 0, 16);
	*status = SCECdErNO;
	if ( shiftval >= 8 || xorval < 0 || xorval >= 256 )
	{
		*status = SCECdErREADCF;
		return 0;
	}
	if ( !g_cdvdman_istruct.m_cd_inited )
	{
		*status = SCECdErCUD;
		return 0;
	}
	switch ( get_disk_type_ex() )
	{
		case SCECdPS2CD:
		case SCECdPS2CDDA:
			is_cd = 1;
			break;
		case SCECdPS2DVD:
			is_cd = 0;
			break;
		default:
			*status = SCECdErCUD;
			return 0;
	}
	CpuSuspendIntr(&state);
	if ( g_cdvdman_istruct.m_stream_flag || g_cdvdman_istruct.m_read2_flag )
	{
		*status = SCECdErREADCF;
		CpuResumeIntr(state);
		return 0;
	}
	if ( !(sceCdStatus() & SCECdStatSpin) )
	{
		dev5_mmio_hwport->m_dev5_reg_007 = 1;
		CpuResumeIntr(state);
		sceCdSync(3);
		CpuSuspendIntr(&state);
	}
	if ( g_cdvdman_istruct.m_stream_flag || g_cdvdman_istruct.m_read2_flag )
	{
		*status = SCECdErREADCF;
		CpuResumeIntr(state);
		return 0;
	}
	rmode.spindlctrl = 0x12;
	rmode.datapattern = SCECdSecS2048;
	rmode.trycount = 0;
	if ( is_cd )
	{
		for ( i = 0; i < 0x14; i += 1 )
		{
			sceCdRead0(75 + 16 * i, 0x10, g_cdvdman_ptoc, &rmode, 0, 0);
			CpuResumeIntr(state);
			sceCdSync(3);
			CpuSuspendIntr(&state);
		}
		CpuResumeIntr(state);
	}
	else
	{
		sceCdRead0(0x4B, 1, g_cdvdman_ptoc, &rmode, 0, 0);
		CpuResumeIntr(state);
		sceCdSync(3);
	}
	WaitEventFlag(g_scmd_evfid, 1, 0, &efbits);
	CpuSuspendIntr(&state);
	if ( g_cdvdman_istruct.m_stream_flag || g_cdvdman_istruct.m_read2_flag )
	{
		*status = SCECdErREADCF;
		CpuResumeIntr(state);
		vSetEventFlag(g_scmd_evfid, 1);
		return 0;
	}
	// The following was inlined
	memset(ndata, 0, sizeof(ndata));
	ndata[3] = 0x4B;
	if ( cdvdman_send_ncmd(0x0C, ndata, sizeof(ndata), 0, 0, 1) < 0 )
	{
		*status = SCECdErREADCF;
		CpuResumeIntr(state);
		vSetEventFlag(g_scmd_evfid, 1);
		return 0;
	}
	CpuResumeIntr(state);
	sceCdSync(3);
	if ( g_cdvdman_istruct.m_last_error != SCECdErNO )
	{
		*status = (u8)g_cdvdman_istruct.m_last_error;
		vSetEventFlag(g_scmd_evfid, 1);
		return 0;
	}
	retval = 0;
	xor_val_2_xstk = 0;
	xor_val_1_xstk = 0;
	xor_val_4_stk = 0;
	xor_val_3_stk = 0;
	xor_val_2_stk = 0;
	xor_val_1_stk = 0;
	if ( (dev5_mmio_hwport->m_dev5_reg_038 & 5) == 5 )
	{
		u8 dev5_reg_020;
		u8 dev5_reg_039;
		u8 dev5_reg_022;
		u8 reg_039_tmp1;

		dev5_reg_020 = dev5_mmio_hwport->m_dev5_reg_020;
		dev5_reg_039 = dev5_mmio_hwport->m_dev5_reg_039;
		xor_val_1_stk = dev5_reg_020 ^ dev5_reg_039;
		xor_val_2_stk = dev5_reg_039 ^ dev5_mmio_hwport->m_dev5_reg_021;
		dev5_reg_022 = dev5_mmio_hwport->m_dev5_reg_022;
		reg_039_tmp1 = dev5_mmio_hwport->m_dev5_reg_039;
		xor_val_3_stk = dev5_reg_022 ^ reg_039_tmp1;
		xor_val_4_stk = reg_039_tmp1 ^ dev5_mmio_hwport->m_dev5_reg_023;
		xor_val_1_xstk = dev5_mmio_hwport->m_dev5_reg_024 ^ dev5_mmio_hwport->m_dev5_reg_039;
		xor_val_2_xstk = dev5_mmio_hwport->m_dev5_reg_039 ^ dev5_mmio_hwport->m_dev5_reg_034;
		retval = 1;
	}
	vSetEventFlag(g_scmd_evfid, 1);
	xorarea[0] = (xor_val_4_stk >> 4) | ((xor_val_1_xstk << 4) & 0x70);
	xorarea[1] = (xor_val_3_stk >> 5) | ((xor_val_4_stk << 3) & 0x78);
	xorarea[2] = (xor_val_2_stk >> 6) | ((xor_val_3_stk << 2) & 0x7C);
	xorarea[3] = (xor_val_1_stk >> 7) | ((xor_val_2_stk << 1) & 0x7E);
	sprintf(&xorarea[4], "%d", ((xor_val_1_stk & 0x7F) << 10) | (xor_val_1_xstk >> 3) | ((xor_val_2_xstk >> 3) << 5));
	for ( i = 0; i < 9; i += 1 )
	{
		xorarea[i] = xorarea[i] ^ xorval;
	}
	if ( shiftval )
	{
		for ( i = 0; i < 9; i += 1 )
		{
			buffer[i] = (xorarea[i] << shiftval) | ((int)(u8)xorarea[i ? i - 1 : 8] >> (8 - shiftval));
		}
	}
	else
	{
		// The following was inlined
		memcpy(buffer, xorarea, sizeof(xorarea));
	}
	return retval;
}
#endif

#ifdef CDVD_VARIANT_OSD
int sceCdReadKey(u8 arg1, u8 arg2, unsigned int command, u8 *key)
{
	int i;
	u8 dev5_reg_038;
	sceCdRMode rmode;
	char sectorbuf[0x800];
	u32 efbits;
	USE_DEV5_MMIO_HWPORT();

	if ( cdvdman_isdvd() && !arg1 )
	{
		if ( !DvdDual_infochk() )
		{
			return 0;
		}
		command = sceCdLsnDualChg(command);
	}
	rmode.spindlctrl = 0x12;
	rmode.datapattern = SCECdSecS2048;
	rmode.trycount = 0;
	sceCdRead0(command, 1, sectorbuf, &rmode, 0, 0);
	sceCdSync(3);
	for ( i = 0; i < 300; i += 1 )
	{
		if ( !cdvdman_ncmd_sender_0C(arg1, arg2, command + i) )
		{
			return 0;
		}
		sceCdSync(3);
		if ( g_cdvdman_istruct.m_last_error == SCECdErNO )
		{
			break;
		}
	}
	WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
	// The following was inlined
	memset(key, 0, 16);
	dev5_reg_038 = dev5_mmio_hwport->m_dev5_reg_038;
	if ( (dev5_reg_038 & 1) )
	{
		key[0] = dev5_mmio_hwport->m_dev5_reg_020 ^ dev5_mmio_hwport->m_dev5_reg_039;
		key[1] = dev5_mmio_hwport->m_dev5_reg_021 ^ dev5_mmio_hwport->m_dev5_reg_039;
		key[2] = dev5_mmio_hwport->m_dev5_reg_022 ^ dev5_mmio_hwport->m_dev5_reg_039;
		key[3] = dev5_mmio_hwport->m_dev5_reg_023 ^ dev5_mmio_hwport->m_dev5_reg_039;
		key[4] = dev5_mmio_hwport->m_dev5_reg_024 ^ dev5_mmio_hwport->m_dev5_reg_039;
	}
	if ( (dev5_reg_038 & 2) )
	{
		key[5] = dev5_mmio_hwport->m_dev5_reg_028 ^ dev5_mmio_hwport->m_dev5_reg_039;
		key[6] = dev5_mmio_hwport->m_dev5_reg_029 ^ dev5_mmio_hwport->m_dev5_reg_039;
		key[7] = dev5_mmio_hwport->m_dev5_reg_02A ^ dev5_mmio_hwport->m_dev5_reg_039;
		key[8] = dev5_mmio_hwport->m_dev5_reg_02B ^ dev5_mmio_hwport->m_dev5_reg_039;
		key[9] = dev5_mmio_hwport->m_dev5_reg_02C ^ dev5_mmio_hwport->m_dev5_reg_039;
	}
	if ( (dev5_reg_038 & 4) )
	{
		key[10] = dev5_mmio_hwport->m_dev5_reg_030 ^ dev5_mmio_hwport->m_dev5_reg_039;
		key[11] = dev5_mmio_hwport->m_dev5_reg_031 ^ dev5_mmio_hwport->m_dev5_reg_039;
		key[12] = dev5_mmio_hwport->m_dev5_reg_032 ^ dev5_mmio_hwport->m_dev5_reg_039;
		key[13] = dev5_mmio_hwport->m_dev5_reg_033 ^ dev5_mmio_hwport->m_dev5_reg_039;
		key[14] = dev5_mmio_hwport->m_dev5_reg_034 ^ dev5_mmio_hwport->m_dev5_reg_039;
	}
	key[15] = dev5_reg_038 & 7;
	vSetEventFlag(g_scmd_evfid, 1);
	return 1;
}
#endif

#ifdef CDVD_VARIANT_DNAS
static int cdvdman_ncmd_sender_0C(int arg1, u32 arg2, u32 arg3)
{
	char ndata[7];

	ndata[1] = !!arg2;
	ndata[0] = arg1;
	ndata[2] = !!(arg2 >> 8);
	*(u32 *)&ndata[3] = !arg1 ? arg3 : 0;
	return cdvdman_send_ncmd(0x0C, ndata, sizeof(ndata), 0, 0, 1) >= 0;
}
#endif

int sceCdDecSet(u8 enable_xor, u8 enable_shift, u8 shiftval)
{
#ifdef CDVD_VARIANT_DNAS
	USE_DEV5_MMIO_HWPORT();

	g_cdvdman_cd36key = enable_shift | shiftval;
	dev5_mmio_hwport->m_dev5_reg_03A = ((shiftval & 7) << 4) | ((!!enable_xor) << 1) | (!!enable_shift);
#else
	(void)enable_xor;
	(void)enable_shift;
	(void)shiftval;
#endif
	return 1;
}

#ifdef CDVD_VARIANT_OSD
int sceCdOpenConfig(int block, int mode, int NumBlocks, u32 *status)
{
	int retval;
#ifdef CDVD_VARIANT_XOSD
	u32 efbits;
#endif
	char wdata[3];

#ifdef CDVD_VARIANT_XOSD
	if ( vPollEventFlag(g_scmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND )
	{
		return 0;
	}
#endif
	DelayThread(16000);
	*status = SCECdErNO;
	wdata[0] = (u8)mode;
	wdata[1] = (u8)block;
	wdata[2] = (u8)NumBlocks;
	retval = set_prev_command(0x40, wdata, sizeof(wdata), (char *)status, 1, 0);
#ifdef CDVD_VARIANT_XOSD
	if ( retval == 1 )
#endif
	{
		g_cdvdman_config_numblocks = NumBlocks;
	}
#ifdef CDVD_VARIANT_XOSD
	vSetEventFlag(g_scmd_evfid, 1);
#endif
	return retval;
}

// cppcheck-suppress funcArgNamesDifferent
int sceCdCloseConfig(u32 *status)
{
	int retval;

	*status = SCECdErNO;
	DelayThread(16000);
	retval = set_prev_command(0x43, NULL, 0, (char *)status, 1, 1);
	g_cdvdman_config_numblocks = 0;
	return retval;
}

static int read_config_process(char *outval, u32 *status)
{
	int retval;
	char rdata[16];

	DelayThread(2000);
	retval = set_prev_command(0x41, NULL, 0, rdata, sizeof(rdata), 0);
	*status = (u8)(rdata[14] + rdata[13] + rdata[12] + rdata[11] + rdata[10] + rdata[9] + rdata[8] + rdata[7] + rdata[6]
								 + rdata[5] + rdata[4] + rdata[3] + rdata[2] + rdata[0] + rdata[1])
				 != (u8)rdata[15];
	memcpy(outval, rdata, sizeof(rdata) - 1);
	return retval;
}

// cppcheck-suppress funcArgNamesDifferent
int sceCdReadConfig(void *buffer, u32 *status)
{
	int retval;
	u32 efbits;

	if ( vPollEventFlag(g_scmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND )
	{
		return 0;
	}
	for ( retval = 0; retval < g_cdvdman_config_numblocks; retval += 1 )
	{
		if ( !read_config_process((char *)buffer + 15 * retval, status) )
		{
			VERBOSE_KPRINTF(1, "RC_fail Command busy\n");
			break;
		}
		if ( *status )
		{
			VERBOSE_KPRINTF(1, "RC_fail status: 0x%02x\n", *status);
			break;
		}
	}
	vSetEventFlag(g_scmd_evfid, 1);
	return retval;
}

static int write_config_process(const char *inval, u32 *status)
{
	char wdata[16];

	DelayThread(2000);
	*status = SCECdErNO;
	memcpy(wdata, inval, 15);
	wdata[15] = wdata[14] + wdata[13] + wdata[12] + wdata[11] + wdata[10] + wdata[9] + wdata[8] + wdata[7] + wdata[6]
						+ wdata[5] + wdata[4] + wdata[3] + wdata[2] + wdata[0] + wdata[1];
	return set_prev_command(0x42, wdata, sizeof(wdata), (char *)status, 1, 0);
}

// cppcheck-suppress funcArgNamesDifferent
int sceCdWriteConfig(const void *buffer, u32 *status)
{
	int retval;
	u32 efbits;

	if ( vPollEventFlag(g_scmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND )
	{
		return 0;
	}
	for ( retval = 0; retval < g_cdvdman_config_numblocks; retval += 1 )
	{
		if ( !write_config_process((char *)buffer + 15 * retval, status) )
		{
			VERBOSE_KPRINTF(1, "WC_fail Command busy\n");
			break;
		}
		if ( *status )
		{
			VERBOSE_KPRINTF(1, "WC_fail status: 0x%02x\n", *status);
			break;
		}
	}
	vSetEventFlag(g_scmd_evfid, 1);
	return retval;
}

int sceCdSetHDMode(u32 mode)
{
	char wdata[1];
	char rdata[1];

	wdata[0] = mode;
	return set_prev_command(0x0C, wdata, sizeof(wdata), rdata, sizeof(rdata), 1) && !rdata[0];
}
#endif

#ifdef CDVD_VARIANT_XOSD
// cppcheck-suppress funcArgNamesDifferent
int sceCdXLEDCtl(u8 arg1, u8 arg2, u32 *param, u32 *status)
{
	int retval;
	char wdata[2];
	char rdata[2];

	*param = 0;
	if ( !g_cdvdman_minver_50600 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	wdata[0] = arg1;
	wdata[1] = arg2;
	retval = set_prev_command(0x2D, wdata, sizeof(wdata), rdata, sizeof(rdata), 1);
	if ( !retval )
	{
		return 0;
	}
	*status = (u8)rdata[0];
	*param = (u8)rdata[1];
	return retval;
}
#endif

#ifdef CDVD_VARIANT_XOSD
// cppcheck-suppress funcArgNamesDifferent
int sceCdBuzzerCtl(u32 *status)
{
	if ( !g_cdvdman_minver_50600 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	return set_prev_command(0x2E, NULL, 0, (char *)status, 1, 1);
}
#endif

#ifdef CDVD_VARIANT_XOSD
int cdvdman_167_atapi2dragon(u8 *inbuf, u32 *status)
{
	if ( !g_cdvdman_minver_50600 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	return cdvdman_write_scmd_swap_dev5(0x2F, inbuf, 16, (char *)status, 1, 1);
}
#endif

#ifdef CDVD_VARIANT_XOSD
int cdvdman_169_dragon2atapi(u8 *outbuf, u32 *status)
{
	int i;
	int retval;
	char rdata[9];
	u32 efbits;
	char wdata[1];

	if ( !g_cdvdman_minver_50600 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	if ( vPollEventFlag(g_scmd_evfid, 1, WEF_AND | WEF_CLEAR, &efbits) == KE_EVF_COND )
	{
		return 0;
	}
	memset(outbuf, 0, 16);
	if ( *status )
	{
		vSetEventFlag(g_scmd_evfid, 1);
		return 1;
	}
	retval = 0;
	for ( i = 0; i < 2 && !rdata[0]; i += 1 )
	{
		wdata[0] = i * 8;
		retval = 0;
		while ( !retval )
		{
			DelayThread(2000);
			retval = cdvdman_write_scmd_swap_dev5(0x30, wdata, sizeof(wdata), rdata, sizeof(rdata), 0);
			memcpy(&outbuf[i * 8], &rdata[1], sizeof(rdata) - 1);
			*status = rdata[0];
		}
	}
	return retval;
}
#endif

#ifdef CDVD_VARIANT_XOSD
// cppcheck-suppress funcArgNamesDifferent
int sceCdXBSPowerCtl(u8 arg1, u8 arg2, u32 *param, u32 *status)
{
	int retval;
	char wdata[2];
	char rdata[2];

	*param = 0;
	if ( !g_cdvdman_minver_50600 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	wdata[0] = arg1;
	wdata[1] = arg2;
	retval = set_prev_command(0x2C, wdata, sizeof(wdata), rdata, sizeof(rdata), 1);
	if ( !retval )
	{
		return 0;
	}
	*status = (u8)rdata[0];
	*param = (u8)rdata[1];
	return retval;
}
#endif

#ifdef CDVD_VARIANT_XOSD
// cppcheck-suppress funcArgNamesDifferent
int sceCdSetMediumRemoval(u8 wanted_val, u32 *status)
{
	int retval;
	int saved_medium_removal_state;
	char wdata[1];
	int state;

	if ( !g_cdvdman_minver_50600 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	CpuSuspendIntr(&state);
	saved_medium_removal_state = g_cdvdman_istruct.m_medium_removal_state;
	g_cdvdman_istruct.m_medium_removal_state = (u8)wanted_val;
	wdata[0] = wanted_val;
	retval = set_prev_command(0x31, wdata, sizeof(wdata), (char *)status, 1, 1);
	if ( !retval || *status )
	{
		g_cdvdman_istruct.m_medium_removal_state = saved_medium_removal_state;
	}
	CpuResumeIntr(state);
	return retval;
}
#endif

#ifdef CDVD_VARIANT_XOSD
// cppcheck-suppress funcArgNamesDifferent
int sceCdGetMediumRemoval(u32 *param, u32 *status)
{
	int retval;
	char rdata[2];

	*param = 0;
	if ( !g_cdvdman_minver_50600 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	retval = set_prev_command(0x32, NULL, 0, rdata, sizeof(rdata), 1);
	if ( !retval )
	{
		return 0;
	}
	*status = (u8)rdata[0];
	*param = (u8)rdata[1];
	if ( !rdata[0] )
	{
		g_cdvdman_istruct.m_medium_removal_state = *param;
	}
	return retval;
}
#endif

#ifdef CDVD_VARIANT_XOSD
// cppcheck-suppress funcArgNamesDifferent
int sceCdXDVRPReset(u8 arg1, u32 *status)
{
	char wdata[1];

	if ( !g_cdvdman_minver_50600 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	wdata[0] = arg1;
	return set_prev_command(0x33, wdata, sizeof(wdata), (char *)status, 1, 1);
}
#endif

#ifdef CDVD_VARIANT_XOSD
static int sc_ffffffd8(u32 *status)
{
	int retval;

	if ( !g_cdvdman_minver_50600 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	retval = set_prev_command(0x34, NULL, 0, (char *)status, 1, 1);
	if ( !retval )
	{
		return 0;
	}
	g_cdvdman_istruct.m_atapi_disk_ejected = 0;
	return retval;
}
#endif

#ifdef CDVD_VARIANT_XOSD
#ifdef DEAD_CODE
int cdvdman_scmd_sender_35(u32 *param, u32 *status)
{
	int retval;
	char rdata[2];

	*param = 0;
	if ( !g_cdvdman_minver_50600 )
	{
		*status = 256;
		return 1;
	}
	*status = SCECdErNO;
	retval = set_prev_command(0x35, NULL, 0, rdata, sizeof(rdata), 1);
	if ( !retval )
	{
		return 0;
	}
	*status = (u8)rdata[0];
	*param = (u8)rdata[1];
	return retval;
}
#endif
#endif
