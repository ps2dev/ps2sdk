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

#include <cdvd-ioctl.h>
#include <kerr.h>
#include <libcdvd-rpc.h>

IRX_ID("cdvd_ee_driver", 2, 38);
// Based on the module from SCE SDK 3.1.0.

extern struct irx_export_table _exp_cdvdfsv;

static int cdvdfsv_init(void);
static void cdvdfsv_main_th(void *arg);
extern int *cdvdfsv_dummyentry(int arg1);
static void cdvdfsv_parseargs(int ac, char **av);
static void cdvdfsv_poffloop(void);
static void cdvdfsv_rpc1_th(void *arg);
static void cdvdfsv_rpc3_th(void *arg);
static void cdvdfsv_rpc2_th(void *arg);
#ifdef CDVD_VARIANT_OSD
static void cdvdfsv_rpc4_th(void *arg);
static void cdvdfsv_rpc5_th(void *arg);
#endif
extern unsigned int optimized_memcpy(char *dst, const char *src, unsigned int n);

static int g_cdvdfsv_def_pri = 81;
static int g_verbose_level = 0;
static int g_cdvdfsv_spinctl = -1;
// Was non-const in XOSD
static const int g_cdvdfsv_sectors = 0x10;
// g_cdvdfsv_sectors_cdda is same as g_cdvdfsv_sectors; separated here for reference
#ifdef CDVD_VARIANT_OSD
static const int g_cdvdfsv_sectors_cdda = 0x10;
#else
static const int g_cdvdfsv_sectors_cdda = 8;
#endif
// Was non-const in XOSD
static const int g_cdvdfsv_r2retry_initval = 3;
static int g_cdvdfsv_plbreak = 0;
static int g_cdvdfsv_nopocm = 0;
static int g_cdvdfsv_rpc5flg = 0;
static int g_cdvdfsv_rpc3flg = 0;
#if 0
static iop_library_t g_modload_libinfo = { NULL, NULL, 256, 0, "modload", { NULL } };
#endif
static int g_cdvdfsv_r2retry = 0;
static int g_cdvdfsv_r2count = 0;
static int g_cdvdfsv_sid_err_recover_cnt = 0;
static int g_cdvdfsv_err_count = 0;
static char *g_cdvdfsv_fsvrbuf[2];
static char *g_cdvdfsv_rtocbuf;
static SifDmaTransfer_t g_cdvdfsv_fssdd;
static SifDmaTransfer_t g_cdvdfsv_iomrsdd;
static SifDmaTransfer_t g_cdvdfsv_rdp2sdd;
static SifDmaTransfer_t g_cdvdfsv_multi_dmat[16];
static sceCdRMode g_cdvdfsv_rmodeee;
static SifDmaTransfer_t g_cdvdfsv_datasdd;
static SifDmaTransfer_t g_cdvdfsv_eerpsdd;
static SifDmaTransfer_t g_cdvdfsv_chrdsdd;
#ifdef CDVD_VARIANT_OSD
static SifDmaTransfer_t g_cdvdfsv_readdvdv_dmat;
#endif
static SifDmaTransfer_t g_cdvdfsv_eereadfull_dma1;
static SifDmaTransfer_t g_cdvdfsv_eereadfull_dma2;
static SifDmaTransfer_t g_cdvdfsv_rtocsdd;
static iop_sys_clock_t g_cdvdfsv_read_timeout;
static int g_cdvdman_intr_evfid;
static int g_scmd_evfid;
#ifdef CDVD_VARIANT_OSD
static int g_cdvdfsv_thids[6];
#else
static int g_cdvdfsv_thids[4];
#endif
static cdvdman_internal_struct_t *g_cdvdman_istruct_ptr;
static cdvdfsv_rpc1_outpacket_t g_cdvdfsv_initres;
static cdvdfsv_unaligned_data_outpacket_t g_cdvdfsv_eereadx;
static SifRpcDataQueue_t g_rpc_qdata2;
static SifRpcDataQueue_t g_rpc_qdata1;
static SifRpcDataQueue_t g_rpc_qdata3;
static SifRpcServerData_t g_rpc_sdata1;
static SifRpcServerData_t g_rpc_sdata4;
static SifRpcServerData_t g_rpc_sdata5;
static SifRpcServerData_t g_rpc_sdata2;
static SifRpcServerData_t g_rpc_sdata6;
static SifRpcServerData_t g_rpc_sdata3;
static cdvdfsv_rpc4_outpacket_t g_cdvdfsv_srchres;
static int g_cdvdfsv_readpos;
static int g_cdvdfsv_rderror;
static cdvdfsv_rpc2_outpacket_t g_diskready_res;
static cdvdfsv_rpc5_outpacket_t g_crr;
static cdvdfsv_rpc3_outpacket_t g_outbuf;
static int g_rpc_buffer3[260];
static int g_rpc_buffer5[256];
static int g_rpc_buffer1[4];
static int g_rpc_buffer4[76];
static int g_rpc_buffer2[4];
static int g_rpc_buffer2[4];
#ifdef CDVD_VARIANT_OSD
static char g_extra_fsvrbuf[40256];
#endif

static int cdvdfsv_checkdmastat(int trid)
{
	int retval;
	int state;

	if ( QueryIntrContext() )
		return sceSifDmaStat(trid);
	CpuSuspendIntr(&state);
	retval = sceSifDmaStat(trid);
	CpuResumeIntr(state);
	return retval;
}

static int cdvdfsv_cleanuprpc(void)
{
	unsigned int i;

	sceSifRemoveRpc(&g_rpc_sdata1, &g_rpc_qdata1);
	sceSifRemoveRpc(&g_rpc_sdata2, &g_rpc_qdata1);
	sceSifRemoveRpc(&g_rpc_sdata3, &g_rpc_qdata1);
	sceSifRemoveRpc(&g_rpc_sdata6, &g_rpc_qdata3);
	sceSifRemoveRpc(&g_rpc_sdata4, &g_rpc_qdata2);
	sceSifRemoveRpc(&g_rpc_sdata5, &g_rpc_qdata2);
	sceSifRemoveRpcQueue(&g_rpc_qdata1);
	sceSifRemoveRpcQueue(&g_rpc_qdata2);
	sceSifRemoveRpcQueue(&g_rpc_qdata3);
	g_cdvdfsv_nopocm = 1;
	g_cdvdfsv_plbreak = 1;
	for ( i = 0; i < (sizeof(g_cdvdfsv_thids) / sizeof(g_cdvdfsv_thids[0])); i += 1 )
	{
		TerminateThread(g_cdvdfsv_thids[i]);
		DeleteThread(g_cdvdfsv_thids[i]);
	}
	return 1;
}

int _start(int ac, char *av[], void *startaddr, ModuleInfo_t *mi)
{
#if 0
	const u16 *LibraryEntryTable;
#endif
	int state;

	(void)startaddr;

	if ( ac < 0 )
	{
		int error_code;

		// cppcheck-suppress knownConditionTrueFalse
		if ( g_cdvdfsv_rpc5flg || g_cdvdfsv_rpc3flg || !cdvdfsv_cleanuprpc() )
		{
			return MODULE_REMOVABLE_END;
		}
		CpuSuspendIntr(&state);
		error_code = ReleaseLibraryEntries(&_exp_cdvdfsv);
		CpuResumeIntr(state);
		if ( error_code && error_code != KE_LIBRARY_NOTFOUND )
		{
			KPRINTF("ReleaseLibraryEntries Error code %d\n", error_code);
			return MODULE_REMOVABLE_END;
		}
		return MODULE_NO_RESIDENT_END;
	}
	if ( RegisterLibraryEntries(&_exp_cdvdfsv) )
	{
		return MODULE_NO_RESIDENT_END;
	}
	g_cdvdfsv_fsvrbuf[0] = (char *)sceGetFsvRbuf();
#ifdef CDVD_VARIANT_OSD
	g_cdvdfsv_fsvrbuf[1] = &g_extra_fsvrbuf[0];
#endif
	g_cdvdfsv_rtocbuf = g_cdvdfsv_fsvrbuf[0];
	// g_cdvdfsv_sectors was set in XOSD variant
	// g_cdvdfsv_r2retry_initval was set in XOSD variant
	cdvdfsv_parseargs(ac, av);
	cdvdfsv_init();
#if 0
	CpuSuspendIntr(&state);
	LibraryEntryTable = (u16 *)QueryLibraryEntryTable(&g_modload_libinfo);
	CpuResumeIntr(state);
	if ( !LibraryEntryTable || (*(LibraryEntryTable - 6) < 0x104) )
	{
		KPRINTF("Warning cdvdfsv.irx: Unload function can't be used.\n");
		return MODULE_RESIDENT_END;
	}
	return MODULE_REMOVABLE_END;
#else
	if ( mi && ((mi->newflags & 2) != 0) )
		mi->newflags |= 0x10;
	return MODULE_RESIDENT_END;
#endif
}

static int cdvdfsv_init(void)
{
	const int *BootMode;
	iop_thread_t thparam;
	int scres;

	BootMode = QueryBootMode(3);
	if ( BootMode && (BootMode[1] & 2) )
	{
		PRINTF(" No cdvd driver \n");
		return 1;
	}
	sceCdSC(0xFFFFFFF2, (int *)&g_cdvdman_istruct_ptr);
	g_scmd_evfid = sceCdSC(0xFFFFFFE7, &scres);
	thparam.attr = TH_C;
	thparam.thread = cdvdfsv_main_th;
	thparam.stacksize = 0x800;
	thparam.option = 0;
	thparam.priority = g_cdvdfsv_def_pri - 1;
	g_cdvdfsv_thids[0] = CreateThread(&thparam);
	if ( g_cdvdfsv_thids[0] <= 0 )
	{
		return 1;
	}
	StartThread(g_cdvdfsv_thids[0], 0);
	return 0;
}

static void cdvdfsv_main_th(void *arg)
{
	iop_thread_t thparam1;
	iop_thread_t thparam2;

	(void)arg;

	if ( !sceSifCheckInit() )
		sceSifInit();
	sceSifInitRpc(0);
	PRINTF("cdvd driver module version 0.1.1 (C)SCEI\n");
	thparam2.thread = cdvdfsv_rpc1_th;
	thparam2.attr = TH_C;
	thparam2.stacksize = 0x1900;
	thparam2.option = 0;
	thparam2.priority = g_cdvdfsv_def_pri;
	g_cdvdfsv_thids[1] = CreateThread(&thparam2);
	StartThread(g_cdvdfsv_thids[1], 0);
	thparam1.attr = TH_C;
	thparam1.thread = cdvdfsv_rpc2_th;
	thparam1.stacksize = 0x1900;
	thparam1.option = 0;
	thparam1.priority = g_cdvdfsv_def_pri;
	g_cdvdfsv_thids[2] = CreateThread(&thparam1);
	StartThread(g_cdvdfsv_thids[2], 0);
	thparam1.thread = cdvdfsv_rpc3_th;
	thparam1.attr = TH_C;
	thparam1.stacksize = 0x800;
	thparam1.option = 0;
	thparam1.priority = g_cdvdfsv_def_pri;
	g_cdvdfsv_thids[3] = CreateThread(&thparam1);
	StartThread(g_cdvdfsv_thids[3], 0);
#ifdef CDVD_VARIANT_OSD
	thparam1.attr = TH_C;
	thparam1.thread = cdvdfsv_rpc4_th;
	thparam1.stacksize = 0x800;
	thparam1.option = 0;
	thparam1.priority = g_cdvdfsv_def_pri;
	g_cdvdfsv_thids[4] = CreateThread(&thparam1);
	StartThread(g_cdvdfsv_thids[4], 0);
	thparam1.attr = TH_C;
	thparam1.thread = cdvdfsv_rpc5_th;
	thparam1.stacksize = 0x800;
	thparam1.option = 0;
	thparam1.priority = g_cdvdfsv_def_pri;
	g_cdvdfsv_thids[5] = CreateThread(&thparam1);
	StartThread(g_cdvdfsv_thids[5], 0);
#endif
	cdvdfsv_poffloop();
	ExitDeleteThread();
}

int *cdvdfsv_dummyentry(int arg1)
{
	VERBOSE_PRINTF(1, "Dummy Entry Called\n");
	if ( arg1 != 128 )
		return 0;
	return &g_verbose_level;
}

static void cdvdfsv_parseargs(int ac, char **av)
{
	int i;

	g_cdvdfsv_def_pri = 81;
	for ( i = 1; i < ac; i += 1 )
	{
		if ( !strncmp(av[i], "thpri=", 6) )
		{
			g_cdvdfsv_def_pri = strtol(av[i] + 6, 0, 10);
			if ( (unsigned int)(g_cdvdfsv_def_pri - 9) >= 0x73 )
			{
				PRINTF("Cdvdfsv:thpri=%d Illegal priority\n", g_cdvdfsv_def_pri);
				g_cdvdfsv_def_pri = 81;
			}
			if ( g_cdvdfsv_def_pri == 9 )
				g_cdvdfsv_def_pri = 10;
		}
	}
}

int sceCdChangeThreadPriority(int priority)
{
	iop_thread_info_t thinfo;

	if ( (unsigned int)(priority - 9) >= 0x73 )
		return -403;
	if ( priority == 9 )
		priority = 10;
	ReferThreadStatus(0, &thinfo);
	ChangeThreadPriority(0, 8);
	ChangeThreadPriority(g_cdvdfsv_thids[0], priority - 1);
	ChangeThreadPriority(g_cdvdfsv_thids[2], priority);
	ChangeThreadPriority(g_cdvdfsv_thids[1], priority);
	ChangeThreadPriority(g_cdvdfsv_thids[3], priority);
#ifdef CDVD_VARIANT_OSD
	ChangeThreadPriority(g_cdvdfsv_thids[4], priority);
	ChangeThreadPriority(g_cdvdfsv_thids[5], priority);
#endif
	return 0;
}

static void *cbrpc_rpc1_cdinit(int fno, void *buffer, int length)
{
	int scres_unused;

	(void)fno;
	(void)length;

	VERBOSE_PRINTF(1, "sceCdInit call\n");
	sceCdInit(((const cdvdfsv_rpc1_inpacket_t *)buffer)->m_mode);
	g_cdvdfsv_spinctl = -1;
	g_cdvdfsv_initres.m_debug_mode = g_verbose_level ? 254 : 0;
	g_cdvdfsv_initres.m_cdvdfsv_ver = (u16)_irx_id.v;
	g_cdvdfsv_initres.m_cdvdman_ver = sceCdSC(0xFFFFFFF7, &scres_unused);
	VERBOSE_PRINTF(1, "sceCdInit end\n");
	g_cdvdfsv_initres.m_retres = 1;
	return (void *)&g_cdvdfsv_initres;
}

static void cdvdfsv_rpc3_16_break(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	(void)inbuf;
	(void)buflen;

	VERBOSE_PRINTF(1, "sceCdAbort call\n");
	sceCdBreak();
	outbuf->m_retres = 1;
}

static void *cbrpc_rpc4_fscall(int fno, void *buffer, int length)
{
	int scres;
	int state;
	cdvdfsv_rpc4_inpacket_t *inbuf;

	(void)fno;

	inbuf = buffer;
	scres = 255;
	sceCdSC(0xFFFFFFF6, &scres);
	VERBOSE_PRINTF(1, "search file name %s call struct_siz %d\n", inbuf->m_pkt_sz12c.m_path, length);
	switch ( length )
	{
		case sizeof(inbuf->m_pkt_sz12c):
			g_cdvdfsv_srchres.m_retres =
				sceCdLayerSearchFile(&(inbuf->m_pkt_sz12c.m_fp), inbuf->m_pkt_sz12c.m_path, inbuf->m_pkt_sz12c.m_layer);
			g_cdvdfsv_fssdd.src = buffer;
			g_cdvdfsv_fssdd.dest = (void *)inbuf->m_pkt_sz12c.m_eedest;
			g_cdvdfsv_fssdd.size = sizeof(sceCdlFILE) + 4;
			break;
		case sizeof(inbuf->m_pkt_sz128):
			PRINTF("sceCdSearchFile: Called from Not_Dual_layer Version.\n");
			g_cdvdfsv_srchres.m_retres = sceCdSearchFile(&(inbuf->m_pkt_sz128.m_fp), inbuf->m_pkt_sz128.m_path);
			g_cdvdfsv_fssdd.src = buffer;
			g_cdvdfsv_fssdd.dest = (void *)inbuf->m_pkt_sz128.m_eedest;
			g_cdvdfsv_fssdd.size = sizeof(sceCdlFILE) + 4;
			break;
		default:
			PRINTF("Warning sceCdSearchFile: Called from Old liblary.\n");
			g_cdvdfsv_srchres.m_retres = sceCdSearchFile(&(inbuf->m_pkt_sz124.m_fp), inbuf->m_pkt_sz124.m_path);
			g_cdvdfsv_fssdd.src = buffer;
			g_cdvdfsv_fssdd.dest = (void *)inbuf->m_pkt_sz124.m_eedest;
			g_cdvdfsv_fssdd.size = sizeof(sceCdlFILE);
			break;
	}
	g_cdvdfsv_fssdd.attr = 0;
	while ( 1 )
	{
		int trid;

		CpuSuspendIntr(&state);
		trid = sceSifSetDma(&g_cdvdfsv_fssdd, 1);
		CpuResumeIntr(state);
		if ( trid )
			break;
		DelayThread(500);
	}
	scres = 0;
	sceCdSC(0xFFFFFFF6, &scres);
	return (void *)&g_cdvdfsv_srchres;
}

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

static void cdvdfsv_rpc5_0D_iopmread(const cdvdfsv_rpc5_inpacket_t *inbuf, int buflen, cdvdfsv_rpc5_outpacket_t *outbuf)
{
	int cmd_error;
	int trid;
	int scres_unused;
	int error_code;
	int state;

	(void)buflen;
	(void)outbuf;

	g_cdvdfsv_rderror = SCECdErREADCFR;
	g_cdvdfsv_read_timeout.hi = 0;
	g_cdvdfsv_read_timeout.lo = 0x9000 * sceCdSC(0xFFFFFFF1, &scres_unused);
	g_cdvdfsv_iomrsdd.src = &g_cdvdfsv_readpos;
	g_cdvdfsv_iomrsdd.size = sizeof(g_cdvdfsv_readpos);
	g_cdvdfsv_iomrsdd.attr = 0;
	g_cdvdfsv_iomrsdd.dest = (void *)inbuf->m_pkt_0D.m_eedest;
	VERBOSE_PRINTF(
		1,
		"sceCdReadIOPm addr= 0x%08x sector= %d\n",
		(unsigned int)(uiptr)(inbuf->m_pkt_0D.m_buf),
		(int)(inbuf->m_pkt_0D.m_sectors));
	cmd_error = sceCdRE(
		inbuf->m_pkt_0D.m_lbn, inbuf->m_pkt_0D.m_sectors, inbuf->m_pkt_0D.m_buf, (sceCdRMode *)&inbuf->m_pkt_0D.m_mode);
	while ( sceCdSync(1) )
	{
		g_cdvdfsv_readpos = sceCdGetReadPos();
		while ( 1 )
		{
			CpuSuspendIntr(&state);
			trid = sceSifSetDma(&g_cdvdfsv_iomrsdd, 1);
			CpuResumeIntr(state);
			if ( trid )
				break;
			DelayThread(500);
		}
		DelayThread(8000);
		while ( cdvdfsv_checkdmastat(trid) >= 0 )
			;
	}
	error_code = sceCdGetError();
	if ( error_code != SCECdErNO || !cmd_error )
	{
		if ( !cmd_error )
			sceCdSC(0xFFFFFFFE, &g_cdvdfsv_rderror);
		VERBOSE_PRINTF(1, "Read error code %x cmd error %d\n", error_code, cmd_error);
	}
}

static u8 cdvdfsv_syncdec(int flag, int xorkey, int arg2, u8 data)
{
	return flag ? (((data << (arg2 % 8)) | (data >> (8 - arg2 % 8))) ^ xorkey) : data;
}

static int cdvdfsv_cb_read(void)
{
	iSetEventFlag(g_cdvdman_intr_evfid, 0x20);
	return 0;
}

static int cdvdfsv_checksid(u32 lsn, u32 sectors, u32 ps2dvd, void *buf, int decflag, int decshift, u32 *syncdec_mask)
{
	int scret;
	u32 i;
	u32 readlsn;
	int syncdec;
	u8 syncdec_4;
	sceCdlLOCCD rpos;
	int scres;
	int ipi_emu;

	ipi_emu = 0;
	*syncdec_mask = 0;
	scret = decflag ? sceCdSC(0xFFFFFFE8, &scres) : 0;
	syncdec_4 = 0;
	for ( i = 0; i < sectors; i += 1 )
	{
		if ( ps2dvd )
		{
			syncdec = cdvdfsv_syncdec(decflag, scret, decshift, ((u8 *)buf + (i * 0x810))[3]);
			syncdec += cdvdfsv_syncdec(decflag, scret, decshift, ((u8 *)buf + (i * 0x810))[2]) << 8;
			syncdec += cdvdfsv_syncdec(decflag, scret, decshift, ((u8 *)buf + (i * 0x810))[1]) << 16;
			syncdec_4 = cdvdfsv_syncdec(decflag, scret, decshift, ((u8 *)buf + (i * 0x810))[0]);
			if ( i && !*syncdec_mask )
			{
				ipi_emu = syncdec_4 & 0xC;
			}
			else if ( !i )
			{
				*syncdec_mask = syncdec_4 & 0xC;
			}
			readlsn = syncdec - 0x30000;
			if (
				g_cdvdman_istruct_ptr->m_opo_or_para && (lsn + i) >= g_cdvdman_istruct_ptr->m_layer_1_lsn
				&& g_cdvdman_istruct_ptr->m_opo_or_para == 1 )
			{
				readlsn += g_cdvdman_istruct_ptr->m_layer_1_lsn;
			}
		}
		else
		{
			rpos.minute = cdvdfsv_syncdec(decflag, scret, decshift, ((u8 *)buf + (i * 0x924))[0]);
			rpos.second = cdvdfsv_syncdec(decflag, scret, decshift, ((u8 *)buf + (i * 0x924))[1]);
			rpos.sector = cdvdfsv_syncdec(decflag, scret, decshift, ((u8 *)buf + (i * 0x924))[2]);
			readlsn = sceCdPosToInt(&rpos);
		}
		if ( readlsn != (lsn + i) || ipi_emu )
		{
			// The following printf was modified for ioprp300x
			VERBOSE_PRINTF(
				1,
				"Read_EE Sector_ID error lsn= %d readlsn= %d layer= %d layer1_start %d ipi_emu %d\n",
				(int)(lsn + i),
				(int)readlsn,
				(syncdec_4 & 1),
				(int)(g_cdvdman_istruct_ptr->m_layer_1_lsn),
				ipi_emu);
			return 0;
		}
	}
	if ( *syncdec_mask )
	{
		VERBOSE_PRINTF(
			1, "Read_EE NO_Data_zone error lsn= %d layer= %d SecID %02x\n", (int)lsn, (syncdec_4 & 1), (int)(*syncdec_mask));
	}
	return 1;
}

static int readproc2(
	u32 lsn,
	u32 nsec,
	sceCdRMode *mode,
	u32 sector_size_selection,
	int do_multi_retries,
	int enable_dec_shift,
	int dec_shift,
	char *ee_addr,
	int fssift,
	int secsize,
	int dmasize,
	SifDmaTransfer_t *post_dmat)
{
	unsigned int i;
	int csec;
	int read_res_tmp;
	int trid;
	int j;
	int size_2;
	int sector_sizes[2];
	int error_code;
	int scres_unused;
	int state;
	u32 syncdec_mask;
	u32 chcr;
	int error_code_tmp;
	char *ee_addr_tmp;
	int dmasize_tmp;
	int csec_comm;
	int nsec_div_cdvdfsv_sectors;
	int retry_flag1;
	int retry_flag2;
	int sector_size;

	error_code_tmp = SCECdErNO;
	sector_sizes[0] = 0x924;
	sector_sizes[1] = 0x810;
	g_cdvdfsv_read_timeout.hi = 0;
	g_cdvdfsv_read_timeout.lo = 0x9000 * sceCdSC(0xFFFFFFF1, &scres_unused);
	g_cdvdfsv_rderror = SCECdErREADCF;
	g_cdvdfsv_r2retry = 0;
	g_cdvdfsv_r2count = 0;
	if ( secsize != 0x924 && !fssift )
	{
		for ( i = 0; i < (unsigned int)g_cdvdfsv_sectors; i += 1 )
		{
			g_cdvdfsv_multi_dmat[i].attr = 0;
			g_cdvdfsv_multi_dmat[i].size = secsize;
		}
	}
	// The following Kprintf was added for ioprp300x
	VERBOSE_KPRINTF(1, "lsn= %d nsec= %d ee_addr= %08x fssift= %d secsize= %d\n", lsn, nsec, ee_addr, fssift, secsize);
	sector_size = sector_sizes[sector_size_selection];
	while ( 1 )
	{
		while ( 1 )
		{
			csec = (nsec <= (u32)g_cdvdfsv_sectors) ? nsec : (u32)g_cdvdfsv_sectors;
#ifdef CDVD_VARIANT_OSD
			if ( !g_cdvdfsv_sectors )
				__builtin_trap();
#endif
			nsec_div_cdvdfsv_sectors = (nsec / g_cdvdfsv_sectors) + (!!((nsec & 0xF)));
			retry_flag2 = 0;
			ee_addr_tmp = ee_addr;
			dmasize_tmp = dmasize;
			g_cdvdman_istruct_ptr->m_dec_mode_set = 1;
			g_cdvdman_istruct_ptr->m_dec_mode_last_set = 0;
			CpuSuspendIntr(&state);
			if ( enable_dec_shift )
			{
				g_cdvdman_istruct_ptr->m_dec_shift = dec_shift;
				g_cdvdman_istruct_ptr->m_dec_state = 2;
			}
			if ( g_cdvdfsv_r2retry )
			{
				VERBOSE_KPRINTF(1, "Rty_Read\n");
				read_res_tmp = (sector_size_selection ? sceCdRV : sceCdRead0)(
					(lsn >= (u32)(6 * g_cdvdfsv_sectors)) ? (lsn - g_cdvdfsv_sectors * g_cdvdfsv_r2retry) :
																									(lsn + g_cdvdfsv_sectors * g_cdvdfsv_r2retry + 6 * g_cdvdfsv_sectors),
					g_cdvdfsv_sectors,
					&g_cdvdfsv_rtocbuf[0x1248],
					mode,
					0,
					0);
				CpuResumeIntr(state);
			}
			else
			{
				read_res_tmp = (sector_size_selection ? sceCdRV : sceCdRead0)(
					lsn, nsec, &g_cdvdfsv_rtocbuf[0x1248], mode, csec, cdvdfsv_cb_read);
				CpuResumeIntr(state);
				if ( read_res_tmp )
				{
					SetAlarm(&g_cdvdfsv_read_timeout, read_timeout_alarm_cb, &g_cdvdfsv_read_timeout);
					csec_comm = 0;
					retry_flag1 = 0;
					break;
				}
			}
			if ( !read_res_tmp )
			{
				g_cdvdman_istruct_ptr->m_dec_state = 0;
				g_cdvdman_istruct_ptr->m_dec_mode_set = 0;
				sceCdSC(0xFFFFFFFE, &g_cdvdfsv_rderror);
				return 0;
			}
			sceCdSync(3);
			g_cdvdfsv_r2retry -= 1;
		}
		for ( i = 0; (int)i < nsec_div_cdvdfsv_sectors; i += 1 )
		{
			sceCdSync(32);
			if ( g_cdvdman_istruct_ptr->m_dec_mode_last_set )
				break;
			if ( sceCdGetError() == SCECdErNO )
			{
				if ( cdvdfsv_checksid(
							 lsn + csec_comm,
							 csec,
							 sector_size_selection,
							 &g_cdvdfsv_rtocbuf[0x1248],
							 enable_dec_shift,
							 dec_shift,
							 &syncdec_mask) )
				{
					if ( do_multi_retries && syncdec_mask && !i )
					{
						retry_flag1 = 1;
						error_code_tmp = SCECdErIPI;
					}
				}
				else if ( do_multi_retries )
				{
					retry_flag2 = 1;
					retry_flag1 = 1;
				}
				if ( retry_flag1 || g_cdvdfsv_r2retry )
				{
				}
				else if ( secsize == 0x924 && !sector_size_selection )
				{
					if ( fssift )
					{
						if ( i )
						{
							optimized_memcpy(&g_cdvdfsv_rtocbuf[secsize], &g_cdvdfsv_rtocbuf[fssift], secsize - fssift);
							optimized_memcpy(g_cdvdfsv_rtocbuf, &g_cdvdfsv_rtocbuf[secsize + secsize * csec], secsize);
							g_cdvdfsv_rdp2sdd.size =
								((int)i == nsec_div_cdvdfsv_sectors - 1) ? dmasize_tmp : (secsize * (csec - 1) + fssift);
							optimized_memcpy(
								&g_cdvdfsv_rtocbuf[secsize + secsize - fssift],
								&g_cdvdfsv_rtocbuf[secsize * 2],
								g_cdvdfsv_rdp2sdd.size);
						}
						else
						{
							optimized_memcpy(g_cdvdfsv_rtocbuf, &g_cdvdfsv_rtocbuf[secsize * 2 + secsize * (csec - 1)], secsize);
							g_cdvdfsv_rdp2sdd.size = ((int)i == nsec_div_cdvdfsv_sectors - 1) ? dmasize_tmp : (secsize * (csec - 1));
							optimized_memcpy(
								&g_cdvdfsv_rtocbuf[secsize], &g_cdvdfsv_rtocbuf[secsize * 2 + fssift], g_cdvdfsv_rdp2sdd.size);
						}
						g_cdvdfsv_rdp2sdd.src = &g_cdvdfsv_rtocbuf[secsize];
					}
					else
					{
						g_cdvdfsv_rdp2sdd.src = &g_cdvdfsv_rtocbuf[secsize * 2];
						g_cdvdfsv_rdp2sdd.size = secsize * csec;
					}
					g_cdvdfsv_rdp2sdd.attr = 0;
					g_cdvdfsv_rdp2sdd.dest = ee_addr_tmp;
					ee_addr_tmp += g_cdvdfsv_rdp2sdd.size;
					dmasize_tmp -= g_cdvdfsv_rdp2sdd.size;
					while ( 1 )
					{
						CpuSuspendIntr(&state);
						trid = sceSifSetDma(&g_cdvdfsv_rdp2sdd, 1);
						CpuResumeIntr(state);
						if ( trid )
							break;
						DelayThread(500);
					}
					while ( cdvdfsv_checkdmastat(trid) >= 0 )
						;
				}
				else if ( !fssift )
				{
					for ( j = 0; j < csec; j += 1 )
					{
						g_cdvdfsv_multi_dmat[j].dest = &ee_addr[(csec_comm + j) * secsize];
						g_cdvdfsv_multi_dmat[j].src = &g_cdvdfsv_rtocbuf[0x1248 + (j * sector_size) + 12];
					}
					while ( 1 )
					{
						CpuSuspendIntr(&state);
						trid = sceSifSetDma(g_cdvdfsv_multi_dmat, csec);
						CpuResumeIntr(state);
						if ( trid )
							break;
						DelayThread(500);
					}
					while ( cdvdfsv_checkdmastat(trid) >= 0 )
						;
				}
				else
				{
					size_2 = ((int)i != nsec_div_cdvdfsv_sectors - 1) ? fssift : secsize;
					g_cdvdfsv_rdp2sdd.size = dmasize_tmp;
					if ( i )
					{
						optimized_memcpy(&g_cdvdfsv_rtocbuf[0x924], &g_cdvdfsv_rtocbuf[fssift + 12], secsize - fssift);
						optimized_memcpy(g_cdvdfsv_rtocbuf, &g_cdvdfsv_rtocbuf[0x1248 + (csec - 1) * sector_size], sector_size);
						for ( j = 0; j < csec - 1; j += 1 )
						{
							optimized_memcpy(
								&g_cdvdfsv_rtocbuf[0x924 + secsize - fssift + (j * secsize)],
								&g_cdvdfsv_rtocbuf[0x1248 + 12 + (j * sector_size)],
								secsize);
						}
						optimized_memcpy(
							&g_cdvdfsv_rtocbuf[0x924 + secsize - fssift + ((csec - 1) * secsize)],
							&g_cdvdfsv_rtocbuf[0x1248 + 12 + ((csec - 1) * sector_size)],
							size_2);
						if ( (int)i != nsec_div_cdvdfsv_sectors - 1 )
						{
							g_cdvdfsv_rdp2sdd.size = secsize * csec;
						}
					}
					else
					{
						optimized_memcpy(g_cdvdfsv_rtocbuf, &g_cdvdfsv_rtocbuf[0x1248 + (csec - 1) * sector_size], sector_size);
						optimized_memcpy(&g_cdvdfsv_rtocbuf[0x924], &g_cdvdfsv_rtocbuf[0x1248 + fssift + 12], secsize - fssift);
						for ( j = 0; j < csec - 2; j += 1 )
						{
							optimized_memcpy(
								&g_cdvdfsv_rtocbuf[0x924 + secsize - fssift + (j * secsize)],
								&g_cdvdfsv_rtocbuf[0x1248 + sector_size + 12 + (j * sector_size)],
								secsize);
						}
						optimized_memcpy(
							&g_cdvdfsv_rtocbuf[0x924 + secsize - fssift + ((csec - 2) * secsize)],
							&g_cdvdfsv_rtocbuf[0x1248 + sector_size + 12 + ((csec - 2) * sector_size)],
							size_2);
						if ( (int)i != nsec_div_cdvdfsv_sectors - 1 )
						{
							g_cdvdfsv_rdp2sdd.size = secsize * (csec - 1);
						}
					}
					g_cdvdfsv_rdp2sdd.src = &g_cdvdfsv_rtocbuf[0x924];
					g_cdvdfsv_rdp2sdd.attr = 0;
					g_cdvdfsv_rdp2sdd.dest = ee_addr_tmp;
					ee_addr_tmp += g_cdvdfsv_rdp2sdd.size;
					dmasize_tmp -= g_cdvdfsv_rdp2sdd.size;
					while ( 1 )
					{
						CpuSuspendIntr(&state);
						trid = sceSifSetDma(&g_cdvdfsv_rdp2sdd, 1);
						CpuResumeIntr(state);
						if ( trid )
							break;
						DelayThread(500);
					}
					while ( cdvdfsv_checkdmastat(trid) >= 0 )
						;
				}
			}
			else
			{
				retry_flag1 = 1;
			}
			CpuSuspendIntr(&state);
			if ( (int)i == nsec_div_cdvdfsv_sectors - 1 )
			{
				DisableIntr(IOP_IRQ_DMA_CDVD, (int *)&chcr);
			}
			else
			{
				csec_comm += csec;
				csec = ((unsigned int)csec > nsec - (unsigned int)csec_comm) ? (nsec - (unsigned int)csec_comm) :
																																			 (unsigned int)g_cdvdfsv_sectors;
				ClearEventFlag(g_cdvdman_intr_evfid, ~0x20);
				dmac_ch_set_chcr(3, 0);
				dmac_ch_get_chcr(3);
				g_cdvdman_istruct_ptr->m_dma3_param.m_dma3_maddress = &g_cdvdfsv_rtocbuf[0x1248];
				dmac_ch_set_madr(3, (uiptr)(&g_cdvdfsv_rtocbuf[0x1248]));
				dmac_ch_set_bcr(
					3,
					g_cdvdman_istruct_ptr->m_dma3_param.m_dma3_blkwords
						| ((g_cdvdman_istruct_ptr->m_dma3_param.m_dma3_blkcount * csec) << 16));
				dmac_ch_set_chcr(3, 0x41000200);
				chcr = dmac_ch_get_chcr(3);
				if ( post_dmat )
				{
					g_cdvdfsv_readpos += secsize * csec;
					sceSifSetDma(post_dmat, 1);
				}
			}
			CpuResumeIntr(state);
		}
		sceCdSync(5);
		CancelAlarm(read_timeout_alarm_cb, &g_cdvdfsv_read_timeout);
		g_cdvdman_istruct_ptr->m_dec_mode_set = 0;
		g_cdvdman_istruct_ptr->m_dec_state = 0;
		error_code = sceCdGetError();
		if ( (u16)g_cdvdman_istruct_ptr->m_dec_mode_last_set )
		{
			retry_flag2 = 1;
			error_code = SCECdErNO;
			mode->spindlctrl = 16;
		}
		if ( error_code != SCECdErNO || g_cdvdfsv_r2count >= 5 )
			break;
		if ( !retry_flag2 )
		{
			if ( error_code_tmp == SCECdErNO )
				return 1;
			sceCdSC(0xFFFFFFFE, &error_code_tmp);
			VERBOSE_KPRINTF(1, "secid_chk_ee_trns lsn %d nsec %d IPI Err\n", lsn, nsec);
			return 0;
		}
		if ( !g_cdvdfsv_r2retry )
		{
			g_cdvdfsv_r2count += 1;
			VERBOSE_PRINTF(1, "Read_CD/DVD-ROM Error Recover Start\n");
			g_cdvdfsv_r2retry = g_cdvdfsv_r2retry_initval;
		}
	}
	if ( g_cdvdfsv_r2count >= 5 && error_code == SCECdErNO )
	{
		sceCdSC(0xFFFFFFFE, &g_cdvdfsv_rderror);
	}
	return 0;
}

static int readproc1(
	unsigned int lsn,
	u32 nsec,
	void *retptr,
	sceCdRMode *rmode,
	int ps2dvd,
	int enable_retries,
	int dec_shift_enable,
	int dec_shift_value)
{
	int scres_unused;
	int state;
	u32 syncdec_mask;
	int error_code_tmp;

	error_code_tmp = SCECdErNO;
	g_cdvdfsv_read_timeout.hi = 0;
	g_cdvdfsv_read_timeout.lo = 0x9000 * sceCdSC(0xFFFFFFF1, &scres_unused);
	g_cdvdfsv_rderror = SCECdErREADCF;
	g_cdvdfsv_sid_err_recover_cnt = 0;
	g_cdvdfsv_err_count = 0;
	while ( 1 )
	{
		int cmd_error;
		int error_code;

		CpuSuspendIntr(&state);
		if ( dec_shift_enable )
		{
			g_cdvdman_istruct_ptr->m_dec_shift = dec_shift_value;
			g_cdvdman_istruct_ptr->m_dec_state = 2;
		}
		cmd_error = (ps2dvd ? sceCdRV : sceCdRead0)(
			(lsn >= 0x30) ? (lsn - 0x10 * g_cdvdfsv_sid_err_recover_cnt) : (lsn + 0x10 * g_cdvdfsv_sid_err_recover_cnt),
			nsec,
			retptr,
			rmode,
			0,
			0);
		CpuResumeIntr(state);
		if ( cmd_error )
			SetAlarm(&g_cdvdfsv_read_timeout, read_timeout_alarm_cb, &g_cdvdfsv_read_timeout);
		sceCdSync(5);
		CancelAlarm(read_timeout_alarm_cb, &g_cdvdfsv_read_timeout);
		g_cdvdman_istruct_ptr->m_dec_state = 0;
		error_code = sceCdGetError();
		if ( error_code != SCECdErNO || !cmd_error || g_cdvdfsv_err_count >= 5 )
		{
			VERBOSE_KPRINTF(1, "Read error error code %x cmd error %d\n", error_code, cmd_error);
			if ( (!cmd_error || g_cdvdfsv_err_count >= 5) && error_code == SCECdErNO )
			{
				sceCdSC(0xFFFFFFFE, &g_cdvdfsv_rderror);
			}
			return 0;
		}
		if ( cdvdfsv_checksid(lsn, nsec, ps2dvd, retptr, dec_shift_enable, dec_shift_value, &syncdec_mask) )
		{
			if ( enable_retries && syncdec_mask )
				error_code_tmp = SCECdErIPI;
			break;
		}
		if ( !enable_retries )
			break;
		if ( !g_cdvdfsv_sid_err_recover_cnt )
		{
			g_cdvdfsv_err_count += 1;
			VERBOSE_PRINTF(1, "Read_CD/DVD-ROM Sector_ID Error Recover Start\n");
			g_cdvdfsv_sid_err_recover_cnt = 3;
		}
		g_cdvdfsv_sid_err_recover_cnt -= 1;
	}
	if ( error_code_tmp == SCECdErNO )
		return 1;
	sceCdSC(0xFFFFFFFE, &error_code_tmp);
	VERBOSE_KPRINTF(1, "secid_chk lsn %d nsec %d IPI Err\n", lsn, nsec);
	return 0;
}

static void cdvdfsv_rpc5_01_readee(
	const cdvdfsv_rpc5_inpacket_t *inbuf, int buflen, cdvdfsv_rpc5_outpacket_t *outbuf, u32 ps2dvd, int sync, int decflag)
{
	unsigned int secsize;
	unsigned int bsize;
	unsigned int bsize_tmp;
	unsigned int psize;
	unsigned int ssize;
	unsigned int i;
	int sizestuff;
	u32 needed_offset;
	int sector_sizes[2];
	int scres_unused;
	int lsndualchg_res;
	int state;
	unsigned int buf_offs_sum;
	unsigned int paddr;
	int saddr;
	int datapattern;
	unsigned int len2_plus_sec2;
	int trid;
	int decval;
	int early_break;

	(void)buflen;

	early_break = 0;
	trid = 0;
	buf_offs_sum = 0;
	sector_sizes[0] = 0x924;
	sector_sizes[1] = 0x810;
	g_cdvdfsv_rmodeee = inbuf->m_pkt_01.m_rmodeee;
	lsndualchg_res = inbuf->m_pkt_01.m_lbn;
	decval = decflag ? inbuf->m_pkt_01.m_decval : 0;
	g_cdvdfsv_eerpsdd.src = &g_cdvdfsv_readpos;
	g_cdvdfsv_eerpsdd.size = sizeof(g_cdvdfsv_readpos);
	g_cdvdfsv_eerpsdd.attr = 0;
	g_cdvdfsv_eerpsdd.dest = (void *)inbuf->m_pkt_01.m_eedest;
	if ( ps2dvd )
	{
		if ( !sceCdSC(0xFFFFFFEA, &scres_unused) )
		{
			g_cdvdfsv_rderror = SCECdErREADCFR;
			sceCdSC(0xFFFFFFFE, &g_cdvdfsv_rderror);
			bsize = 0;
			psize = 0;
			ssize = 0;
			saddr = 0;
			paddr = 0;
			early_break = 1;
		}
		else
		{
			lsndualchg_res = sceCdSC(0xFFFFFFE9, &lsndualchg_res);
			secsize = 0x800;
			datapattern = SCECdSecS2048;
			g_cdvdfsv_rmodeee.datapattern = SCECdSecS2048;
		}
	}
	else
	{
		datapattern = g_cdvdfsv_rmodeee.datapattern;
		switch ( datapattern )
		{
			case SCECdSecS2328:
				secsize = 0x918;
				break;
			case SCECdSecS2340:
				secsize = 0x924;
				break;
			case SCECdSecS2048:
			default:
				secsize = 0x800;
				break;
		}
		g_cdvdfsv_rmodeee.datapattern = SCECdSecS2340;
	}
	len2_plus_sec2 = lsndualchg_res + inbuf->m_pkt_01.m_sectors;
	if ( !early_break )
	{
		int all_sec_bytes;

		all_sec_bytes = secsize * inbuf->m_pkt_01.m_sectors;
		if ( g_cdvdfsv_spinctl != -1 )
			g_cdvdfsv_rmodeee.spindlctrl = g_cdvdfsv_spinctl;
		paddr = inbuf->m_pkt_01.m_paddr;
		saddr = (paddr + all_sec_bytes) & ~0x3F;
		psize = ((paddr & 0x3F)) ? ((paddr & ~0x3F) - (paddr - 0x40)) : 0;
		bsize = saddr - (paddr + psize);
		ssize = paddr + all_sec_bytes - saddr;
		VERBOSE_KPRINTF(1, "CD/DVD-ROM lsn= %d sec= %d\n", lsndualchg_res, inbuf->m_pkt_01.m_sectors);
		VERBOSE_KPRINTF(1, "f psize= %d bsize= %d ssize= %d\n", psize, bsize, ssize);
	}
	if ( psize )
	{
		u32 sectors;

		sectors = (len2_plus_sec2 < lsndualchg_res + buf_offs_sum / secsize + 2) ? 1 : 2;
		VERBOSE_PRINTF(
			1,
			"0 CD_READ LBN= %d sectors= %d all= %d\n",
			(int)(lsndualchg_res + buf_offs_sum / secsize),
			(int)sectors,
			(int)inbuf->m_pkt_01.m_sectors);
		if ( !readproc1(
					 lsndualchg_res + buf_offs_sum / secsize,
					 sectors,
					 g_cdvdfsv_rtocbuf,
					 &g_cdvdfsv_rmodeee,
					 ps2dvd,
					 sync,
					 decflag,
					 decval) )
		{
			ssize = 0;
			psize = 0;
			bsize = 0;
		}
		else
		{
			if ( datapattern != SCECdSecS2340 || ps2dvd )
			{
				int rtoc_ind;

				rtoc_ind = 12;
				for ( i = 0; i < psize; i += 1 )
				{
					rtoc_ind += (i && !(i % secsize)) ? (sector_sizes[ps2dvd] - secsize) : 0;
					g_cdvdfsv_eereadx.m_pbuf1[i] = g_cdvdfsv_rtocbuf[rtoc_ind + i];
				}
			}
			else
			{
				for ( i = 0; i < psize; i += 1 )
					g_cdvdfsv_eereadx.m_pbuf1[i] = g_cdvdfsv_rtocbuf[i];
			}
			buf_offs_sum += psize;
		}
	}
	bsize_tmp = bsize;
	for ( i = 0; i < bsize; i += sizestuff )
	{
		u32 offs_sector_only;

		bsize_tmp = bsize - i;
		if ( g_cdvdfsv_spinctl != -1 )
			g_cdvdfsv_rmodeee.spindlctrl = g_cdvdfsv_spinctl;
		offs_sector_only = lsndualchg_res + buf_offs_sum / secsize;
		if ( (unsigned int)(secsize << 6) >= bsize_tmp )
		{
			needed_offset = (bsize_tmp / secsize) + (!!(bsize_tmp % secsize));
			sizestuff = bsize_tmp;
		}
		else
		{
			needed_offset = (((offs_sector_only & 0xF)) && (!(secsize & 0xF))) ? (0x10 - (offs_sector_only & 0xF)) : 0x40;
			sizestuff = secsize * needed_offset;
		}
		needed_offset += !!((buf_offs_sum + i) % secsize);
		if ( len2_plus_sec2 < offs_sector_only + needed_offset )
			needed_offset = len2_plus_sec2 - (lsndualchg_res + (buf_offs_sum + i) / secsize);
		g_cdvdfsv_readpos = buf_offs_sum + i;
		if ( !readproc2(
					 offs_sector_only,
					 needed_offset,
					 &g_cdvdfsv_rmodeee,
					 ps2dvd,
					 sync,
					 decflag,
					 decval,
					 (char *)(inbuf->m_pkt_01.m_paddr + psize) + i,
					 (buf_offs_sum + i) % secsize,
					 secsize,
					 sizestuff,
					 &g_cdvdfsv_eerpsdd) )
		{
			bsize_tmp = 0;
			early_break = 1;
			break;
		}
		while ( cdvdfsv_checkdmastat(trid) >= 0 )
			;
		CpuSuspendIntr(&state);
		trid = sceSifSetDma(&g_cdvdfsv_eerpsdd, 1);
		CpuResumeIntr(state);
	}
	buf_offs_sum += i;
	bsize = bsize_tmp;
	if ( !early_break && ssize )
	{
		u32 sectors_1;
		unsigned int buf_offs_sum_bytes_in_sector;

		buf_offs_sum_bytes_in_sector = buf_offs_sum % secsize;
		sectors_1 = (len2_plus_sec2 < lsndualchg_res + buf_offs_sum / secsize + 2) ? 1 : 2;
		VERBOSE_PRINTF(
			1, "2 CD_READ LBN= %d sectors= %d\n", (int)(lsndualchg_res + buf_offs_sum / secsize), (int)sectors_1);
		if ( !readproc1(
					 lsndualchg_res + buf_offs_sum / secsize,
					 sectors_1,
					 g_cdvdfsv_rtocbuf,
					 &g_cdvdfsv_rmodeee,
					 ps2dvd,
					 sync,
					 decflag,
					 decval) )
		{
			bsize = 0;
		}
		else
		{
			if ( datapattern != SCECdSecS2340 || ps2dvd )
			{
				int i2_offs;

				i2_offs = 12;
				for ( i = 0; i < ssize; i += 1 )
				{
					i2_offs +=
						((i + buf_offs_sum_bytes_in_sector)
						 && (i % secsize) == (secsize - (buf_offs_sum_bytes_in_sector ? buf_offs_sum_bytes_in_sector : secsize))) ?
							(sector_sizes[ps2dvd] - secsize) :
							0;
					g_cdvdfsv_eereadx.m_pbuf2[i] = g_cdvdfsv_rtocbuf[buf_offs_sum_bytes_in_sector + i2_offs + i];
				}
			}
			else
			{
				for ( i = 0; i < ssize; i += 1 )
					g_cdvdfsv_eereadx.m_pbuf2[i] = g_cdvdfsv_rtocbuf[buf_offs_sum_bytes_in_sector + i];
			}
			buf_offs_sum += ssize;
		}
	}
	g_cdvdfsv_eereadx.m_b1len = psize;
	g_cdvdfsv_eereadx.m_b2len = ssize;
	g_cdvdfsv_eereadx.m_b1dst = paddr;
	g_cdvdfsv_eereadx.m_b2dst = saddr;
	VERBOSE_PRINTF(
		1, "b psize= %d paddr= %08x bsize= %d ssize= %d saddr %08x\n", (int)psize, paddr, (int)bsize, (int)ssize, saddr);
	while ( cdvdfsv_checkdmastat(trid) >= 0 )
		;
	g_cdvdfsv_datasdd.src = &g_cdvdfsv_eereadx;
	g_cdvdfsv_datasdd.size = sizeof(g_cdvdfsv_eereadx);
	g_cdvdfsv_datasdd.attr = 0;
	g_cdvdfsv_readpos = buf_offs_sum;
	g_cdvdfsv_datasdd.dest = (void *)inbuf->m_pkt_01.m_eeremaindest;
	while ( 1 )
	{
		CpuSuspendIntr(&state);
		trid = sceSifSetDma(&g_cdvdfsv_datasdd, 1);
		sceSifSetDma(&g_cdvdfsv_eerpsdd, 1);
		CpuResumeIntr(state);
		if ( trid )
			break;
		DelayThread(500);
	}
	while ( cdvdfsv_checkdmastat(trid) >= 0 )
		;
	g_cdvdfsv_spinctl = -1;
	VERBOSE_PRINTF(1, "read end\n");
	outbuf->m_retres = buf_offs_sum;
}

static int
cdvdfsv_chreadee(int secoffs, int seccount, char *ee_addr, const sceCdRMode *in_rmode, u32 disktype_14, int sync)
{
	unsigned int secsize;
	unsigned int i;
	int readsize_bytes;
	sceCdRMode rmode;
	int scres_unused;
	int lsndualchg_res;

	lsndualchg_res = secoffs;
	rmode = *in_rmode;
	if ( disktype_14 )
	{
		if ( !sceCdSC(0xFFFFFFEA, &scres_unused) )
		{
			g_cdvdfsv_rderror = SCECdErREADCFR;
			sceCdSC(0xFFFFFFFE, &g_cdvdfsv_rderror);
			return 1;
		}
		secsize = 0x800;
		lsndualchg_res = sceCdSC(0xFFFFFFE9, &lsndualchg_res);
		rmode.datapattern = SCECdSecS2048;
	}
	else
	{
		switch ( rmode.datapattern )
		{
			case SCECdSecS2328:
				secsize = 0x918;
				break;
			case SCECdSecS2340:
				secsize = 0x924;
				break;
			case SCECdSecS2048:
			default:
				secsize = 0x800;
				break;
		}
		rmode.datapattern = SCECdSecS2340;
	}
	for ( i = 0; i < (unsigned int)(secsize * seccount); i += readsize_bytes )
	{
		unsigned int bytescount;
		int sectors_partial;
		int bytescount_in_sectors;

		bytescount = (unsigned int)(secsize * seccount) - i;
		sectors_partial = (lsndualchg_res + i / secsize) & 0xF;
		bytescount_in_sectors = 0x10;
		readsize_bytes = secsize * bytescount_in_sectors;
		if ( (unsigned int)readsize_bytes >= bytescount )
		{
			bytescount_in_sectors = (bytescount / secsize) + (!!(bytescount % secsize));
			readsize_bytes = bytescount;
		}
		else if ( sectors_partial && !(secsize & 0xF) )
		{
			bytescount_in_sectors -= sectors_partial;
		}
		if ( !readproc2(
					 lsndualchg_res + i / secsize,
					 bytescount_in_sectors,
					 &rmode,
					 disktype_14,
					 sync,
					 0,
					 0,
					 ee_addr + i,
					 0,
					 secsize,
					 readsize_bytes,
					 0) )
		{
			break;
		}
	}
	return 1;
}

static void
cdvdfsv_rpc5_0F_readchain(const cdvdfsv_rpc5_inpacket_t *inbuf, int buflen, cdvdfsv_rpc5_outpacket_t *outbuf)
{
	int sector_size;
	unsigned int i;
	const sceCdRChain *chain;
	void *buf;
	int re_result;
	int trid;
	int scres_unused;
	int state;

	(void)buflen;
	(void)outbuf;

	g_cdvdfsv_rderror = SCECdErREADCFR;
	g_cdvdfsv_readpos = 0;
	g_cdvdman_istruct_ptr->m_break_cdvdfsv_readchain = 0;
	g_cdvdfsv_chrdsdd.src = &g_cdvdfsv_readpos;
	g_cdvdfsv_chrdsdd.size = sizeof(g_cdvdfsv_readpos);
	g_cdvdfsv_chrdsdd.attr = 0;
	g_cdvdfsv_chrdsdd.dest = (void *)inbuf->m_pkt_0F.m_eedest;
	switch ( inbuf->m_pkt_0F.m_mode.datapattern )
	{
		case SCECdSecS2328:
			sector_size = 0x918;
			break;
		case SCECdSecS2340:
			sector_size = 0x924;
			break;
		case SCECdSecS2048:
		default:
			sector_size = 0x800;
			break;
	}
	chain = inbuf->m_pkt_0F.m_readChain;
	for ( i = 0; i < 0x40; i += 1 )
	{
		if ( g_cdvdman_istruct_ptr->m_break_cdvdfsv_readchain )
		{
			VERBOSE_PRINTF(1, "ReadChain cnt %d on sceCdBreak()\n", (int)i);
			return;
		}
		if ( chain[i].lbn == 0xFFFFFFFF || chain[i].sectors == 0xFFFFFFFF || chain[i].buffer == 0xFFFFFFFF )
			return;
		if ( (chain[i].buffer & 1) )
		{
			buf = (void *)(chain[i].buffer & ~1);
			VERBOSE_PRINTF(
				1,
				"ReadChain lsn= %d nsec= %d buf= %08x secsize= %d\n",
				(int)(chain[i].lbn),
				(int)(chain[i].sectors),
				(unsigned int)(uiptr)buf,
				inbuf->m_pkt_0F.m_mode.datapattern);
			re_result = sceCdRE(chain[i].lbn, chain[i].sectors, buf, (sceCdRMode *)&(inbuf->m_pkt_0F.m_mode));
			if ( re_result == 1 )
			{
				sceCdSync(0);
				re_result = sceCdGetError() == SCECdErNO;
			}
			else
			{
				sceCdSC(0xFFFFFFFE, &g_cdvdfsv_rderror);
			}
		}
		else
		{
			// The following printf was modified for ioprp300x
			VERBOSE_PRINTF(
				1, "ReadChain EE\t Memory addr= 0x%08x sector= %d\n", (unsigned int)(chain[i].lbn), (int)(chain[i].sectors));
			// The following call to sceCdGetDiskType was inlined
			re_result = cdvdfsv_chreadee(
				chain[i].lbn,
				chain[i].sectors,
				(char *)chain[i].buffer,
				&(inbuf->m_pkt_0F.m_mode),
				sceCdGetDiskType() == SCECdPS2DVD,
				!sceCdSC(0xFFFFFFFC, &scres_unused));
		}
		if ( !re_result )
		{
			VERBOSE_PRINTF(1, "ReadChain error code= 0x%02x\n", sceCdGetError());
			break;
		}
		g_cdvdfsv_readpos += chain[i].sectors * sector_size;
		while ( 1 )
		{
			CpuSuspendIntr(&state);
			trid = sceSifSetDma(&g_cdvdfsv_chrdsdd, 1);
			CpuResumeIntr(state);
			if ( trid )
				break;
			DelayThread(500);
		}
		while ( cdvdfsv_checkdmastat(trid) >= 0 )
			;
	}
}

#ifdef CDVD_VARIANT_OSD
static void cdvdfsv_rpc5_03_readdvdv(const cdvdfsv_rpc5_inpacket_t *inbuf, int buflen, cdvdfsv_rpc5_outpacket_t *outbuf)
{
	int all_sec_bytes;
	unsigned int buf_1_toalign;
	unsigned int buf_offs_mod_sector_size;
	unsigned int buf_aligned;
	u32 sectors;
	int lbn;
	int cmd_error;
	int error_code;
	unsigned int i;
	int readbuf;
	int cpysize;
	int trid;
	int state;
	unsigned int buf_offs;
	unsigned int buf_toalign;
	unsigned int buf_sec_tmp;

	(void)buflen;
	error_code = SCECdErNO;
	cmd_error = 1;
	buf_offs = 0;
	g_cdvdfsv_rderror = SCECdErREADCFR;
	all_sec_bytes = 0x810 * inbuf->m_pkt_03.m_nsectors;
	buf_toalign = ((inbuf->m_pkt_03.m_buf & 0x3F)) ? (inbuf->m_pkt_03.m_buf & ~0x3F) - (inbuf->m_pkt_03.m_buf - 0x40) : 0;
	buf_1_toalign = (inbuf->m_pkt_03.m_buf + all_sec_bytes) & ~0x3F;
	buf_sec_tmp = all_sec_bytes - (buf_1_toalign - inbuf->m_pkt_03.m_buf);
	if ( buf_toalign )
	{
		lbn = inbuf->m_pkt_03.m_lbn + buf_offs / 0x810;
		sectors = 1 + (!!((inbuf->m_pkt_03.m_lbn + inbuf->m_pkt_03.m_nsectors) >= (unsigned int)(lbn + 2)));
		VERBOSE_PRINTF(1, "0 CD_READ LBN= %d sectors= %d all= %d\n", lbn, (int)sectors, (int)inbuf->m_pkt_03.m_nsectors);
		cmd_error = sceCdReadDVDV(lbn, sectors, g_cdvdfsv_fsvrbuf[0], (sceCdRMode *)&inbuf->m_pkt_03.m_mode);
		sceCdSync(0);
		error_code = sceCdGetError();
		if ( error_code != SCECdErNO || !cmd_error )
		{
			if ( !cmd_error )
				sceCdSC(0xFFFFFFFE, &g_cdvdfsv_rderror);
			VERBOSE_PRINTF(1, "Read error code %x cmd error %d\n", error_code, cmd_error);
			buf_toalign = 0;
		}
		for ( i = 0; i < buf_toalign; i += 1 )
			g_cdvdfsv_eereadx.m_pbuf1[i] = g_cdvdfsv_fsvrbuf[0][i];
		buf_offs += buf_toalign;
	}
	if ( error_code == SCECdErNO && cmd_error )
	{
		int firstflag;
		unsigned int sector_count_in_bytes;

		buf_offs_mod_sector_size = 0;
		readbuf = 0;
		firstflag = 0;
		cpysize = 0;
		for ( buf_aligned = inbuf->m_pkt_03.m_buf + buf_toalign; buf_aligned < buf_1_toalign;
					buf_aligned += sector_count_in_bytes )
		{
			unsigned int buf_align_remain;

			buf_align_remain = buf_1_toalign - buf_aligned;
			buf_offs_mod_sector_size = (buf_aligned - inbuf->m_pkt_03.m_buf) % 0x810;
			sector_count_in_bytes = (0x8100 >= buf_align_remain) ? buf_align_remain : 0x8100;
			sectors = (0x8100 >= buf_align_remain) ? (buf_align_remain / 0x810 + (!!(buf_align_remain % 0x810))) : 16;
			sectors += !!buf_offs_mod_sector_size;
			lbn = inbuf->m_pkt_03.m_lbn + (buf_aligned - inbuf->m_pkt_03.m_buf) / 0x810;
			if ( sectors > (inbuf->m_pkt_03.m_lbn + inbuf->m_pkt_03.m_nsectors) - lbn )
				sectors = (inbuf->m_pkt_03.m_lbn + inbuf->m_pkt_03.m_nsectors) - lbn;
			cmd_error = sceCdReadDVDV(lbn, sectors, g_cdvdfsv_fsvrbuf[readbuf], (sceCdRMode *)&inbuf->m_pkt_03.m_mode);
			if ( firstflag )
			{
				if ( buf_offs_mod_sector_size )
					optimized_memcpy(
						g_cdvdfsv_fsvrbuf[readbuf ^ 1], &g_cdvdfsv_fsvrbuf[readbuf ^ 1][buf_offs_mod_sector_size], cpysize);
				g_cdvdfsv_readdvdv_dmat.dest = (void *)(buf_aligned - cpysize);
				g_cdvdfsv_readdvdv_dmat.size = cpysize;
				g_cdvdfsv_readdvdv_dmat.attr = 0;
				g_cdvdfsv_readdvdv_dmat.src = g_cdvdfsv_fsvrbuf[readbuf ^ 1];
				while ( 1 )
				{
					CpuSuspendIntr(&state);
					trid = sceSifSetDma(&g_cdvdfsv_readdvdv_dmat, 1);
					CpuResumeIntr(state);
					if ( trid )
						break;
					DelayThread(500);
				}
				while ( cdvdfsv_checkdmastat(trid) >= 0 )
					;
			}
			sceCdSync(0);
			error_code = sceCdGetError();
			if ( error_code != SCECdErNO || !cmd_error )
			{
				if ( !cmd_error )
					sceCdSC(0xFFFFFFFE, &g_cdvdfsv_rderror);
				VERBOSE_PRINTF(1, "Read error code %x cmd error %d\n", error_code, cmd_error);
				break;
			}
			firstflag = 1;
			readbuf ^= 1;
			cpysize = sector_count_in_bytes;
			buf_offs += sector_count_in_bytes;
		}
	}
	if ( error_code == SCECdErNO && cmd_error )
	{
		if ( buf_offs_mod_sector_size )
			optimized_memcpy(
				g_cdvdfsv_fsvrbuf[readbuf ^ 1], &g_cdvdfsv_fsvrbuf[readbuf ^ 1][buf_offs_mod_sector_size], cpysize);
		g_cdvdfsv_readdvdv_dmat.dest = (void *)buf_aligned;
		g_cdvdfsv_readdvdv_dmat.size = cpysize;
		g_cdvdfsv_readdvdv_dmat.attr = 0;
		g_cdvdfsv_readdvdv_dmat.src = g_cdvdfsv_fsvrbuf[readbuf ^ 1];
		while ( 1 )
		{
			CpuSuspendIntr(&state);
			trid = sceSifSetDma(&g_cdvdfsv_readdvdv_dmat, 1);
			CpuResumeIntr(state);
			if ( trid )
				break;
			DelayThread(500);
		}
		while ( cdvdfsv_checkdmastat(trid) >= 0 )
			;
		if ( buf_sec_tmp )
		{
			lbn = inbuf->m_pkt_03.m_lbn + buf_offs / 0x810;
			sectors = 1 + (!!((inbuf->m_pkt_03.m_lbn + inbuf->m_pkt_03.m_nsectors) >= (unsigned int)(lbn + 2)));
			VERBOSE_PRINTF(1, "2 CD_READ LBN= %d sectors= %d\n", lbn, (int)sectors);
			cmd_error = sceCdReadDVDV(lbn, sectors, g_cdvdfsv_fsvrbuf[0], (sceCdRMode *)&inbuf->m_pkt_03.m_mode);
			sceCdSync(0);
			error_code = sceCdGetError();
			if ( error_code != SCECdErNO || !cmd_error )
			{
				if ( !cmd_error )
					sceCdSC(0xFFFFFFFE, &g_cdvdfsv_rderror);
				VERBOSE_PRINTF(1, "Read error code %x cmd error %d\n", error_code, cmd_error);
				buf_sec_tmp = 0;
			}
			for ( i = 0; i < buf_sec_tmp; i += 1 )
				g_cdvdfsv_eereadx.m_pbuf2[i] = g_cdvdfsv_fsvrbuf[0][(buf_offs % 0x810) + i];
			buf_offs += buf_sec_tmp;
		}
	}
	g_cdvdfsv_eereadx.m_b1len = buf_toalign;
	g_cdvdfsv_eereadx.m_b2len = buf_sec_tmp;
	g_cdvdfsv_eereadx.m_b1dst = inbuf->m_pkt_03.m_buf;
	g_cdvdfsv_eereadx.m_b2dst = buf_1_toalign;
	g_cdvdfsv_readdvdv_dmat.src = &g_cdvdfsv_eereadx;
	g_cdvdfsv_readdvdv_dmat.size = sizeof(g_cdvdfsv_eereadx);
	g_cdvdfsv_readdvdv_dmat.attr = 0;
	g_cdvdfsv_readdvdv_dmat.dest = (void *)inbuf->m_pkt_03.m_eedest;
	while ( 1 )
	{
		CpuSuspendIntr(&state);
		trid = sceSifSetDma(&g_cdvdfsv_readdvdv_dmat, 1);
		CpuResumeIntr(state);
		if ( trid )
			break;
		DelayThread(500);
	}
	while ( cdvdfsv_checkdmastat(trid) >= 0 )
		;
	VERBOSE_PRINTF(1, "read end\n");
	outbuf->m_retres = buf_offs;
}
#endif

// Note: in OSD variant, the do_read_full argument was added
static void cdvdfsv_rpc5_02_readcdda(
	const cdvdfsv_rpc5_inpacket_t *inbuf, int buflen, cdvdfsv_rpc5_outpacket_t *outbuf, int do_read_full)
{
	int trid;
	unsigned int sector_size;
	int all_sec_bytes;
	unsigned int buf_1_toalign;
	u32 sectors;
	int lbn;
	int cmd_error;
	int error_code;
	unsigned int i;
	int state;
	int error_code_tmp;
	unsigned int buf_offs;
	unsigned int buf_toalign;
	unsigned int buf_sec_tmp;

#ifndef CDVD_VARIANT_OSD
	(void)do_read_full;
#endif
	trid = 0;
	g_cdvdfsv_rderror = SCECdErREADCFR;
	error_code_tmp = SCECdErNO;
	g_cdvdfsv_eereadfull_dma2.src = &g_cdvdfsv_readpos;
	g_cdvdfsv_eereadfull_dma2.size = sizeof(g_cdvdfsv_readpos);
	g_cdvdfsv_eereadfull_dma2.attr = 0;
	g_cdvdfsv_eereadfull_dma2.dest = (void *)inbuf->m_pkt_02.m_eedest;
	switch ( inbuf->m_pkt_02.m_mode.datapattern )
	{
		case SCECdSecS2368:
			sector_size = 0x940;
			break;
		case SCECdSecS2352:
		case SCECdSecS2448:
		default:
			sector_size = 0x930;
			break;
	}
	buf_offs = 0;
	all_sec_bytes = sector_size * inbuf->m_pkt_02.m_sectors;
	buf_toalign =
		((inbuf->m_pkt_02.m_buf & 0x3F)) ? ((inbuf->m_pkt_02.m_buf & ~0x3F) - (inbuf->m_pkt_02.m_buf - 0x40)) : 0;
	buf_1_toalign = (inbuf->m_pkt_02.m_buf + all_sec_bytes) & ~0x3F;
	buf_sec_tmp = all_sec_bytes - (buf_1_toalign - inbuf->m_pkt_02.m_buf);
	if ( buf_toalign )
	{
		unsigned int buf_offs_sectors;

		buf_offs_sectors = buf_offs / sector_size;
		lbn = inbuf->m_pkt_02.m_lbn + buf_offs_sectors;
		sectors = 1 + !!((inbuf->m_pkt_02.m_lbn + inbuf->m_pkt_02.m_sectors) >= (unsigned int)(lbn + 2));
		VERBOSE_PRINTF(
			1, "0 CD_READ LBN= %d sectors= %d all= %d\n", (int)lbn, (int)sectors, (int)inbuf->m_pkt_02.m_sectors);
#ifdef CDVD_VARIANT_OSD
		cmd_error = do_read_full ? sceCdReadFull(lbn, sectors, g_cdvdfsv_rtocbuf, (sceCdRMode *)&inbuf->m_pkt_02.m_mode) :
															 sceCdReadCDDA(lbn, sectors, g_cdvdfsv_rtocbuf, (sceCdRMode *)&inbuf->m_pkt_02.m_mode);
#else
		cmd_error = sceCdReadCDDA(lbn, sectors, g_cdvdfsv_rtocbuf, (sceCdRMode *)&inbuf->m_pkt_02.m_mode);
#endif
		sceCdSync(3);
		error_code = sceCdGetError();
		if ( error_code != SCECdErNO || !cmd_error )
		{
			if ( !cmd_error )
				sceCdSC(0xFFFFFFFE, &g_cdvdfsv_rderror);
			VERBOSE_PRINTF(1, "Read error code %x cmd error %d\n", error_code, cmd_error);
			if ( error_code == SCECdErEOM || error_code == SCECdErSFRMTNG )
				error_code_tmp = error_code;
			else
			{
				buf_toalign = 0;
			}
		}
		if ( error_code_tmp != SCECdErNO && error_code_tmp != SCECdErEOM && error_code_tmp != SCECdErSFRMTNG )
		{
			for ( i = 0; i < buf_toalign; i += 1 )
			{
				g_cdvdfsv_eereadx.m_pbuf1[i] = g_cdvdfsv_rtocbuf[i];
			}
			buf_offs += buf_toalign;
		}
	}
	if ( error_code_tmp != SCECdErNO && error_code_tmp != SCECdErEOM && error_code_tmp != SCECdErSFRMTNG )
	{
		unsigned int sector_count_in_bytes;
		unsigned int buf_aligned;

		for ( buf_aligned = inbuf->m_pkt_02.m_buf + buf_toalign; buf_aligned < buf_1_toalign;
					buf_aligned += sector_count_in_bytes )
		{
			unsigned int buf_align_remain;
			unsigned int buf_offs_mod_sector_size;

			buf_align_remain = buf_1_toalign - buf_aligned;
			buf_offs_mod_sector_size = buf_offs % sector_size;
			lbn = inbuf->m_pkt_02.m_lbn + buf_offs / sector_size;
			sectors = (g_cdvdfsv_sectors_cdda * sector_size >= buf_align_remain) ?
									(buf_align_remain / sector_size) + (!!(buf_align_remain % sector_size)) :
									g_cdvdfsv_sectors_cdda;
			sectors += !!buf_offs_mod_sector_size;
			sector_count_in_bytes = (g_cdvdfsv_sectors_cdda * sector_size >= buf_align_remain) ?
																g_cdvdfsv_sectors_cdda * sector_size :
																buf_align_remain;
			if ( sectors > (inbuf->m_pkt_02.m_lbn + inbuf->m_pkt_02.m_sectors) - lbn )
				sectors = (inbuf->m_pkt_02.m_lbn + inbuf->m_pkt_02.m_sectors) - lbn;
			while ( cdvdfsv_checkdmastat(trid) >= 0 )
				;
#ifdef CDVD_VARIANT_OSD
			cmd_error = do_read_full ? sceCdReadFull(lbn, sectors, g_cdvdfsv_rtocbuf, (sceCdRMode *)&inbuf->m_pkt_02.m_mode) :
																 sceCdReadCDDA(lbn, sectors, g_cdvdfsv_rtocbuf, (sceCdRMode *)&inbuf->m_pkt_02.m_mode);
#else
			cmd_error = sceCdReadCDDA(lbn, sectors, g_cdvdfsv_rtocbuf, (sceCdRMode *)&inbuf->m_pkt_02.m_mode);
#endif
			sceCdSync(3);
			error_code = sceCdGetError();
			if ( error_code != SCECdErNO || !cmd_error )
			{
				if ( !cmd_error )
					sceCdSC(0xFFFFFFFE, &g_cdvdfsv_rderror);
				VERBOSE_PRINTF(1, "Read error code %x cmd error %d\n", error_code, cmd_error);
				if ( error_code == SCECdErEOM || error_code == SCECdErSFRMTNG )
				{
					error_code_tmp = error_code;
					break;
				}
			}
			if ( buf_offs_mod_sector_size )
				optimized_memcpy(g_cdvdfsv_rtocbuf, &g_cdvdfsv_rtocbuf[buf_offs_mod_sector_size], sector_count_in_bytes);
			g_cdvdfsv_eereadfull_dma1.src = g_cdvdfsv_rtocbuf;
			g_cdvdfsv_eereadfull_dma1.size = sector_count_in_bytes;
			g_cdvdfsv_eereadfull_dma1.attr = 0;
			g_cdvdfsv_eereadfull_dma1.dest = (char *)buf_aligned;
			g_cdvdfsv_readpos = buf_offs;
			while ( 1 )
			{
				CpuSuspendIntr(&state);
				trid = sceSifSetDma(&g_cdvdfsv_eereadfull_dma1, 1);
				CpuResumeIntr(state);
				if ( trid )
					break;
				DelayThread(500);
			}
			if ( (unsigned int)buflen >= 0x19 )
			{
				CpuSuspendIntr(&state);
				sceSifSetDma(&g_cdvdfsv_eereadfull_dma2, 1);
				CpuResumeIntr(state);
			}
			buf_offs += sector_count_in_bytes;
		}
	}
	if (
		(error_code_tmp != SCECdErNO && error_code_tmp != SCECdErEOM && error_code_tmp != SCECdErSFRMTNG) && buf_sec_tmp )
	{
		lbn = inbuf->m_pkt_02.m_lbn + buf_offs / sector_size;
		sectors = 1 + !!((inbuf->m_pkt_02.m_lbn + inbuf->m_pkt_02.m_sectors) >= (unsigned int)(lbn + 2));
		VERBOSE_PRINTF(
			1,
			"0 CD_READ LBN= %d sectors= %d all= %d\n",
			(int)(inbuf->m_pkt_02.m_lbn + buf_offs / sector_size),
			(int)sectors,
			(int)inbuf->m_pkt_02.m_sectors);
		VERBOSE_PRINTF(1, "2 CD_READ LBN= %d sectors= %d\n", (int)lbn, (int)sectors);
#ifdef CDVD_VARIANT_OSD
		cmd_error = do_read_full ? sceCdReadFull(lbn, sectors, g_cdvdfsv_rtocbuf, (sceCdRMode *)&inbuf->m_pkt_02.m_mode) :
															 sceCdReadCDDA(lbn, sectors, g_cdvdfsv_rtocbuf, (sceCdRMode *)&inbuf->m_pkt_02.m_mode);
#else
		cmd_error = sceCdReadCDDA(lbn, sectors, g_cdvdfsv_rtocbuf, (sceCdRMode *)&inbuf->m_pkt_02.m_mode);
#endif
		sceCdSync(3);
		error_code = sceCdGetError();
		if ( error_code != SCECdErNO || !cmd_error )
		{
			if ( !cmd_error )
				sceCdSC(0xFFFFFFFE, &g_cdvdfsv_rderror);
			VERBOSE_PRINTF(1, "Read error code %x cmd error %d\n", error_code, cmd_error);
			if ( error_code == SCECdErEOM || error_code == SCECdErSFRMTNG )
				error_code_tmp = error_code;
			else
				buf_sec_tmp = 0;
		}
		for ( i = 0; i < buf_sec_tmp; i += 1 )
		{
			g_cdvdfsv_eereadx.m_pbuf2[i] = g_cdvdfsv_rtocbuf[(buf_offs % sector_size) + i];
		}
		buf_offs += buf_sec_tmp;
	}
	g_cdvdfsv_eereadx.m_b1len = buf_toalign;
	g_cdvdfsv_eereadx.m_b2len = buf_sec_tmp;
	g_cdvdfsv_eereadx.m_b1dst = inbuf->m_pkt_02.m_buf;
	g_cdvdfsv_eereadx.m_b2dst = buf_1_toalign;
	while ( cdvdfsv_checkdmastat(trid) >= 0 )
		;
	g_cdvdfsv_eereadfull_dma1.src = &g_cdvdfsv_eereadx;
	g_cdvdfsv_eereadfull_dma1.size = sizeof(g_cdvdfsv_eereadx);
	g_cdvdfsv_eereadfull_dma1.attr = 0;
	g_cdvdfsv_readpos = buf_offs;
	g_cdvdfsv_eereadfull_dma1.dest = (void *)inbuf->m_pkt_02.m_eeremaindest;
	while ( 1 )
	{
		CpuSuspendIntr(&state);
		trid = sceSifSetDma(&g_cdvdfsv_eereadfull_dma1, 1);
		if ( (unsigned int)buflen >= 0x19 )
			sceSifSetDma(&g_cdvdfsv_eereadfull_dma2, 1);
		CpuResumeIntr(state);
		if ( trid )
			break;
		DelayThread(500);
	}
	while ( cdvdfsv_checkdmastat(trid) >= 0 )
		;
	if ( error_code_tmp != SCECdErNO )
		sceCdSC(0xFFFFFFFE, &error_code_tmp);
	VERBOSE_PRINTF(1, "read end\n");
	outbuf->m_retres = buf_offs;
}

static void *cbrpc_rpc2_diskready(int fno, void *buffer, int length)
{
	(void)fno;
	(void)length;

	// The following call to sceCdStatus was inlined
	VERBOSE_KPRINTF(1, "DISK READY call 0x%02x\n", sceCdStatus());
	// The following call to sceCdDiskReady was inlined
	g_diskready_res.m_retres = sceCdDiskReady(((const cdvdfsv_rpc2_inpacket_t *)buffer)->m_mode);
	return (void *)&g_diskready_res;
}

static void cdvdfsv_rpc5_04_gettoc(const cdvdfsv_rpc5_inpacket_t *inbuf, int buflen, cdvdfsv_rpc5_outpacket_t *outbuf)
{
	int trid;
	int state;

	(void)buflen;

	VERBOSE_PRINTF(1, "GET TOC call 0x%08x\n", (int)inbuf);
	outbuf->m_retres = sceCdGetToc((u8 *)g_cdvdfsv_rtocbuf);
	VERBOSE_PRINTF(1, "GET TOC called\n");
	g_cdvdfsv_rtocsdd.src = g_cdvdfsv_rtocbuf;
	g_cdvdfsv_rtocsdd.size = 0x810;
	g_cdvdfsv_rtocsdd.attr = 0;
	g_cdvdfsv_rtocsdd.dest = (void *)inbuf->m_pkt_04.m_eedest;
	while ( 1 )
	{
		CpuSuspendIntr(&state);
		trid = sceSifSetDma(&g_cdvdfsv_rtocsdd, 1);
		CpuResumeIntr(state);
		if ( trid )
			break;
		DelayThread(500);
	}
	while ( cdvdfsv_checkdmastat(trid) >= 0 )
		;
	// The following call to sceCdGetDiskType was inlined
	switch ( sceCdGetDiskType() )
	{
		case SCECdPS2DVD:
		case SCECdDVDVR:
		case SCECdDVDV:
			outbuf->m_pkt_04.m_isdvd = 1;
			break;
		default:
			outbuf->m_pkt_04.m_isdvd = 0;
			break;
	}
}

static void cdvdfsv_rpc3_03_disktype(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	(void)inbuf;
	(void)buflen;

	outbuf->m_retres = sceCdGetDiskType();
}

static void cdvdfsv_rpc3_0C_cdstatus(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	(void)inbuf;
	(void)buflen;

	outbuf->m_retres = sceCdStatus();
}

static void cdvdfsv_rpc3_06_ri(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;

	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdRI(outbuf->m_pkt_06.m_buffer, &outbuf->m_pkt_06.m_result);
	}
}

static void cdvdfsv_rpc3_1A_rm(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;

	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdRM(outbuf->m_pkt_1A.m_buffer, &outbuf->m_pkt_1A.m_status);
	}
}

#ifdef CDVD_VARIANT_DNAS
static void cdvdfsv_rpc3_24_readguid(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;

	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdReadGUID(&outbuf->m_pkt_24.m_guid);
	}
}

static void
cdvdfsv_rpc3_26_readmodelid(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;

	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdReadModelID(&outbuf->m_pkt_26.m_id);
	}
}
#endif

static void cdvdfsv_rpc3_22_mmode(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	if ( buflen == 4 || !inbuf->m_pkt_22.m_char4 )
	{
		outbuf->m_retres = sceCdMmode(inbuf->m_pkt_22.m_media);
	}
}

#ifdef CDVD_VARIANT_XOSD
static void cdvdfsv_rpc3_2D_chgsys(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	(void)buflen;
	VERBOSE_KPRINTF(1, "EE call recv sceCdChgSys %d %08x\n", inbuf->m_pkt_2D.m_arg1, inbuf);
	outbuf->m_retres = sceCdChgSys(inbuf->m_pkt_2D.m_arg1);
}
#endif

static void
cdvdfsv_rpc3_23_changethreadpriority(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	(void)buflen;

	outbuf->m_retres = sceCdChangeThreadPriority(inbuf->m_pkt_23.m_priority);
}

#ifdef CDVD_VARIANT_OSD
static void cdvdfsv_rpc3_07_wi(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdWI(inbuf->m_pkt_07.m_buffer, &outbuf->m_pkt_07.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void cdvdfsv_rpc3_1B_wm(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdWM(inbuf->m_pkt_1B.m_buffer, &outbuf->m_pkt_1B.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_12_readconsoleid(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdReadConsoleID(outbuf->m_pkt_12.m_buffer, &outbuf->m_pkt_12.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_13_writeconsoleid(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdWriteConsoleID(inbuf->m_pkt_13.m_buffer, &outbuf->m_pkt_13.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_14_getmversion(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdMV(outbuf->m_pkt_14.m_buffer, &outbuf->m_pkt_14.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void cdvdfsv_rpc3_17_readsubq(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdReadSUBQ(outbuf->m_pkt_17.m_buffer, &outbuf->m_pkt_17.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_18_forbiddvdp(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdForbidDVDP(&outbuf->m_pkt_18.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_1C_forbidread(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdForbidRead(&outbuf->m_pkt_1C.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_1E_bootcertify(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdBootCertify(inbuf->m_pkt_1E.m_romname);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_1F_cancelpoffrdy(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdCancelPOffRdy(&outbuf->m_pkt_1F.m_status);
	}
}
#endif

static void cdvdfsv_rpc3_21_poweroff(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;

	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdPowerOff(&outbuf->m_pkt_21.m_result);
	}
}

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_20_blueledctl(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdBlueLEDCtl(inbuf->m_pkt_20.m_control, &outbuf->m_pkt_20.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_1D_sc_FFFFFFF8(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	(void)buflen;
	g_cdvdfsv_spinctl = inbuf->m_pkt_1D.m_spinctl;
	outbuf->m_retres = sceCdSC(0xFFFFFFF8, &g_cdvdfsv_spinctl);
}
#endif

static void
cdvdfsv_rpc3_15_ctrladout(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;

	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdCtrlADout(inbuf->m_pkt_15.m_mode, &outbuf->m_pkt_15.m_status);
	}
}

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_19_autoadjustctrl(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdAutoAdjustCtrl(inbuf->m_pkt_19.m_mode, &outbuf->m_pkt_19.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_29_readwakeuptime(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdReadWakeUpTime(
			&outbuf->m_pkt_29.m_clock,
			&outbuf->m_pkt_29.m_userdata,
			&outbuf->m_pkt_29.m_wakeupreason,
			&outbuf->m_pkt_29.m_flags);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_28_writewakeuptime(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		// Unofficial: copy to output buffer then use it
		memcpy(&outbuf->m_pkt_28.m_clock, &inbuf->m_pkt_28.m_clock, sizeof(sceCdCLOCK));
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres =
			sceCdWriteWakeUpTime(&outbuf->m_pkt_28.m_clock, inbuf->m_pkt_28.m_userdata, inbuf->m_pkt_28.m_flags);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_30_rcbypassctl(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdRcBypassCtl(inbuf->m_pkt_30.m_mode, &outbuf->m_pkt_30.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_32_sendscmd1d(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdSendSCmd1D(
			(int *)&outbuf->m_pkt_32.m_arg1, &outbuf->m_pkt_32.m_arg2, &outbuf->m_pkt_32.m_arg3, &outbuf->m_pkt_32.m_arg4);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_31_remote2_7(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceRemote2_7(inbuf->m_pkt_31.m_param, &outbuf->m_pkt_31.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_38_remote2_7_get(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceRemote2_7Get(&outbuf->m_pkt_38.m_param, &outbuf->m_pkt_38.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_2A_readps1bootparam(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdReadPS1BootParam(outbuf->m_pkt_2A.m_out, &outbuf->m_pkt_2A.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_2B_setfanprofile(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdSetFanProfile(inbuf->m_pkt_2B.m_param, &outbuf->m_pkt_2B.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void cdvdfsv_rpc3_2C_i_152(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
#if 0
		// TODO FIXME IMPORT
		outbuf->m_retres = cdvdman_152(&outbuf->m_pkt_2C.m_arg1, &outbuf->m_pkt_2C.m_arg2);
#else
		outbuf->m_retres = 1;
#endif
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_43_readregionparams(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdReadRegionParams(outbuf->m_pkt_43.m_arg1, &outbuf->m_pkt_43.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_44_writeregionparams(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdWriteRegionParams(
			inbuf->m_pkt_44.m_arg1, (u32 *)inbuf->m_pkt_44.m_arg2, (u8 *)inbuf->m_pkt_44.m_arg3, &outbuf->m_pkt_44.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_XOSD
static void
cdvdfsv_rpc3_2F_noticegamestart(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdNoticeGameStart(inbuf->m_pkt_2F.m_arg1, &outbuf->m_pkt_2F.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_35_setledsmode(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdSetLEDsMode(inbuf->m_pkt_35.m_param, &outbuf->m_pkt_35.m_status);
	}
}
#endif

static void
cdvdfsv_rpc3_01_readclock(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;

	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdReadClock(&outbuf->m_pkt_01.m_clock);
	}
}

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_02_writeclock(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;

	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		// Unofficial: copy to output buffer then use it
		memcpy(&outbuf->m_pkt_02.m_clock, &inbuf->m_pkt_02.m_clock, sizeof(sceCdCLOCK));
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdWriteClock(&outbuf->m_pkt_02.m_clock);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void cdvdfsv_rpc3_08_readnvm(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		// Unofficial: direct to outbuf
		outbuf->m_retres = sceCdReadNVM(inbuf->m_pkt_08.m_address, &outbuf->m_pkt_08.m_data, &outbuf->m_pkt_08.m_status);
	}
	outbuf->m_pkt_08.m_address = inbuf->m_pkt_08.m_address;
}
#endif

#ifdef CDVD_VARIANT_OSD
static void cdvdfsv_rpc3_09_writenvm(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdWriteNVM(inbuf->m_pkt_09.m_address, inbuf->m_pkt_09.m_data, &outbuf->m_pkt_09.m_status);
	}
	outbuf->m_pkt_09.m_address = inbuf->m_pkt_09.m_address;
	outbuf->m_pkt_09.m_data = inbuf->m_pkt_09.m_data;
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_0D_sethdmode(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	(void)buflen;
	outbuf->m_retres = sceCdSetHDMode(inbuf->m_pkt_0D.m_mode);
}
#endif

#ifdef CDVD_VARIANT_XOSD
static void cdvdfsv_rpc3_2E_xledctl(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdXLEDCtl(
			inbuf->m_pkt_2E.m_arg1, inbuf->m_pkt_2E.m_arg2, &outbuf->m_pkt_2E.m_result1, &outbuf->m_pkt_2E.m_result2);
	}
}
#endif

#ifdef CDVD_VARIANT_XOSD
static void
cdvdfsv_rpc3_39_buzzerctl(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdBuzzerCtl(&outbuf->m_pkt_39.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_XOSD
static void
cdvdfsv_rpc3_3A_resetwakeupreason(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
#if 0
		// TODO FIXME IMPORT
		outbuf->m_retres = sceCdResetWakeupReason((u32 *)inbuf, &outbuf->m_pkt_3A.m_arg2);
#else
		(void)inbuf;
		outbuf->m_retres = 1;
#endif
	}
}
#endif

#ifdef CDVD_VARIANT_XOSD
static void cdvdfsv_rpc3_3B_i_169(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
#if 0
		// TODO FIXME IMPORT
		outbuf->m_retres = cdvdman_169(&outbuf->m_pkt_3B.m_arg1, &outbuf->m_pkt_3B.m_arg2);
#else
		outbuf->m_retres = 1;
#endif
	}
}
#endif

#ifdef CDVD_VARIANT_XOSD
static void
cdvdfsv_rpc3_3C_xbspowerctl(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdXBSPowerCtl(
			inbuf->m_pkt_3C.m_arg1, inbuf->m_pkt_3C.m_arg2, &outbuf->m_pkt_3C.m_result1, &outbuf->m_pkt_3C.m_result2);
	}
}
#endif

#ifdef CDVD_VARIANT_XOSD
static void
cdvdfsv_rpc3_3D_setmediumremoval(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdSetMediumRemoval(inbuf->m_pkt_3D.m_arg1, &outbuf->m_pkt_3D.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_XOSD
static void
cdvdfsv_rpc3_3E_getmediumremoval(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdGetMediumRemoval(&outbuf->m_pkt_3E.m_result1, &outbuf->m_pkt_3E.m_result2);
	}
}
#endif

#ifdef CDVD_VARIANT_XOSD
static void
cdvdfsv_rpc3_3F_xdvrpreset(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdXDVRPReset(inbuf->m_pkt_3F.m_arg1, &outbuf->m_pkt_3F.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_XOSD
static void
cdvdfsv_rpc3_40_getwakeupreason(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = sceCdGetWakeUpReason();
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_0E_openconfig(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdOpenConfig(
			inbuf->m_pkt_0E.m_block, inbuf->m_pkt_0E.m_mode, inbuf->m_pkt_0E.m_NumBlocks, &outbuf->m_pkt_0E.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_0F_closeconfig(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdCloseConfig(&outbuf->m_pkt_0F.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_10_readconfig(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)inbuf;
	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdReadConfig(outbuf->m_pkt_10.m_buffer, &outbuf->m_pkt_10.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_OSD
static void
cdvdfsv_rpc3_11_writeconfig(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;
	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdWriteConfig(inbuf->m_pkt_11.m_buffer, &outbuf->m_pkt_11.m_status);
	}
}
#endif

#ifdef CDVD_VARIANT_DNAS
static void
cdvdfsv_rpc5_11_readdiskid(const cdvdfsv_rpc5_inpacket_t *inbuf, int buflen, cdvdfsv_rpc5_outpacket_t *outbuf)
{
	(void)inbuf;
	(void)buflen;

	outbuf->m_retres = sceCdReadDiskID((unsigned int *)&(outbuf->m_pkt_11.m_diskid));
}

static void
cdvdfsv_rpc5_17_doesuniquekeyexist(const cdvdfsv_rpc5_inpacket_t *inbuf, int buflen, cdvdfsv_rpc5_outpacket_t *outbuf)
{
	(void)inbuf;
	(void)buflen;

	outbuf->m_retres = sceCdDoesUniqueKeyExist(&outbuf->m_pkt_17.m_status);
}
#endif

#ifdef CDVD_VARIANT_OSD
static void cdvdfsv_rpc5_0B_dg(const cdvdfsv_rpc5_inpacket_t *inbuf, int buflen, cdvdfsv_rpc5_outpacket_t *outbuf)
{
	(void)buflen;
	// Unofficial: write directly to output buffer
	outbuf->m_retres = sceCdReadKey(
		inbuf->m_pkt_0B.m_arg1, inbuf->m_pkt_0B.m_arg2, inbuf->m_pkt_0B.m_command, (u8 *)outbuf->m_pkt_0B.m_dg_buf);
}
#endif

#ifdef CDVD_VARIANT_OSD
static void cdvdfsv_rpc3_0A_decset(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	(void)buflen;
	outbuf->m_retres =
		sceCdDecSet(inbuf->m_pkt_0A.m_enable_xor, inbuf->m_pkt_0A.m_enable_shift, inbuf->m_pkt_0A.m_shiftval);
}
#endif

static void
cdvdfsv_rpc3_0B_applyscmd(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	(void)buflen;

	sceCdApplySCmd(
		inbuf->m_pkt_0B.m_cmdNum, &inbuf->m_pkt_0B.m_inBuff, inbuf->m_pkt_0B.m_inBuffSize, &(outbuf->m_pkt_0B.m_outbuf));
}

static void
cdvdfsv_rpc5_0C_applyncmd(const cdvdfsv_rpc5_inpacket_t *inbuf, int buflen, cdvdfsv_rpc5_outpacket_t *outbuf)
{
	(void)buflen;

	outbuf->m_retres = sceCdApplyNCmd(inbuf->m_pkt_0C.m_cmdNum, &inbuf->m_pkt_0C.m_inBuff, inbuf->m_pkt_0C.m_inBuffSize);
	sceCdSync(2);
}

static void cdvdfsv_rpc3_04_geterror(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	(void)inbuf;
	(void)buflen;

	outbuf->m_retres = sceCdGetError();
}

static void cdvdfsv_rpc3_05_trayreq(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	int i;
	u32 efbits;

	(void)buflen;

	outbuf->m_retres = 0;
	for ( i = 0; i < 3 && !outbuf->m_retres; i += 1 )
	{
		WaitEventFlag(g_scmd_evfid, 1, WEF_AND, &efbits);
		outbuf->m_retres = sceCdTrayReq(inbuf->m_pkt_05.m_param, &outbuf->m_pkt_05.m_traychk);
	}
}

static void
cdvdfsv_rpc3_25_settimeout(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	(void)buflen;

	outbuf->m_retres = sceCdSetTimeout(inbuf->m_pkt_25.m_param, inbuf->m_pkt_25.m_timeout);
}

static void
cdvdfsv_rpc3_27_readdvddualinfo(const cdvdfsv_rpc3_inpacket_t *inbuf, int buflen, cdvdfsv_rpc3_outpacket_t *outbuf)
{
	(void)inbuf;
	(void)buflen;

	outbuf->m_retres = sceCdReadDvdDualInfo(&outbuf->m_pkt_27.m_on_dual, &outbuf->m_pkt_27.m_layer1_start);
}

static int cdvdfsv_rpc5_0E_diskready(void)
{
	int is_detecting;
	int scres_unused;

	is_detecting = 0;
	// The following printf was added for ioprp300x
	VERBOSE_PRINTF(1, "sceCdNcmddiskready call\n");
	// The following call to sceCdGetDiskType was inlined
	switch ( sceCdGetDiskType() )
	{
		case SCECdDETCT:
		case SCECdDETCTCD:
		case SCECdDETCTDVDS:
		case SCECdDETCTDVDD:
			is_detecting = 1;
			break;
		default:
			break;
	}
	// The following call to sceCdDiskReady was inlined
	if (
		(sceCdDiskReady(8) & 0xC0) != 0x40 || sceCdSC(0xFFFFFFFD, &scres_unused) || !sceCdSC(0xFFFFFFF4, &scres_unused)
		|| is_detecting )
	{
		VERBOSE_PRINTF(1, "Drive Not Ready\n");
		return 6;
	}
	// The following printf was added for ioprp300x
	VERBOSE_PRINTF(1, "sceCdNcmddiskready call OKend\n");
	return 2;
}

static void *cbrpc_rpc5_cdvdncmds(int fno, void *buffer, int length)
{
	int scres_unused;
	int fno_1;

	fno_1 = fno;
	// Note: in ioprp300x, the following was changed from printf to Kprintf
	VERBOSE_KPRINTF(1, "sce_cdvd N cmd start %d\n", fno);
	g_cdvdfsv_rpc5flg = 1;
	sceCdSC(0xFFFFFFF6, &fno_1);
	switch ( fno )
	{
		case 1:
			// The following call to sceCdGetDiskType was inlined
			cdvdfsv_rpc5_01_readee(
				buffer, length, &g_crr, !(sceCdGetDiskType() ^ SCECdPS2DVD), !sceCdSC(0xFFFFFFFC, &scres_unused), 0);
			break;
		case 2:
			cdvdfsv_rpc5_02_readcdda(buffer, length, &g_crr, 0);
			break;
#ifdef CDVD_VARIANT_OSD
		case 3:
			cdvdfsv_rpc5_03_readdvdv(buffer, length, &g_crr);
			break;
#endif
		case 4:
			cdvdfsv_rpc5_04_gettoc(buffer, length, &g_crr);
			break;
		case 5:
			VERBOSE_PRINTF(1, "Call Seek lsn= %d\n", (int)(((const cdvdfsv_rpc5_inpacket_t *)buffer)->m_pkt_05.m_lbn));
			g_crr.m_retres = sceCdSeek(((const cdvdfsv_rpc5_inpacket_t *)buffer)->m_pkt_05.m_lbn);
			VERBOSE_PRINTF(1, "Call Seek end\n");
			sceCdSync(6);
			break;
		case 6:
			VERBOSE_PRINTF(1, "Call Standby\n");
			g_crr.m_retres = sceCdStandby();
			sceCdSync(4);
			VERBOSE_PRINTF(1, "Call Standby called\n");
			break;
		case 7:
			VERBOSE_PRINTF(1, "Call Stop\n");
			g_crr.m_retres = sceCdStop();
			sceCdSync(4);
			break;
		case 8:
			VERBOSE_PRINTF(1, "Call Pause\n");
			g_crr.m_retres = sceCdPause();
			sceCdSync(6);
			break;
		case 9:
			if ( devctl("cdrom_stm0:", 0x4396, buffer, length, &g_crr.m_retres, sizeof(g_crr.m_retres)) < 0 )
				g_crr.m_retres = 0;
			break;
		case 10:
			if ( devctl("cdrom_stm0:", 0x4398, buffer, length, &g_crr.m_retres, sizeof(g_crr.m_retres)) < 0 )
				g_crr.m_retres = 0;
			break;
#ifdef CDVD_VARIANT_OSD
		case 11:
			cdvdfsv_rpc5_0B_dg(buffer, length, &g_crr);
			break;
#endif
		case 12:
			cdvdfsv_rpc5_0C_applyncmd(buffer, length, &g_crr);
			break;
		case 13:
			cdvdfsv_rpc5_0D_iopmread(buffer, length, &g_crr);
			break;
		case 14:
			g_crr.m_retres = cdvdfsv_rpc5_0E_diskready();
			break;
		case 15:
			cdvdfsv_rpc5_0F_readchain(buffer, length, &g_crr);
			break;
#ifdef CDVD_VARIANT_OSD
		case 16:
			cdvdfsv_rpc5_02_readcdda(buffer, length, &g_crr, 1);
			break;
#endif
#ifdef CDVD_VARIANT_DNAS
		case 17:
			cdvdfsv_rpc5_11_readdiskid(buffer, length, &g_crr);
			break;
#endif
		case 19:
			// The following call to sceCdGetDiskType was inlined
			cdvdfsv_rpc5_01_readee(
				buffer, length, &g_crr, !(sceCdGetDiskType() ^ SCECdPS2DVD), 1, !g_cdvdman_istruct_ptr->m_no_dec_flag);
			break;
#ifdef CDVD_VARIANT_DNAS
		case 23:
			cdvdfsv_rpc5_17_doesuniquekeyexist(buffer, length, &g_crr);
			break;
#endif
		default:
			VERBOSE_PRINTF(1, "sce_cdvd no block IO :unrecognized code %x\n", fno);
			g_crr.m_retres = 0;
			break;
	}
	fno_1 = 0;
	sceCdSC(0xFFFFFFF6, &fno_1);
	g_cdvdfsv_rpc5flg = 0;
	// Note: in ioprp300x, the following was changed from printf to Kprintf
	VERBOSE_KPRINTF(1, "sce_cdvd N cmd end\n");
	return (void *)&g_crr;
}

// cppcheck-suppress constParameterCallback
static void *cbrpc_rpc3_cdvdscmds(int fno, void *buffer, int length)
{
	VERBOSE_PRINTF(1, "sce_cdvd S cmd start %d\n", fno);
	g_cdvdfsv_rpc3flg = 1;
	switch ( fno )
	{
		case 1:
			cdvdfsv_rpc3_01_readclock(buffer, length, &g_outbuf);
			break;
#ifdef CDVD_VARIANT_OSD
		case 2:
			cdvdfsv_rpc3_02_writeclock(buffer, length, &g_outbuf);
			break;
#endif
		case 3:
			cdvdfsv_rpc3_03_disktype(buffer, length, &g_outbuf);
			break;
		case 4:
			cdvdfsv_rpc3_04_geterror(buffer, length, &g_outbuf);
			break;
		case 5:
			cdvdfsv_rpc3_05_trayreq(buffer, length, &g_outbuf);
			break;
		case 6:
			cdvdfsv_rpc3_06_ri(buffer, length, &g_outbuf);
			break;
#ifdef CDVD_VARIANT_OSD
		// Not in XOSD or DVD Player 3.11
		case 7:
			cdvdfsv_rpc3_07_wi(buffer, length, &g_outbuf);
			break;
		case 8:
			cdvdfsv_rpc3_08_readnvm(buffer, length, &g_outbuf);
			break;
		case 9:
			cdvdfsv_rpc3_09_writenvm(buffer, length, &g_outbuf);
			break;
		case 10:
			cdvdfsv_rpc3_0A_decset(buffer, length, &g_outbuf);
			break;
#endif
		case 11:
			cdvdfsv_rpc3_0B_applyscmd(buffer, length, &g_outbuf);
			break;
		case 12:
			cdvdfsv_rpc3_0C_cdstatus(buffer, length, &g_outbuf);
			break;
#ifdef CDVD_VARIANT_OSD
		case 13:
			cdvdfsv_rpc3_0D_sethdmode(buffer, length, &g_outbuf);
			break;
		case 14:
			cdvdfsv_rpc3_0E_openconfig(buffer, length, &g_outbuf);
			break;
		case 15:
			cdvdfsv_rpc3_0F_closeconfig(buffer, length, &g_outbuf);
			break;
		case 16:
			cdvdfsv_rpc3_10_readconfig(buffer, length, &g_outbuf);
			break;
		case 17:
			cdvdfsv_rpc3_11_writeconfig(buffer, length, &g_outbuf);
			break;
		case 18:
			cdvdfsv_rpc3_12_readconsoleid(buffer, length, &g_outbuf);
			break;
		// Not in XOSD or DVD Player 3.11
		case 19:
			cdvdfsv_rpc3_13_writeconsoleid(buffer, length, &g_outbuf);
			break;
		case 20:
			cdvdfsv_rpc3_14_getmversion(buffer, length, &g_outbuf);
			break;
#endif
		case 21:
			cdvdfsv_rpc3_15_ctrladout(buffer, length, &g_outbuf);
			break;
		case 22:
			cdvdfsv_rpc3_16_break(buffer, length, &g_outbuf);
			break;
#ifdef CDVD_VARIANT_OSD
		case 23:
			cdvdfsv_rpc3_17_readsubq(buffer, length, &g_outbuf);
			break;
		case 24:
			cdvdfsv_rpc3_18_forbiddvdp(buffer, length, &g_outbuf);
			break;
		case 25:
			cdvdfsv_rpc3_19_autoadjustctrl(buffer, length, &g_outbuf);
			break;
#endif
		case 26:
			cdvdfsv_rpc3_1A_rm(buffer, length, &g_outbuf);
			break;
#ifdef CDVD_VARIANT_OSD
		// Not in XOSD or DVD Player 3.11
		case 27:
			cdvdfsv_rpc3_1B_wm(buffer, length, &g_outbuf);
			break;
		case 28:
			cdvdfsv_rpc3_1C_forbidread(buffer, length, &g_outbuf);
			break;
		case 29:
			cdvdfsv_rpc3_1D_sc_FFFFFFF8(buffer, length, &g_outbuf);
			break;
		case 30:
			cdvdfsv_rpc3_1E_bootcertify(buffer, length, &g_outbuf);
			break;
		case 31:
			cdvdfsv_rpc3_1F_cancelpoffrdy(buffer, length, &g_outbuf);
			break;
		case 32:
			cdvdfsv_rpc3_20_blueledctl(buffer, length, &g_outbuf);
			break;
#endif
		case 33:
			cdvdfsv_rpc3_21_poweroff(buffer, length, &g_outbuf);
			break;
		case 34:
			cdvdfsv_rpc3_22_mmode(buffer, length, &g_outbuf);
			break;
		case 35:
			cdvdfsv_rpc3_23_changethreadpriority(buffer, length, &g_outbuf);
			break;
#ifdef CDVD_VARIANT_DNAS
		case 36:
			cdvdfsv_rpc3_24_readguid(buffer, length, &g_outbuf);
			break;
#endif
		case 37:
			cdvdfsv_rpc3_25_settimeout(buffer, length, &g_outbuf);
			break;
#ifdef CDVD_VARIANT_DNAS
		case 38:
			cdvdfsv_rpc3_26_readmodelid(buffer, length, &g_outbuf);
			break;
#endif
		case 39:
			cdvdfsv_rpc3_27_readdvddualinfo(buffer, length, &g_outbuf);
			break;
#ifdef CDVD_VARIANT_OSD
		case 40:
			cdvdfsv_rpc3_28_writewakeuptime(buffer, length, &g_outbuf);
			break;
		case 41:
			cdvdfsv_rpc3_29_readwakeuptime(buffer, length, &g_outbuf);
			break;
		case 42:
			cdvdfsv_rpc3_2A_readps1bootparam(buffer, length, &g_outbuf);
			break;
		case 43:
			cdvdfsv_rpc3_2B_setfanprofile(buffer, length, &g_outbuf);
			break;
		case 44:
			cdvdfsv_rpc3_2C_i_152(buffer, length, &g_outbuf);
			break;
#endif
#ifdef CDVD_VARIANT_XOSD
		case 45:
			cdvdfsv_rpc3_2D_chgsys(buffer, length, &g_outbuf);
			break;
		case 46:
			cdvdfsv_rpc3_2E_xledctl(buffer, length, &g_outbuf);
			break;
		case 47:
			cdvdfsv_rpc3_2F_noticegamestart(buffer, length, &g_outbuf);
			break;
#endif
#ifdef CDVD_VARIANT_OSD
		case 48:
			cdvdfsv_rpc3_30_rcbypassctl(buffer, length, &g_outbuf);
			break;
		case 49:
			cdvdfsv_rpc3_31_remote2_7(buffer, length, &g_outbuf);
			break;
		case 50:
			cdvdfsv_rpc3_32_sendscmd1d(buffer, length, &g_outbuf);
			break;
		case 53:
			cdvdfsv_rpc3_35_setledsmode(buffer, length, &g_outbuf);
			break;
		case 56:
			cdvdfsv_rpc3_38_remote2_7_get(buffer, length, &g_outbuf);
			break;
#endif
#ifdef CDVD_VARIANT_XOSD
		case 57:
			cdvdfsv_rpc3_39_buzzerctl(buffer, length, &g_outbuf);
			break;
		case 58:
			cdvdfsv_rpc3_3A_resetwakeupreason(buffer, length, &g_outbuf);
			break;
		case 59:
			cdvdfsv_rpc3_3B_i_169(buffer, length, &g_outbuf);
			break;
		case 60:
			cdvdfsv_rpc3_3C_xbspowerctl(buffer, length, &g_outbuf);
			break;
		case 61:
			cdvdfsv_rpc3_3D_setmediumremoval(buffer, length, &g_outbuf);
			break;
		case 62:
			cdvdfsv_rpc3_3E_getmediumremoval(buffer, length, &g_outbuf);
			break;
		case 63:
			cdvdfsv_rpc3_3F_xdvrpreset(buffer, length, &g_outbuf);
			break;
		case 64:
			cdvdfsv_rpc3_40_getwakeupreason(buffer, length, &g_outbuf);
			break;
#endif
#ifdef CDVD_VARIANT_OSD
		case 67:
			cdvdfsv_rpc3_43_readregionparams(buffer, length, &g_outbuf);
			break;
		// Not in XOSD or DVD Player 3.11
		case 68:
			cdvdfsv_rpc3_44_writeregionparams(buffer, length, &g_outbuf);
			break;
#endif
		default:
			VERBOSE_PRINTF(1, "sce_cdvd block IO :unrecognized code 0x%02x\n", fno);
			g_outbuf.m_retres = 0;
			break;
	}
	VERBOSE_PRINTF(1, "sce_cdvd S cmd end\n");
	g_cdvdfsv_rpc3flg = 0;
	return (void *)&g_outbuf;
}

static void cdvdfsv_poffloop(void)
{
	int trid;
	char cmdpkt[16];
	int scres;
	u32 efbits;

	g_cdvdman_intr_evfid = sceCdSC(0xFFFFFFF5, &scres);
	while ( 1 )
	{
		ClearEventFlag(g_cdvdman_intr_evfid, ~4);
		WaitEventFlag(g_cdvdman_intr_evfid, 4, WEF_AND, &efbits);
		if ( g_cdvdfsv_nopocm )
			break;
		if ( !g_cdvdfsv_plbreak )
		{
			while ( 1 )
			{
				trid = sceSifSendCmd(0x80000012, cmdpkt, sizeof(cmdpkt), 0, 0, 0);
				if ( trid )
					break;
				DelayThread(500);
			}
			while ( cdvdfsv_checkdmastat(trid) >= 0 )
				;
		}
	}
}

#ifdef CDVD_VARIANT_OSD
static void cdvdfsv_rpc4_th(void *arg)
{
	int trid;
	char cmdpkt[16];
	int scres;
	u32 efbits;

	(void)arg;
	g_cdvdman_intr_evfid = sceCdSC(0xFFFFFFF5, &scres);
	while ( 1 )
	{
		ClearEventFlag(g_cdvdman_intr_evfid, ~0x40);
		WaitEventFlag(g_cdvdman_intr_evfid, 0x40, WEF_AND, &efbits);
		if ( g_cdvdfsv_nopocm )
			break;
		if ( !g_cdvdfsv_plbreak )
		{
			while ( 1 )
			{
				trid = sceSifSendCmd(0x80000015, cmdpkt, sizeof(cmdpkt), 0, 0, 0);
				if ( trid )
					break;
				DelayThread(500);
			}
			while ( cdvdfsv_checkdmastat(trid) >= 0 )
				;
		}
	}
	ExitDeleteThread();
}
#endif

#ifdef CDVD_VARIANT_OSD
static void cdvdfsv_rpc5_th(void *arg)
{
	int trid;
	char cmdpkt[16];
	int scres;
	u32 efbits;

	(void)arg;
	g_cdvdman_intr_evfid = sceCdSC(0xFFFFFFF5, &scres);
	while ( 1 )
	{
		ClearEventFlag(g_cdvdman_intr_evfid, ~0x80);
		WaitEventFlag(g_cdvdman_intr_evfid, 0x80, WEF_AND, &efbits);
		if ( g_cdvdfsv_nopocm )
			break;
		if ( !g_cdvdfsv_plbreak )
		{
			while ( 1 )
			{
				trid = sceSifSendCmd(0x80000016, cmdpkt, sizeof(cmdpkt), 0, 0, 0);
				if ( trid )
					break;
				DelayThread(500);
			}
			while ( cdvdfsv_checkdmastat(trid) >= 0 )
				;
		}
	}
	ExitDeleteThread();
}
#endif

static void cdvdfsv_rpc1_th(void *arg)
{
	(void)arg;

	sceSifSetRpcQueue(&g_rpc_qdata1, GetThreadId());
	sceSifRegisterRpc(&g_rpc_sdata1, 0x80000592, cbrpc_rpc1_cdinit, g_rpc_buffer1, 0, 0, &g_rpc_qdata1);
	sceSifRegisterRpc(&g_rpc_sdata2, 0x8000059A, cbrpc_rpc2_diskready, g_rpc_buffer2, 0, 0, &g_rpc_qdata1);
	sceSifRegisterRpc(&g_rpc_sdata3, 0x80000593, cbrpc_rpc3_cdvdscmds, g_rpc_buffer3, 0, 0, &g_rpc_qdata1);
	sceSifRpcLoop(&g_rpc_qdata1);
	ExitDeleteThread();
}

static void cdvdfsv_rpc3_th(void *arg)
{
	(void)arg;

	sceSifSetRpcQueue(&g_rpc_qdata3, GetThreadId());
	sceSifRegisterRpc(&g_rpc_sdata6, 0x8000059C, cbrpc_rpc2_diskready, g_rpc_buffer2, 0, 0, &g_rpc_qdata3);
	sceSifRpcLoop(&g_rpc_qdata3);
	ExitDeleteThread();
}

static void cdvdfsv_rpc2_th(void *arg)
{
	(void)arg;

	sceSifSetRpcQueue(&g_rpc_qdata2, GetThreadId());
	sceSifRegisterRpc(&g_rpc_sdata4, 0x80000597, cbrpc_rpc4_fscall, g_rpc_buffer4, 0, 0, &g_rpc_qdata2);
	sceSifRegisterRpc(&g_rpc_sdata5, 0x80000595, cbrpc_rpc5_cdvdncmds, g_rpc_buffer5, 0, 0, &g_rpc_qdata2);
	sceSifRpcLoop(&g_rpc_qdata2);
	ExitDeleteThread();
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
