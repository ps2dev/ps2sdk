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

#include <cdvd-ioctl.h>
#include <errno.h>
#include <kerr.h>

IRX_ID("cdvd_st_driver", 2, 2);
// Based on the module from SCE SDK 3.1.0.

extern struct irx_export_table _exp_cdvdstm;

int cdvdstm_dummyentry();
int cdvdstm_termcall();
static unsigned int iop_stream_handler(
	unsigned int posszarg1, unsigned int posszarg2, void *buffer, int cmdid, const sceCdRMode *rmode, int *error_ptr);
static unsigned int iop_stream_intr_cb(void *userdata);
static int cdrom_stm_init(iop_device_t *device);
static int cdrom_stm_deinit(iop_device_t *device);
static int cdrom_stm_devctl(
	iop_file_t *f, const char *name, int cmd, void *inbuf, unsigned int inbuf_len, void *outbuf, unsigned int outbuf_len);
static void ee_stream_handler_normal(cdrom_stm_devctl_t *instruct, int inbuf_len, int *outres_ptr);
static unsigned int ee_stream_intr_cb_normal(void *userdata);
static void ee_stream_handler_cdda(cdrom_stm_devctl_t *instruct, int inbuf_len, int *outres_ptr);
static unsigned int ee_stream_intr_cb_cdda(void *userdata);
unsigned int optimized_memcpy(char *dst, const char *src, unsigned int n);

static void iop_stream_intr_cb_thunk(int userdata)
{
	iop_stream_intr_cb((void *)userdata);
}

static void ee_stream_intr_cb_normal_thunk(int userdata)
{
	ee_stream_intr_cb_normal((void *)userdata);
}

static void ee_stream_intr_cb_cdda_thunk(int userdata)
{
	ee_stream_intr_cb_cdda((void *)userdata);
}

static int g_verbose_level = 0;
static int g_cdvdstm_in_deldrv = 0;
static int g_cdvdstm_bufmax = 0;
static int g_cdvdstm_numbytes = 0;
static int g_cdvdstm_bankmax = 0;
static void *g_cdvdstm_buffer = NULL;
static unsigned int g_cdvdstm_sectorcount = 0;
static int g_cdvdstm_last_error_for_iop = 0;
static int g_cdvdstm_retryerr_iop = 0;
static int g_cdvdstm_retrycnt_iop = 0;

IOMANX_RETURN_VALUE_IMPL(EIO);

static iop_device_ops_t g_cdrom_stm_dev_ops = {
	&cdrom_stm_init, // init,
	&cdrom_stm_deinit, // deinit,
	IOMANX_RETURN_VALUE(EIO), // format,
	IOMANX_RETURN_VALUE(EIO), // open,
	IOMANX_RETURN_VALUE(EIO), // close,
	IOMANX_RETURN_VALUE(EIO), // read,
	IOMANX_RETURN_VALUE(EIO), // write,
	IOMANX_RETURN_VALUE(EIO), // lseek,
	IOMANX_RETURN_VALUE(EIO), // ioctl,
	IOMANX_RETURN_VALUE(EIO), // remove,
	IOMANX_RETURN_VALUE(EIO), // mkdir,
	IOMANX_RETURN_VALUE(EIO), // rmdir,
	IOMANX_RETURN_VALUE(EIO), // dopen,
	IOMANX_RETURN_VALUE(EIO), // dclose,
	IOMANX_RETURN_VALUE(EIO), // dread,
	IOMANX_RETURN_VALUE(EIO), // getstat,
	IOMANX_RETURN_VALUE(EIO), // chstat,
	IOMANX_RETURN_VALUE(EIO), // rename,
	IOMANX_RETURN_VALUE(EIO), // chdir,
	IOMANX_RETURN_VALUE(EIO), // sync,
	IOMANX_RETURN_VALUE(EIO), // mount,
	IOMANX_RETURN_VALUE(EIO), // umount,
	IOMANX_RETURN_VALUE_S64(EIO), // lseek64,
	&cdrom_stm_devctl,
	IOMANX_RETURN_VALUE(EIO), // symlink
	IOMANX_RETURN_VALUE(EIO), // readlink
	IOMANX_RETURN_VALUE(EIO), // ioctl2
};
static iop_device_t g_cdrom_stm_dev = {"cdrom_stm", IOP_DT_FSEXT | IOP_DT_FS, 1, "CD-ROM_STM ", &g_cdrom_stm_dev_ops};
static int g_cdvdstm_last_error_for_ee = 0;
static int g_cdvdstm_bufsz2 = 0;
static int g_cdvdstm_chunksz2 = 0;
static int g_cdvdstm_bankcnt2 = 0;
static void *g_cdvdstm_buffer2 = NULL;
static u32 g_cdvdstm_sectorcount2 = 0;
static int g_cdvdstm_retryerr_ee = 0;
static int g_cdvdstm_retrycnt_ee_normal = 0;
static int g_cdvdstm_usedchunksize2 = 0x930;
static u32 g_cdvdstm_retrycnt_ee_cdda = 0;
static sceCdRMode g_rmode_for_stream0;
static int g_cdvdstm_tgt;
static int g_cdvdstm_semid;
static int g_cdvdman_intr_efid;
static char g_cdvdstm_usedmap_iop[512];
static unsigned int g_cdvdstm_lsn_iop;
static int g_cdvdstm_bankgp_iop;
static int g_cdvdstm_bankcur_iop;
static int g_cdvdstm_bankoffs_iop;
static sceCdRMode g_cdvdstm_mode_iop;
static int g_cdvdstm_stmstart_iop;
static iop_sys_clock_t g_cdvdstm_curclk_iop;
static SifDmaTransfer_t g_cdvdstm_dmat;
static int g_cdvdstm_readlbn_ee_normal;
static SifDmaTransfer_t g_cdvdstm_dmat2;
static u32 g_cdvdstm_readlbn_ee_cdda;
static char g_cdvdstm_usedmap_ee[512];
static u32 g_cdvdstm_lsn_ee;
static int g_cdvdstm_bankgp_ee;
static int g_cdvdstm_bankcur_ee;
static int g_cdvdstm_bankoffs_ee;
static sceCdRMode g_cdvdstm_mode_ee;
static int g_cdvdstm_stmstart_ee;
static iop_sys_clock_t g_cdvdstm_curclk_ee;

static int vCancelAlarm(unsigned int (*alarm_cb)(void *), void *arg)
{
	return (QueryIntrContext() ? iCancelAlarm : CancelAlarm)(alarm_cb, arg);
}

static int vSetEventFlag()
{
	return (QueryIntrContext() ? iSetEventFlag : SetEventFlag)(g_cdvdman_intr_efid, 8);
}

static int vClearEventFlag()
{
	return (QueryIntrContext() ? iClearEventFlag : ClearEventFlag)(g_cdvdman_intr_efid, ~8);
}

int cdvdstm_dummyentry()
{
	VERBOSE_PRINTF(1, "Dummy Entry Called\n");
	return 0;
}

int cdvdstm_termcall()
{
	cdrom_stm_devctl_t instruct;
	int outres;

	memset(&instruct, 0, sizeof(instruct));
	instruct.m_cmdid = 3;
	instruct.m_rmode.datapattern = SCECdSecS2048;
	instruct.m_rmode.spindlctrl = SCECdSpinMax;
	instruct.m_rmode.trycount = 0;
	ee_stream_handler_normal(&instruct, 0x14, &outres);
	sceCdStStop();
	return 0;
}

static int stm_iop_read_timeout_alarm_cb(const iop_sys_clock_t *sys_clock)
{
	KPRINTF("Stm Iop Read Time Out %d(msec)\n", sys_clock->lo / 0x9000);
	return !sceCdBreak();
}

static int sceCdStream0_inner(unsigned int rdsize, char *addrarg, int modearg, int *error_ptr)
{
	int cur_size;
	unsigned int streamres;
	int last_error;
	u32 efbits;
	int err;

	VERBOSE_KPRINTF(1, "sceCdStream0 call read size= %d mode= %d addr= %08x\n", rdsize, modearg, addrarg);
	cur_size = 0;
	if ( !sceCdSC(0xFFFFFFFF, &last_error) )
		return 0;
	*error_ptr = 0;
	if ( !modearg )
		return iop_stream_handler(0, rdsize, addrarg, 2, &g_rmode_for_stream0, error_ptr);
	vSetEventFlag();
	err = 0;
	streamres = 0;
	while ( (!err || streamres) )
	{
		WaitEventFlag(g_cdvdman_intr_efid, 8, WEF_AND, &efbits);
		streamres = iop_stream_handler(0, rdsize - cur_size, &addrarg[cur_size], 2, &g_rmode_for_stream0, &err);
		if ( rdsize - cur_size != streamres )
			vClearEventFlag();
		cur_size += streamres;
		if ( err )
			*error_ptr = err;
		VERBOSE_KPRINTF(1, "sceCdStream0 BLK cur_size= %d req_size= %d err 0x%x\n", cur_size, rdsize, err);
		if ( (unsigned int)cur_size == rdsize )
			break;
	}
	return cur_size;
}

static int sceCdStream0(int rdsize_sectors, char *addrarg, int modearg, int *error_ptr)
{
	return sceCdStream0_inner(rdsize_sectors << 11, addrarg, modearg, error_ptr) / 0x800;
}

static unsigned int iop_stream_handler(
	unsigned int posszarg1, unsigned int posszarg2, void *buffer, int cmdid, const sceCdRMode *rmode, int *error_ptr)
{
	int retryflag;
	int bankcur_tmp;
	unsigned int i;
	unsigned int written_chunk_size_tmp;
	int bankcur_next_tmp1;
	int chunk_size;
	int bankcur_next_tmp2;
	int state;

	retryflag = 0;
	VERBOSE_KPRINTF(1, "CD Stream Call mode= %d\n", cmdid);
	*error_ptr = 0;
	if ( g_cdvdstm_stmstart_iop == 2 && cmdid != 9 && cmdid != 3 )
	{
		return 0;
	}
	switch ( cmdid )
	{
		case 8:
			sceCdSC(1, &g_cdvdstm_last_error_for_iop);
			if ( sceCdNop() )
				return 1;
			sceCdSC(0, &g_cdvdstm_last_error_for_iop);
			return 0;
		case 7:
			CpuSuspendIntr(&state);
			vCancelAlarm((unsigned int (*)(void *))iop_stream_intr_cb, &g_cdvdstm_curclk_iop);
			sceCdSC(0, &g_cdvdstm_last_error_for_iop);
			CpuResumeIntr(state);
			sceCdSync(0);
			vCancelAlarm((unsigned int (*)(void *))stm_iop_read_timeout_alarm_cb, &g_cdvdstm_curclk_iop);
			return 1;
		case 6:
			bankcur_tmp = g_cdvdstm_bankcur_iop;
			if ( !g_cdvdstm_usedmap_iop[bankcur_tmp] )
			{
				bankcur_tmp += 1;
				if ( (unsigned int)bankcur_tmp >= (unsigned int)g_cdvdstm_bankmax )
					bankcur_tmp = 0;
				if ( !g_cdvdstm_usedmap_iop[bankcur_tmp] )
					bankcur_tmp = g_cdvdstm_bankcur_iop;
			}
			for ( i = 0; (i < (unsigned int)g_cdvdstm_bankmax) && g_cdvdstm_usedmap_iop[bankcur_tmp]
									 && (g_cdvdstm_bankgp_iop != bankcur_tmp);
						i += 1 )
			{
				bankcur_tmp += 1;
				if ( (unsigned int)bankcur_tmp >= (unsigned int)g_cdvdstm_bankmax )
					bankcur_tmp = 0;
			}
			return i * ((unsigned int)g_cdvdstm_numbytes >> 11);
		case 5:
			sceCdstm0Cb((void (*)(int))iop_stream_intr_cb_thunk);
			g_cdvdstm_bufmax = posszarg1;
			g_cdvdstm_sectorcount = posszarg1 / posszarg2;
			g_cdvdstm_numbytes = g_cdvdstm_sectorcount << 11;
			g_cdvdstm_buffer = buffer;
			g_cdvdstm_bankmax = posszarg2;
			KPRINTF(
				"Stream Buffer 1Bank %dbyte %dbanks addr:%08x %dbyte used.\n",
				g_cdvdstm_numbytes,
				posszarg2,
				buffer,
				g_cdvdstm_numbytes * posszarg2);
			return 1;
		case 3:
			CpuSuspendIntr(&state);
			g_cdvdstm_stmstart_iop = 0;
			vCancelAlarm((unsigned int (*)(void *))iop_stream_intr_cb, &g_cdvdstm_curclk_iop);
			sceCdSC(0, &g_cdvdstm_last_error_for_iop);
			CpuResumeIntr(state);
			sceCdBreak();
			for ( i = 0; i < (unsigned int)g_cdvdstm_bankmax; i += 1 )
				g_cdvdstm_usedmap_iop[i] = 0;
			g_cdvdstm_bankoffs_iop = 0;
			g_cdvdstm_bankcur_iop = 0;
			g_cdvdstm_bankgp_iop = 0;
			sceCdSync(0);
			g_cdvdstm_last_error_for_iop = 0;
			sceCdSC(0xFFFFFFFE, &g_cdvdstm_last_error_for_iop);
			vCancelAlarm((unsigned int (*)(void *))stm_iop_read_timeout_alarm_cb, &g_cdvdstm_curclk_iop);
			return 1;
		case 9:
			if ( sceCdSC(0xFFFFFFFF, &g_cdvdstm_last_error_for_iop) )
			{
				CpuSuspendIntr(&state);
				g_cdvdstm_lsn_iop = posszarg1;
				for ( i = 0; i < (unsigned int)g_cdvdstm_bankmax; i += 1 )
					g_cdvdstm_usedmap_iop[i] = 0;
				g_cdvdstm_stmstart_iop = 2;
				CpuResumeIntr(state);
				return 1;
			}
			return 0;
		case 1:
			g_cdvdstm_mode_iop.datapattern = rmode->datapattern;
			g_cdvdstm_mode_iop.trycount = rmode->trycount;
			g_cdvdstm_mode_iop.spindlctrl = rmode->spindlctrl;
			g_cdvdstm_retryerr_iop = 0;
			break;
		case 4:
			CpuSuspendIntr(&state);
			vCancelAlarm((unsigned int (*)(void *))iop_stream_intr_cb, &g_cdvdstm_curclk_iop);
			sceCdSC(0, &g_cdvdstm_last_error_for_iop);
			retryflag = 1;
			CpuResumeIntr(state);
			posszarg2 = 0;
			cmdid = 1;
			g_cdvdstm_lsn_iop = posszarg1;
			g_cdvdstm_bankoffs_iop = 0;
			g_cdvdstm_bankcur_iop = 0;
			g_cdvdstm_bankgp_iop = 0;
			sceCdSync(0);
			vCancelAlarm((unsigned int (*)(void *))stm_iop_read_timeout_alarm_cb, &g_cdvdstm_curclk_iop);
			break;
	}
	if ( cmdid == 1 )
	{
		CpuSuspendIntr(&state);
		retryflag = 1;
		vCancelAlarm((unsigned int (*)(void *))iop_stream_intr_cb, &g_cdvdstm_curclk_iop);
		sceCdSC(0, &g_cdvdstm_last_error_for_iop);
		CpuResumeIntr(state);
		for ( i = 0; i < (unsigned int)g_cdvdstm_bankmax; i += 1 )
			g_cdvdstm_usedmap_iop[i] = 0;
		g_cdvdstm_lsn_iop = posszarg1;
		sceCdSC(0xFFFFFFE9, (int *)&g_cdvdstm_lsn_iop);
		g_cdvdstm_bankoffs_iop = 0;
		g_cdvdstm_bankcur_iop = 0;
		g_cdvdstm_bankgp_iop = 0;
		sceCdSync(0);
		vCancelAlarm((unsigned int (*)(void *))stm_iop_read_timeout_alarm_cb, &g_cdvdstm_curclk_iop);
		g_cdvdstm_stmstart_iop = 1;
		sceCdSC(1, &g_cdvdstm_last_error_for_iop);
		if ( !sceCdNop() )
		{
			sceCdSC(0, &g_cdvdstm_last_error_for_iop);
			return 0;
		}
	}
	CpuSuspendIntr(&state);
	written_chunk_size_tmp = -1;
	for ( i = 0; i < posszarg2; i += chunk_size )
	{
		if ( !g_cdvdstm_usedmap_iop[g_cdvdstm_bankcur_iop] )
		{
			VERBOSE_KPRINTF(
				1,
				"CD read buffer over run %d %d %d %d %d gp %d pp %d\n",
				(u8)g_cdvdstm_usedmap_iop[0],
				(u8)g_cdvdstm_usedmap_iop[1],
				(u8)g_cdvdstm_usedmap_iop[2],
				(u8)g_cdvdstm_usedmap_iop[3],
				(u8)g_cdvdstm_usedmap_iop[4],
				g_cdvdstm_bankgp_iop,
				g_cdvdstm_bankcur_iop);
			bankcur_next_tmp1 = g_cdvdstm_bankcur_iop;
			g_cdvdstm_bankcur_iop += 1;
			if ( (unsigned int)g_cdvdstm_bankcur_iop >= (unsigned int)g_cdvdstm_bankmax )
				g_cdvdstm_bankcur_iop = 0;
			written_chunk_size_tmp = i;
			if ( !g_cdvdstm_usedmap_iop[g_cdvdstm_bankcur_iop] )
			{
				g_cdvdstm_bankcur_iop = bankcur_next_tmp1;
			}
			break;
		}
		if ( (unsigned int)g_cdvdstm_bankoffs_iop >= (unsigned int)g_cdvdstm_numbytes )
		{
			g_cdvdstm_bankoffs_iop = 0;
			g_cdvdstm_usedmap_iop[g_cdvdstm_bankcur_iop] = 0;
			bankcur_next_tmp2 = g_cdvdstm_bankcur_iop;
			g_cdvdstm_bankcur_iop += 1;
			if ( (unsigned int)g_cdvdstm_bankcur_iop >= (unsigned int)g_cdvdstm_bankmax )
				g_cdvdstm_bankcur_iop = 0;
			if ( !g_cdvdstm_usedmap_iop[g_cdvdstm_bankcur_iop] || g_cdvdstm_bankgp_iop == g_cdvdstm_bankcur_iop )
			{
				g_cdvdstm_bankcur_iop = bankcur_next_tmp2;
				written_chunk_size_tmp = i;
				VERBOSE_KPRINTF(
					1,
					"CD read buffer over run %d %d %d %d %d gp %d pp %d\n",
					(u8)g_cdvdstm_usedmap_iop[0],
					(u8)g_cdvdstm_usedmap_iop[1],
					(u8)g_cdvdstm_usedmap_iop[2],
					(u8)g_cdvdstm_usedmap_iop[3],
					(u8)g_cdvdstm_usedmap_iop[4],
					g_cdvdstm_bankgp_iop,
					g_cdvdstm_bankcur_iop);
				break;
			}
		}
		optimized_memcpy(
			&((char *)buffer)[i],
			&((char *)g_cdvdstm_buffer)[g_cdvdstm_bankcur_iop * g_cdvdstm_numbytes + g_cdvdstm_bankoffs_iop],
			0x800);
		chunk_size = ((unsigned int)0x800 > posszarg2 - i) ? (posszarg2 - i) : 0x800;
		g_cdvdstm_bankoffs_iop += chunk_size;
	}
	if ( written_chunk_size_tmp == 0xFFFFFFFF )
		written_chunk_size_tmp = posszarg2;
	CpuResumeIntr(state);
	if ( !retryflag )
	{
		if ( sceCdSC(0xFFFFFFFF, &g_cdvdstm_last_error_for_iop) != 1 && !written_chunk_size_tmp && !g_cdvdstm_retryerr_iop )
		{
			g_cdvdstm_retryerr_iop = 273;
		}
		*error_ptr = g_cdvdstm_retryerr_iop;
		g_cdvdstm_retryerr_iop = 0;
		return written_chunk_size_tmp;
	}
	return 1;
}

static unsigned int iop_stream_intr_cb(void *userdata)
{
	int last_error;
	int scres_unused;

	(void)userdata;

	VERBOSE_KPRINTF(1, "Intr Read call\n");
	iCancelAlarm((unsigned int (*)(void *))stm_iop_read_timeout_alarm_cb, &g_cdvdstm_curclk_iop);
	iCancelAlarm((unsigned int (*)(void *))iop_stream_intr_cb, &g_cdvdstm_curclk_iop);
	sceCdSC(0xFFFFFFFF, &last_error);
	if ( !last_error )
	{
		switch ( sceCdGetDiskType() )
		{
			case SCECdPSCD:
			case SCECdPSCDDA:
			case SCECdPS2CD:
			case SCECdPS2CDDA:
			case SCECdPS2DVD:
				break;
			default:
				last_error = SCECdErREADCF;
				break;
		}
	}
	g_cdvdstm_curclk_iop.hi = 0;
	if ( g_cdvdstm_stmstart_iop )
		g_cdvdstm_retrycnt_iop = 0;
	if ( g_cdvdstm_stmstart_iop || last_error || g_cdvdstm_retrycnt_iop )
	{
		VERBOSE_KPRINTF(
			1, "Stm Rtry stmstart:%d err:%02x retry:%d\n", g_cdvdstm_stmstart_iop, last_error, g_cdvdstm_retrycnt_iop);
	}
	g_cdvdstm_curclk_iop.lo = (g_cdvdstm_stmstart_iop || last_error || g_cdvdstm_retrycnt_iop) ?
															0x20f58000 :
															(0x9000 * sceCdSC(0xFFFFFFEF, &scres_unused));
	if ( last_error )
	{
		VERBOSE_KPRINTF(1, "IOP Stream read Error code= 0x%02x retry= %d\n", last_error, g_cdvdstm_retrycnt_iop);
		if ( last_error == SCECdErREAD || last_error == SCECdErABRT )
		{
			VERBOSE_KPRINTF(1, "On err %08x\n", last_error);
			if ( g_cdvdstm_retrycnt_iop )
				g_cdvdstm_retryerr_iop = last_error;
			g_cdvdstm_retrycnt_iop = 3;
		}
		else
		{
			g_cdvdstm_retryerr_iop = last_error;
			g_cdvdstm_retrycnt_iop = 1;
		}
	}
	if ( g_cdvdstm_retrycnt_iop )
	{
		unsigned int tgttmp;

		g_cdvdstm_retrycnt_iop -= 1;
		tgttmp = ((unsigned int)g_cdvdstm_tgt >= (unsigned int)(0x10 * g_cdvdstm_retrycnt_iop)) ?
							 (g_cdvdstm_tgt - 0x10 * g_cdvdstm_retrycnt_iop) :
							 (g_cdvdstm_tgt + 0x10 * g_cdvdstm_retrycnt_iop);
		VERBOSE_KPRINTF(1, "Stm Rtry Start Tgt=%d Cur= %d\n", g_cdvdstm_tgt, tgttmp);
		if ( sceCdRE(
					 tgttmp,
					 g_cdvdstm_sectorcount,
					 (char *)g_cdvdstm_buffer + g_cdvdstm_bankgp_iop * g_cdvdstm_numbytes,
					 &g_cdvdstm_mode_iop) )
		{
			iSetAlarm(&g_cdvdstm_curclk_iop, (unsigned int (*)(void *))stm_iop_read_timeout_alarm_cb, &g_cdvdstm_curclk_iop);
		}
		else
		{
			g_cdvdstm_curclk_iop.lo = 0x708000;
			if (
				iSetAlarm(&g_cdvdstm_curclk_iop, (unsigned int (*)(void *))iop_stream_intr_cb, &g_cdvdstm_curclk_iop)
				&& !sceCdNop() )
			{
				sceCdSC(0, &last_error);
			}
			g_cdvdstm_retrycnt_iop += 1;
		}
		return 0;
	}
	else
	{
		int gptmp;

		if ( !g_cdvdstm_stmstart_iop )
		{
			g_cdvdstm_usedmap_iop[g_cdvdstm_bankgp_iop] = 1;
			gptmp = g_cdvdstm_bankgp_iop;
			g_cdvdstm_bankgp_iop += 1;
			if ( (unsigned int)g_cdvdstm_bankgp_iop >= (unsigned int)g_cdvdstm_bankmax )
				g_cdvdstm_bankgp_iop = 0;
		}
		if (
			!g_cdvdstm_stmstart_iop
			&& (g_cdvdstm_usedmap_iop[g_cdvdstm_bankgp_iop] || g_cdvdstm_bankcur_iop == g_cdvdstm_bankgp_iop) )
		{
			g_cdvdstm_bankgp_iop = gptmp;
			g_cdvdstm_usedmap_iop[gptmp] = 0;
			VERBOSE_KPRINTF(
				1,
				"read Full %d %d %d %d %d gp %d pp %d spn %d\n",
				(u8)g_cdvdstm_usedmap_iop[0],
				(u8)g_cdvdstm_usedmap_iop[1],
				(u8)g_cdvdstm_usedmap_iop[2],
				(u8)g_cdvdstm_usedmap_iop[3],
				(u8)g_cdvdstm_usedmap_iop[4],
				g_cdvdstm_bankgp_iop,
				g_cdvdstm_bankcur_iop,
				g_cdvdstm_mode_iop.spindlctrl);
			g_cdvdstm_curclk_iop.lo = 0x48000;
			if (
				iSetAlarm(&g_cdvdstm_curclk_iop, (unsigned int (*)(void *))iop_stream_intr_cb, &g_cdvdstm_curclk_iop)
				&& !sceCdNop() )
			{
				sceCdSC(0, &last_error);
			}
		}
		else
		{
			if ( g_cdvdstm_stmstart_iop == 2 )
			{
				unsigned int i;

				g_cdvdstm_bankoffs_iop = 0;
				g_cdvdstm_bankcur_iop = 0;
				g_cdvdstm_bankgp_iop = 0;
				for ( i = 0; i < (unsigned int)g_cdvdstm_bankmax; i += 1 )
					g_cdvdstm_usedmap_iop[i] = 0;
			}
			g_cdvdstm_stmstart_iop = 0;
			g_cdvdstm_tgt = g_cdvdstm_lsn_iop;
			if ( sceCdRE(
						 g_cdvdstm_lsn_iop,
						 g_cdvdstm_sectorcount,
						 (char *)g_cdvdstm_buffer + g_cdvdstm_bankgp_iop * g_cdvdstm_numbytes,
						 &g_cdvdstm_mode_iop) )
			{
				iSetAlarm(
					&g_cdvdstm_curclk_iop, (unsigned int (*)(void *))stm_iop_read_timeout_alarm_cb, &g_cdvdstm_curclk_iop);
			}
			else
			{
				g_cdvdstm_curclk_iop.lo = 0x708000;
				if (
					iSetAlarm(&g_cdvdstm_curclk_iop, (unsigned int (*)(void *))iop_stream_intr_cb, &g_cdvdstm_curclk_iop)
					&& !sceCdNop() )
				{
					sceCdSC(0, &last_error);
				}
				g_cdvdstm_retrycnt_iop = 1;
			}
			g_cdvdstm_lsn_iop += g_cdvdstm_sectorcount;
		}
	}
	return 0;
}

static int cdrom_stm_init(iop_device_t *device)
{
	(void)device;
	iop_sema_t semaparam;

	semaparam.attr = SA_THPRI;
	semaparam.initial = 1;
	semaparam.max = 1;
	semaparam.option = 0;
	g_cdvdstm_semid = CreateSema(&semaparam);
	return 0;
}

static int cdrom_stm_deinit(iop_device_t *device)
{
	(void)device;
	SignalSema(g_cdvdstm_semid);
	DeleteSema(g_cdvdstm_semid);
	return 0;
}

static int cdrom_stm_devctl(
	iop_file_t *f, const char *name, int cmd, void *inbuf, unsigned int inbuf_len, void *outbuf, unsigned int outbuf_len)
{
	int retres;
	cdrom_stm_devctl_t *instruct;
	int *outres_ptr;

	(void)f;
	(void)name;
	(void)outbuf_len;

	instruct = inbuf;
	outres_ptr = outbuf;
	WaitSema(g_cdvdstm_semid);
	retres = 0;
	if ( g_cdvdstm_in_deldrv )
	{
		SignalSema(g_cdvdstm_semid);
		return -EIO;
	}
	switch ( cmd )
	{
		case 0x4393:
			if ( instruct->m_cmdid == 5 || instruct->m_cmdid == 3 || instruct->m_cmdid - 7 < 2 )
			{
				vSetEventFlag();
			}
			*outres_ptr = iop_stream_handler(
				instruct->m_posszarg1,
				instruct->m_posszarg2,
				instruct->m_buffer,
				instruct->m_cmdid,
				&instruct->m_rmode,
				(int *)&instruct->m_error);
			break;
		case 0x4394:
			*outres_ptr =
				sceCdStream0(instruct->m_posszarg2, (char *)instruct->m_buffer, instruct->m_cmdid, (int *)&instruct->m_error);
			break;
		case 0x4396:
			ee_stream_handler_normal(instruct, inbuf_len, outres_ptr);
			break;
		case 0x4398:
			ee_stream_handler_cdda(instruct, inbuf_len, outres_ptr);
			break;
		default:
			PRINTF("Un-support devctl %08x\n", cmd);
			retres = -EIO;
			break;
	}
	SignalSema(g_cdvdstm_semid);
	return retres;
}

int _start(int ac, char *av[], void *startaddr, ModuleInfo_t *mi)
{
	int last_error;
	int scres_unused;
	int state;

	(void)av;
	(void)startaddr;

	if ( ac < 0 )
	{
		int relres;

		if ( !sceCdSC(0xFFFFFFFF, &last_error) )
		{
			return MODULE_REMOVABLE_END;
		}
		g_cdvdstm_in_deldrv = 1;
		DelDrv(g_cdrom_stm_dev.name);
		CpuSuspendIntr(&state);
		relres = ReleaseLibraryEntries(&_exp_cdvdstm);
		CpuResumeIntr(state);
		g_cdvdstm_in_deldrv = 0;
		if ( relres && relres != KE_LIBRARY_NOTFOUND )
		{
			g_cdvdstm_in_deldrv = 0;
			return MODULE_REMOVABLE_END;
		}
		return MODULE_NO_RESIDENT_END;
	}
	if ( RegisterLibraryEntries(&_exp_cdvdstm) )
		return MODULE_NO_RESIDENT_END;
	DelDrv(g_cdrom_stm_dev.name);
	if ( AddDrv(&g_cdrom_stm_dev) )
	{
		cdrom_stm_deinit(&g_cdrom_stm_dev);
		return MODULE_NO_RESIDENT_END;
	}
	g_cdvdman_intr_efid = sceCdSC(0xFFFFFFF5, &scres_unused);
#if 0
	return MODULE_REMOVABLE_END;
#else
	if ( mi && ((mi->newflags & 2) != 0) )
		mi->newflags |= 0x10;
	return MODULE_RESIDENT_END;
#endif
}

static int stm_ee_read_timeout_alarm_cb(const iop_sys_clock_t *sys_clock)
{
	int read_timeout;

	read_timeout = sys_clock->lo / 0x9000;
	KPRINTF("Stm EE Read Time Out %d(msec)\n", read_timeout);
	sceCdSC(0xFFFFFFEE, &read_timeout);
	return !sceCdBreak();
}

static void ee_stream_handler_normal(cdrom_stm_devctl_t *instruct, int inbuf_len, int *outres_ptr)
{
	int retryflag;
	u32 cmdid;
	u32 posszarg2_bytes;
	int bankcur_tmp;
	unsigned int chunks_sectors;
	int bankcur_next_tmp1;
	int posszarg2_bytes_clamped;
	int dmat1;
	int bankcur_next_tmp2;
	int posszarg2_bytes_overrun;
	int outres_tmp2;
	int state;
	unsigned int i;

	(void)inbuf_len;

	retryflag = 0;
	cmdid = instruct->m_cmdid;
	if ( g_cdvdstm_stmstart_ee == 2 && (cmdid != 9 && cmdid != 3) )
	{
		*outres_ptr = 0;
		return;
	}
	switch ( cmdid )
	{
		case 8:
			sceCdSC(2, &g_cdvdstm_last_error_for_ee);
			if ( !sceCdNop() )
			{
				sceCdSC(0, &g_cdvdstm_last_error_for_ee);
				*outres_ptr = 0;
				return;
			}
			*outres_ptr = 1;
			return;
		case 7:
			CpuSuspendIntr(&state);
			CancelAlarm((unsigned int (*)(void *))ee_stream_intr_cb_normal, &g_cdvdstm_curclk_ee);
			sceCdSC(0, &g_cdvdstm_last_error_for_ee);
			CpuResumeIntr(state);
			sceCdSync(0);
			*outres_ptr = 1;
			CancelAlarm((unsigned int (*)(void *))stm_ee_read_timeout_alarm_cb, &g_cdvdstm_curclk_ee);
			return;
		case 6:
			bankcur_tmp = g_cdvdstm_bankcur_ee;
			if ( !g_cdvdstm_usedmap_ee[bankcur_tmp] )
			{
				bankcur_tmp += 1;
				if ( (unsigned int)bankcur_tmp >= (unsigned int)g_cdvdstm_bankcnt2 )
					bankcur_tmp = 0;
				if ( !g_cdvdstm_usedmap_ee[bankcur_tmp] )
					bankcur_tmp = g_cdvdstm_bankcur_ee;
			}
			for ( i = 0; (i < (unsigned int)g_cdvdstm_bankcnt2) && g_cdvdstm_usedmap_ee[bankcur_tmp]
									 && (g_cdvdstm_bankgp_ee != bankcur_tmp);
						i += 1 )
			{
				bankcur_tmp += 1;
				if ( (unsigned int)bankcur_tmp >= (unsigned int)g_cdvdstm_bankcnt2 )
					bankcur_tmp = 0;
			}
			*outres_ptr = i * ((unsigned int)g_cdvdstm_chunksz2 >> 11);
			return;
		case 5:
			sceCdstm1Cb((void (*)(int))ee_stream_intr_cb_normal_thunk);
			if ( !instruct->m_posszarg2 )
				__builtin_trap();
			chunks_sectors = instruct->m_posszarg1 / instruct->m_posszarg2;
			g_cdvdstm_bufsz2 = instruct->m_posszarg1;
			g_cdvdstm_sectorcount2 = chunks_sectors;
			g_cdvdstm_chunksz2 = chunks_sectors << 11;
			g_cdvdstm_buffer2 = (char *)instruct->m_buffer;
			g_cdvdstm_bankcnt2 = instruct->m_posszarg2;
			PRINTF(
				"Stream Buffer 1Bank %dbyte %dbanks %dbyte used\n",
				(int)(chunks_sectors << 11),
				(int)(instruct->m_posszarg2),
				(int)((chunks_sectors << 11) * (instruct->m_posszarg2)));
			*outres_ptr = 1;
			return;
		case 3:
			CpuSuspendIntr(&state);
			g_cdvdstm_stmstart_ee = 0;
			CancelAlarm((unsigned int (*)(void *))ee_stream_intr_cb_normal, &g_cdvdstm_curclk_ee);
			sceCdSC(0, &g_cdvdstm_last_error_for_ee);
			CpuResumeIntr(state);
			sceCdBreak();
			for ( i = 0; i < (unsigned int)g_cdvdstm_bankcnt2; i += 1 )
			{
				g_cdvdstm_usedmap_ee[i] = 0;
			}
			g_cdvdstm_bankoffs_ee = 0;
			g_cdvdstm_bankcur_ee = 0;
			g_cdvdstm_bankgp_ee = 0;
			*outres_ptr = 1;
			sceCdSync(0);
			g_cdvdstm_last_error_for_ee = 0;
			sceCdSC(0xFFFFFFFE, &g_cdvdstm_last_error_for_ee);
			CancelAlarm((unsigned int (*)(void *))stm_ee_read_timeout_alarm_cb, &g_cdvdstm_curclk_ee);
			return;
		case 1:
			g_cdvdstm_mode_ee.datapattern = instruct->m_rmode.datapattern;
			g_cdvdstm_mode_ee.trycount = instruct->m_rmode.trycount;
			g_cdvdstm_mode_ee.spindlctrl = instruct->m_rmode.spindlctrl;
			g_cdvdstm_retryerr_ee = 0;
			break;
	}
	posszarg2_bytes = instruct->m_posszarg2 << 11;
	if ( cmdid == 9 )
	{
		if ( sceCdSC(0xFFFFFFFF, &g_cdvdstm_last_error_for_ee) )
		{
			CpuSuspendIntr(&state);
			g_cdvdstm_lsn_ee = instruct->m_posszarg1;
			for ( i = 0; i < (unsigned int)g_cdvdstm_bankcnt2; i += 1 )
			{
				g_cdvdstm_usedmap_ee[i] = 0;
			}
			g_cdvdstm_stmstart_ee = 2;
			CpuResumeIntr(state);
			*outres_ptr = 1;
			return;
		}
		*outres_ptr = 0;
		return;
	}
	if ( cmdid == 4 )
	{
		retryflag = 1;
		CpuSuspendIntr(&state);
		CancelAlarm((unsigned int (*)(void *))ee_stream_intr_cb_normal, &g_cdvdstm_curclk_ee);
		sceCdSC(0, &g_cdvdstm_last_error_for_ee);
		posszarg2_bytes = 0;
		cmdid = 1;
		CpuResumeIntr(state);
		g_cdvdstm_lsn_ee = instruct->m_posszarg1;
		g_cdvdstm_bankoffs_ee = 0;
		g_cdvdstm_bankcur_ee = 0;
		g_cdvdstm_bankgp_ee = 0;
		sceCdSync(0);
		CancelAlarm((unsigned int (*)(void *))stm_ee_read_timeout_alarm_cb, &g_cdvdstm_curclk_ee);
	}
	if ( cmdid == 1 )
	{
		CpuSuspendIntr(&state);
		CancelAlarm((unsigned int (*)(void *))ee_stream_intr_cb_normal, &g_cdvdstm_curclk_ee);
		sceCdSC(0, &g_cdvdstm_last_error_for_ee);
		CpuResumeIntr(state);
		retryflag = 1;
		for ( i = 0; i < (unsigned int)g_cdvdstm_bankcnt2; i += 1 )
		{
			g_cdvdstm_usedmap_ee[i] = 0;
		}
		g_cdvdstm_lsn_ee = instruct->m_posszarg1;
		sceCdSC(0xFFFFFFE9, (int *)&g_cdvdstm_lsn_ee);
		g_cdvdstm_bankoffs_ee = 0;
		g_cdvdstm_bankcur_ee = 0;
		sceCdSync(0);
		CancelAlarm((unsigned int (*)(void *))stm_ee_read_timeout_alarm_cb, &g_cdvdstm_curclk_ee);
		g_cdvdstm_stmstart_ee = 1;
		sceCdSC(2, &g_cdvdstm_last_error_for_ee);
		if ( !sceCdNop() )
		{
			sceCdSC(0, &g_cdvdstm_last_error_for_ee);
			*outres_ptr = 0;
			return;
		}
	}
	posszarg2_bytes_overrun = -1;
	for ( i = 0; i < posszarg2_bytes; i += posszarg2_bytes_clamped )
	{
		unsigned int posszarg2_bytes_remain;

		posszarg2_bytes_remain = posszarg2_bytes - i;
		if ( !g_cdvdstm_usedmap_ee[g_cdvdstm_bankcur_ee] )
		{
			VERBOSE_KPRINTF(
				1,
				"CD read buffer over run %d %d %d %d %d gp %d pp %d\n",
				(u8)g_cdvdstm_usedmap_ee[0],
				(u8)g_cdvdstm_usedmap_ee[1],
				(u8)g_cdvdstm_usedmap_ee[2],
				(u8)g_cdvdstm_usedmap_ee[3],
				(u8)g_cdvdstm_usedmap_ee[4],
				g_cdvdstm_bankgp_ee,
				g_cdvdstm_bankcur_ee);
			CpuSuspendIntr(&state);
			bankcur_next_tmp1 = g_cdvdstm_bankcur_ee;
			g_cdvdstm_bankcur_ee += 1;
			if ( (unsigned int)g_cdvdstm_bankcur_ee >= (unsigned int)g_cdvdstm_bankcnt2 )
				g_cdvdstm_bankcur_ee = 0;
			if ( !g_cdvdstm_usedmap_ee[g_cdvdstm_bankcur_ee] )
				g_cdvdstm_bankcur_ee = bankcur_next_tmp1;
			posszarg2_bytes_overrun = posszarg2_bytes - posszarg2_bytes_remain;
			CpuResumeIntr(state);
			break;
		}
		posszarg2_bytes_clamped = ((unsigned int)(g_cdvdstm_chunksz2 - g_cdvdstm_bankoffs_ee) < posszarg2_bytes_remain) ?
																(unsigned int)(g_cdvdstm_chunksz2 - g_cdvdstm_bankoffs_ee) :
																posszarg2_bytes_remain;
		g_cdvdstm_dmat.dest = ((char *)instruct->m_buffer) + i;
		g_cdvdstm_dmat.size = posszarg2_bytes_clamped;
		g_cdvdstm_dmat.attr = 0;
		g_cdvdstm_dmat.src = (char *)g_cdvdstm_buffer2 + g_cdvdstm_bankcur_ee * g_cdvdstm_chunksz2 + g_cdvdstm_bankoffs_ee;
		if ( posszarg2_bytes_clamped )
		{
			while ( 1 )
			{
				CpuSuspendIntr(&state);
				dmat1 = sceSifSetDma(&g_cdvdstm_dmat, 1);
				CpuResumeIntr(state);
				if ( dmat1 )
					break;
				DelayThread(500);
			}
			g_cdvdstm_bankoffs_ee += posszarg2_bytes_clamped;
			while ( sceSifDmaStat(dmat1) >= 0 )
				;
		}
		if ( (unsigned int)g_cdvdstm_bankoffs_ee >= (unsigned int)g_cdvdstm_chunksz2 )
		{
			CpuSuspendIntr(&state);
			g_cdvdstm_bankoffs_ee = 0;
			g_cdvdstm_usedmap_ee[g_cdvdstm_bankcur_ee] = 0;
			bankcur_next_tmp2 = g_cdvdstm_bankcur_ee;
			g_cdvdstm_bankcur_ee += 1;
			if ( (unsigned int)g_cdvdstm_bankcur_ee >= (unsigned int)g_cdvdstm_bankcnt2 )
				g_cdvdstm_bankcur_ee = 0;
			if ( g_cdvdstm_usedmap_ee[g_cdvdstm_bankcur_ee] && g_cdvdstm_bankgp_ee != g_cdvdstm_bankcur_ee )
			{
				CpuResumeIntr(state);
			}
			else
			{
				g_cdvdstm_bankcur_ee = bankcur_next_tmp2;
				CpuResumeIntr(state);
				VERBOSE_KPRINTF(
					1,
					"CD read buffer over run %d %d %d %d %d gp %d pp %d\n",
					(u8)g_cdvdstm_usedmap_ee[0],
					(u8)g_cdvdstm_usedmap_ee[1],
					(u8)g_cdvdstm_usedmap_ee[2],
					(u8)g_cdvdstm_usedmap_ee[3],
					(u8)g_cdvdstm_usedmap_ee[4],
					g_cdvdstm_bankgp_ee,
					g_cdvdstm_bankcur_ee);
				posszarg2_bytes_overrun = posszarg2_bytes - (posszarg2_bytes_remain - posszarg2_bytes_clamped);
				break;
			}
		}
	}
	if ( posszarg2_bytes_overrun == -1 )
		posszarg2_bytes_overrun = posszarg2_bytes;
	outres_tmp2 = (retryflag) ? 1 : (posszarg2_bytes_overrun / 0x800);
	if ( !retryflag )
	{
		if ( sceCdSC(0xFFFFFFFF, &g_cdvdstm_last_error_for_ee) != 2 && !outres_tmp2 && !g_cdvdstm_retryerr_ee )
			g_cdvdstm_retryerr_ee = 273;
		if ( g_cdvdstm_retryerr_ee )
		{
			outres_tmp2 = (u16)outres_tmp2 | (g_cdvdstm_retryerr_ee << 16);
			g_cdvdstm_retryerr_ee = 0;
		}
	}
	*outres_ptr = outres_tmp2;
	return;
}

static unsigned int ee_stream_intr_cb_normal(void *userdata)
{
	int scres_unused;

	(void)userdata;

	VERBOSE_KPRINTF(1, "Intr EE Stm Read call\n");
	iCancelAlarm((unsigned int (*)(void *))stm_ee_read_timeout_alarm_cb, &g_cdvdstm_curclk_ee);
	iCancelAlarm((unsigned int (*)(void *))ee_stream_intr_cb_normal, &g_cdvdstm_curclk_ee);
	sceCdSC(0xFFFFFFFF, &g_cdvdstm_last_error_for_ee);
	if ( !g_cdvdstm_last_error_for_ee )
	{
		switch ( sceCdGetDiskType() )
		{
			case SCECdPSCD:
			case SCECdPSCDDA:
			case SCECdPS2CD:
			case SCECdPS2CDDA:
			case SCECdPS2DVD:
				break;
			default:
				g_cdvdstm_last_error_for_ee = SCECdErREADCF;
				break;
		}
	}
	g_cdvdstm_curclk_ee.hi = 0;
	if ( g_cdvdstm_stmstart_ee )
		g_cdvdstm_retrycnt_ee_normal = 0;
	g_cdvdstm_curclk_ee.lo = (g_cdvdstm_stmstart_ee || g_cdvdstm_last_error_for_ee || g_cdvdstm_retrycnt_ee_normal) ?
														 0x20f58000 :
														 (0x9000 * sceCdSC(0xFFFFFFEF, &scres_unused));
	if ( g_cdvdstm_last_error_for_ee )
	{
		VERBOSE_KPRINTF(
			1,
			"EE Stream read LBN= %d Error code= 0x%02x retry= %d\n",
			g_cdvdstm_readlbn_ee_normal,
			g_cdvdstm_last_error_for_ee,
			g_cdvdstm_retrycnt_ee_normal);
		if ( g_cdvdstm_last_error_for_ee == SCECdErREAD || g_cdvdstm_last_error_for_ee == SCECdErABRT )
		{
			if ( g_cdvdstm_retrycnt_ee_normal )
			{
				VERBOSE_KPRINTF(1, "On Retry retry %d err %08x\n", g_cdvdstm_retrycnt_ee_normal, g_cdvdstm_last_error_for_ee);
			}
			g_cdvdstm_retrycnt_ee_normal = 3;
		}
		else
		{
			g_cdvdstm_retrycnt_ee_normal = 1;
		}
		g_cdvdstm_retryerr_ee = g_cdvdstm_last_error_for_ee;
	}
	if ( g_cdvdstm_retrycnt_ee_normal )
	{
		g_cdvdstm_retrycnt_ee_normal -= 1;
		if ( sceCdRE(
					 ((unsigned int)g_cdvdstm_readlbn_ee_normal >= (unsigned int)(0x10 * g_cdvdstm_retrycnt_ee_normal)) ?
						 (g_cdvdstm_readlbn_ee_normal - 0x10 * g_cdvdstm_retrycnt_ee_normal) :
						 (g_cdvdstm_readlbn_ee_normal + 0x10 * g_cdvdstm_retrycnt_ee_normal),
					 g_cdvdstm_sectorcount2,
					 (char *)g_cdvdstm_buffer2 + g_cdvdstm_bankgp_ee * g_cdvdstm_chunksz2,
					 &g_cdvdstm_mode_ee) )
		{
			iSetAlarm(&g_cdvdstm_curclk_ee, (unsigned int (*)(void *))stm_ee_read_timeout_alarm_cb, &g_cdvdstm_curclk_ee);
		}
		else
		{
			VERBOSE_KPRINTF(1, "Stm Read Call fail\n");
			g_cdvdstm_curclk_ee.lo = 0x708000;
			if (
				iSetAlarm(&g_cdvdstm_curclk_ee, (unsigned int (*)(void *))ee_stream_intr_cb_normal, &g_cdvdstm_curclk_ee)
				&& !sceCdNop() )
			{
				sceCdSC(0, &g_cdvdstm_last_error_for_ee);
			}
			g_cdvdstm_retrycnt_ee_normal += 1;
		}
	}
	else
	{
		int gptmp;

		if ( !g_cdvdstm_stmstart_ee )
		{
			g_cdvdstm_usedmap_ee[g_cdvdstm_bankgp_ee] = 1;
			gptmp = g_cdvdstm_bankgp_ee;
			g_cdvdstm_bankgp_ee += 1;
			if ( (unsigned int)g_cdvdstm_bankgp_ee >= (unsigned int)g_cdvdstm_bankcnt2 )
				g_cdvdstm_bankgp_ee = 0;
		}
		if (
			!g_cdvdstm_stmstart_ee
			&& (g_cdvdstm_usedmap_ee[g_cdvdstm_bankgp_ee] || g_cdvdstm_bankcur_ee == g_cdvdstm_bankgp_ee) )
		{
			g_cdvdstm_bankgp_ee = gptmp;
			g_cdvdstm_usedmap_ee[gptmp] = 0;
			VERBOSE_KPRINTF(
				1,
				"read Full %d %d %d %d %d gp %d pp %d spn %d\n",
				(u8)g_cdvdstm_usedmap_ee[0],
				(u8)g_cdvdstm_usedmap_ee[1],
				(u8)g_cdvdstm_usedmap_ee[2],
				(u8)g_cdvdstm_usedmap_ee[3],
				(u8)g_cdvdstm_usedmap_ee[4],
				g_cdvdstm_bankgp_ee,
				g_cdvdstm_bankcur_ee,
				g_cdvdstm_mode_ee.spindlctrl);
			g_cdvdstm_curclk_ee.lo = 0x48000;
			if (
				iSetAlarm(&g_cdvdstm_curclk_ee, (unsigned int (*)(void *))ee_stream_intr_cb_normal, &g_cdvdstm_curclk_ee)
				&& !sceCdNop() )
			{
				sceCdSC(0, &g_cdvdstm_last_error_for_ee);
			}
		}
		else
		{
			if ( g_cdvdstm_stmstart_ee == 2 )
			{
				unsigned int i;

				g_cdvdstm_bankoffs_ee = 0;
				g_cdvdstm_bankcur_ee = 0;
				g_cdvdstm_bankgp_ee = 0;
				for ( i = 0; i < (unsigned int)g_cdvdstm_bankcnt2; i += 1 )
					g_cdvdstm_usedmap_ee[i] = 0;
			}
			g_cdvdstm_stmstart_ee = 0;
			g_cdvdstm_readlbn_ee_normal = g_cdvdstm_lsn_ee;
			if ( sceCdRE(
						 g_cdvdstm_lsn_ee,
						 g_cdvdstm_sectorcount2,
						 (char *)g_cdvdstm_buffer2 + g_cdvdstm_bankgp_ee * g_cdvdstm_chunksz2,
						 &g_cdvdstm_mode_ee) )
			{
				iSetAlarm(&g_cdvdstm_curclk_ee, (unsigned int (*)(void *))stm_ee_read_timeout_alarm_cb, &g_cdvdstm_curclk_ee);
			}
			else
			{
				VERBOSE_KPRINTF(1, "Stm Read Call1 fail\n");
				g_cdvdstm_curclk_ee.lo = 0x708000;
				if (
					iSetAlarm(&g_cdvdstm_curclk_ee, (unsigned int (*)(void *))ee_stream_intr_cb_normal, &g_cdvdstm_curclk_ee)
					&& !sceCdNop() )
				{
					sceCdSC(0, &g_cdvdstm_last_error_for_ee);
				}
				g_cdvdstm_retrycnt_ee_normal = 1;
			}
			g_cdvdstm_lsn_ee += g_cdvdstm_sectorcount2;
		}
	}
	return 0;
}

static void ee_stream_handler_cdda(cdrom_stm_devctl_t *instruct, int inbuf_len, int *outres_ptr)
{
	u32 cmdid;
	u32 posszarg2_bytes;
	int retryflag;
	int bankcur_tmp;
	u32 chunks_sectors;
	int bankcur_next_tmp1;
	int posszarg2_bytes_clamped;
	int dmat2;
	int bankcur_next_tmp2;
	int posszarg2_overrun_chunks2;
	unsigned int posszarg2_bytes_overrun;
	int state;
	unsigned int i;

	(void)inbuf_len;

	cmdid = instruct->m_cmdid;
	posszarg2_bytes = instruct->m_posszarg2 * g_cdvdstm_usedchunksize2;
	retryflag = 0;
	if ( g_cdvdstm_stmstart_ee == 2 && (cmdid != 9 && cmdid != 3) )
	{
		*outres_ptr = 0;
		return;
	}
	switch ( cmdid )
	{
		case 8:
			sceCdSC(2, &g_cdvdstm_last_error_for_ee);
			if ( sceCdNop() )
			{
				*outres_ptr = 1;
				return;
			}
			sceCdSC(0, &g_cdvdstm_last_error_for_ee);
			*outres_ptr = 0;
			return;
		case 7:
			CpuSuspendIntr(&state);
			CancelAlarm((unsigned int (*)(void *))ee_stream_intr_cb_cdda, &g_cdvdstm_curclk_ee);
			sceCdSC(0, &g_cdvdstm_last_error_for_ee);
			CpuResumeIntr(state);
			sceCdSync(0);
			*outres_ptr = 1;
			CancelAlarm((unsigned int (*)(void *))stm_ee_read_timeout_alarm_cb, &g_cdvdstm_curclk_ee);
			return;
		case 6:
			bankcur_tmp = g_cdvdstm_bankcur_ee;
			if ( !g_cdvdstm_usedmap_ee[bankcur_tmp] )
			{
				bankcur_tmp += 1;
				if ( (unsigned int)bankcur_tmp >= (unsigned int)g_cdvdstm_bankcnt2 )
					bankcur_tmp = 0;
				if ( !g_cdvdstm_usedmap_ee[bankcur_tmp] )
					bankcur_tmp = g_cdvdstm_bankcur_ee;
			}
			for ( i = 0; (i < (unsigned int)g_cdvdstm_bankcnt2) && g_cdvdstm_usedmap_ee[bankcur_tmp]
									 && (g_cdvdstm_bankgp_ee != bankcur_tmp);
						i += 1 )
			{
				bankcur_tmp += 1;
				if ( (unsigned int)(bankcur_tmp) >= (unsigned int)g_cdvdstm_bankcnt2 )
					bankcur_tmp = 0;
			}
			if ( !g_cdvdstm_usedchunksize2 )
				__builtin_trap();
			*outres_ptr = i * (g_cdvdstm_chunksz2 / (unsigned int)g_cdvdstm_usedchunksize2);
			return;
		case 5:
			sceCdstm1Cb((void (*)(int))ee_stream_intr_cb_cdda_thunk);
			switch ( instruct->m_rmode.datapattern )
			{
				case SCECdSecS2368:
					g_cdvdstm_usedchunksize2 = 2368;
					break;
				case SCECdSecS2448:
					g_cdvdstm_usedchunksize2 = 2448;
					break;
				case SCECdSecS2352:
				default:
					g_cdvdstm_usedchunksize2 = 0x930;
					break;
			}
			if ( !instruct->m_posszarg2 )
				__builtin_trap();
			chunks_sectors = instruct->m_posszarg1 / instruct->m_posszarg2;
			g_cdvdstm_bufsz2 = instruct->m_posszarg1;
			g_cdvdstm_sectorcount2 = chunks_sectors;
			g_cdvdstm_chunksz2 = chunks_sectors * g_cdvdstm_usedchunksize2;
			g_cdvdstm_buffer2 = (char *)instruct->m_buffer;
			g_cdvdstm_bankcnt2 = instruct->m_posszarg2;
			PRINTF(
				"DA Stream Buffer 1Bank %dbyte %dbanks %dbyte used\n",
				(int)(chunks_sectors * g_cdvdstm_usedchunksize2),
				(int)instruct->m_posszarg2,
				(int)(chunks_sectors * g_cdvdstm_usedchunksize2 * instruct->m_posszarg2));
			*outres_ptr = 1;
			return;
		case 3:
			CpuSuspendIntr(&state);
			g_cdvdstm_stmstart_ee = 0;
			CancelAlarm((unsigned int (*)(void *))ee_stream_intr_cb_cdda, &g_cdvdstm_curclk_ee);
			sceCdSC(0, &g_cdvdstm_last_error_for_ee);
			CpuResumeIntr(state);
			sceCdBreak();
			for ( i = 0; i < (unsigned int)g_cdvdstm_bankcnt2; i += 1 )
			{
				g_cdvdstm_usedmap_ee[i] = 0;
			}
			g_cdvdstm_bankoffs_ee = 0;
			g_cdvdstm_bankcur_ee = 0;
			g_cdvdstm_bankgp_ee = 0;
			*outres_ptr = 1;
			sceCdSync(0);
			g_cdvdstm_last_error_for_ee = 0;
			sceCdSC(0xFFFFFFFE, &g_cdvdstm_last_error_for_ee);
			CancelAlarm((unsigned int (*)(void *))stm_ee_read_timeout_alarm_cb, &g_cdvdstm_curclk_ee);
			return;
		case 1:
			g_cdvdstm_mode_ee.datapattern = instruct->m_rmode.datapattern;
			g_cdvdstm_mode_ee.trycount = instruct->m_rmode.trycount;
			g_cdvdstm_mode_ee.spindlctrl = instruct->m_rmode.spindlctrl;
			g_cdvdstm_retryerr_ee = 0;
			break;
	}
	if ( cmdid == 9 )
	{
		if ( sceCdSC(0xFFFFFFFF, &g_cdvdstm_last_error_for_ee) )
		{
			CpuSuspendIntr(&state);
			g_cdvdstm_lsn_ee = instruct->m_posszarg1;
			for ( i = 0; i < (unsigned int)g_cdvdstm_bankcnt2; i += 1 )
			{
				g_cdvdstm_usedmap_ee[i] = 0;
			}
			g_cdvdstm_stmstart_ee = 2;
			CpuResumeIntr(state);
			*outres_ptr = 1;
			return;
		}
		*outres_ptr = 0;
		return;
	}
	if ( cmdid == 4 )
	{
		retryflag = 1;
		CpuSuspendIntr(&state);
		CancelAlarm((unsigned int (*)(void *))ee_stream_intr_cb_cdda, &g_cdvdstm_curclk_ee);
		sceCdSC(0, &g_cdvdstm_last_error_for_ee);
		posszarg2_bytes = 0;
		cmdid = 1;
		CpuResumeIntr(state);
		g_cdvdstm_lsn_ee = instruct->m_posszarg1;
		g_cdvdstm_bankoffs_ee = 0;
		g_cdvdstm_bankcur_ee = 0;
		g_cdvdstm_bankgp_ee = 0;
		sceCdSync(0);
		CancelAlarm((unsigned int (*)(void *))stm_ee_read_timeout_alarm_cb, &g_cdvdstm_curclk_ee);
	}
	if ( cmdid == 1 )
	{
		signed int posszarg2_chunks;

		CpuSuspendIntr(&state);
		CancelAlarm((unsigned int (*)(void *))ee_stream_intr_cb_cdda, &g_cdvdstm_curclk_ee);
		sceCdSC(0, &g_cdvdstm_last_error_for_ee);
		CpuResumeIntr(state);
		retryflag = 1;
		for ( i = 0; i < (unsigned int)g_cdvdstm_bankcnt2; i += 1 )
		{
			g_cdvdstm_usedmap_ee[i] = 0;
		}
		g_cdvdstm_lsn_ee = instruct->m_posszarg1;
		g_cdvdstm_bankoffs_ee = 0;
		g_cdvdstm_bankcur_ee = 0;
		sceCdSync(0);
		CancelAlarm((unsigned int (*)(void *))stm_ee_read_timeout_alarm_cb, &g_cdvdstm_curclk_ee);
		if ( !g_cdvdstm_chunksz2 )
			__builtin_trap();
		posszarg2_chunks = (posszarg2_bytes / g_cdvdstm_chunksz2) + (!!(posszarg2_bytes % g_cdvdstm_chunksz2));
		for ( g_cdvdstm_bankgp_ee = 0; g_cdvdstm_bankgp_ee < posszarg2_chunks; g_cdvdstm_bankgp_ee += 1 )
		{
			int outres_tmp2;

			outres_tmp2 = sceCdReadCDDA(g_cdvdstm_lsn_ee, g_cdvdstm_sectorcount2, g_cdvdstm_buffer2, &g_cdvdstm_mode_ee);
			sceCdSync(3);
			sceCdSC(0xFFFFFFFF, &g_cdvdstm_last_error_for_ee);
			if ( g_cdvdstm_last_error_for_ee || !outres_tmp2 )
			{
				sceCdSC(0, &g_cdvdstm_last_error_for_ee);
				*outres_ptr = 0;
				return;
			}
			g_cdvdstm_lsn_ee += g_cdvdstm_sectorcount2;
			g_cdvdstm_usedmap_ee[g_cdvdstm_bankgp_ee] = 1;
		}
		g_cdvdstm_stmstart_ee = 1;
		sceCdSC(2, &g_cdvdstm_last_error_for_ee);
		if ( !sceCdNop() )
		{
			sceCdSC(0, &g_cdvdstm_last_error_for_ee);
			*outres_ptr = 0;
			return;
		}
	}
	posszarg2_bytes_overrun = -1;
	for ( i = 0; i < posszarg2_bytes; i += posszarg2_bytes_clamped )
	{
		unsigned int posszarg2_bytes_remain;

		posszarg2_bytes_remain = posszarg2_bytes - i;
		if ( !g_cdvdstm_usedmap_ee[g_cdvdstm_bankcur_ee] )
		{
			VERBOSE_KPRINTF(
				1,
				"CD read buffer over run %d %d %d %d %d gp %d pp %d\n",
				(u8)g_cdvdstm_usedmap_ee[0],
				(u8)g_cdvdstm_usedmap_ee[1],
				(u8)g_cdvdstm_usedmap_ee[2],
				(u8)g_cdvdstm_usedmap_ee[3],
				(u8)g_cdvdstm_usedmap_ee[4],
				g_cdvdstm_bankgp_ee,
				g_cdvdstm_bankcur_ee);
			CpuSuspendIntr(&state);
			bankcur_next_tmp1 = g_cdvdstm_bankcur_ee;
			g_cdvdstm_bankcur_ee += 1;
			if ( (unsigned int)g_cdvdstm_bankcur_ee >= (unsigned int)g_cdvdstm_bankcnt2 )
				g_cdvdstm_bankcur_ee = 0;
			if ( !g_cdvdstm_usedmap_ee[g_cdvdstm_bankcur_ee] )
				g_cdvdstm_bankcur_ee = bankcur_next_tmp1;
			posszarg2_bytes_overrun = posszarg2_bytes - posszarg2_bytes_remain;
			CpuResumeIntr(state);
			break;
		}
		posszarg2_bytes_clamped = ((unsigned int)(g_cdvdstm_chunksz2 - g_cdvdstm_bankoffs_ee) < posszarg2_bytes_remain) ?
																(unsigned int)(g_cdvdstm_chunksz2 - g_cdvdstm_bankoffs_ee) :
																posszarg2_bytes_remain;
		g_cdvdstm_dmat2.dest = ((char *)instruct->m_buffer) + i;
		g_cdvdstm_dmat2.size = posszarg2_bytes_clamped;
		g_cdvdstm_dmat2.attr = 0;
		g_cdvdstm_dmat2.src = (char *)g_cdvdstm_buffer2 + g_cdvdstm_bankcur_ee * g_cdvdstm_chunksz2 + g_cdvdstm_bankoffs_ee;
		if ( posszarg2_bytes_clamped )
		{
			while ( 1 )
			{
				CpuSuspendIntr(&state);
				dmat2 = sceSifSetDma(&g_cdvdstm_dmat2, 1);
				CpuResumeIntr(state);
				if ( dmat2 )
					break;
				DelayThread(500);
			}
			g_cdvdstm_bankoffs_ee += posszarg2_bytes_clamped;
			while ( sceSifDmaStat(dmat2) >= 0 )
				;
		}
		if ( (unsigned int)g_cdvdstm_bankoffs_ee >= (unsigned int)g_cdvdstm_chunksz2 )
		{
			CpuSuspendIntr(&state);
			g_cdvdstm_bankoffs_ee = 0;
			g_cdvdstm_usedmap_ee[g_cdvdstm_bankcur_ee] = 0;
			bankcur_next_tmp2 = g_cdvdstm_bankcur_ee;
			g_cdvdstm_bankcur_ee += 1;
			if ( (unsigned int)g_cdvdstm_bankcur_ee >= (unsigned int)g_cdvdstm_bankcnt2 )
				g_cdvdstm_bankcur_ee = 0;
			if ( g_cdvdstm_usedmap_ee[g_cdvdstm_bankcur_ee] && g_cdvdstm_bankgp_ee != g_cdvdstm_bankcur_ee )
			{
				CpuResumeIntr(state);
			}
			else
			{
				g_cdvdstm_bankcur_ee = bankcur_next_tmp2;
				CpuResumeIntr(state);
				VERBOSE_KPRINTF(
					1,
					"CD read buffer over run %d %d %d %d %d gp %d pp %d\n",
					(u8)g_cdvdstm_usedmap_ee[0],
					(u8)g_cdvdstm_usedmap_ee[1],
					(u8)g_cdvdstm_usedmap_ee[2],
					(u8)g_cdvdstm_usedmap_ee[3],
					(u8)g_cdvdstm_usedmap_ee[4],
					g_cdvdstm_bankgp_ee,
					g_cdvdstm_bankcur_ee);
				posszarg2_bytes_overrun = posszarg2_bytes - (posszarg2_bytes_remain - posszarg2_bytes_clamped);
				break;
			}
		}
	}
	if ( posszarg2_bytes_overrun == 0xFFFFFFFF )
		posszarg2_bytes_overrun = posszarg2_bytes;
	if ( !g_cdvdstm_usedchunksize2 )
		__builtin_trap();
	posszarg2_overrun_chunks2 = posszarg2_bytes_overrun / g_cdvdstm_usedchunksize2;
	if ( retryflag )
	{
		*outres_ptr = 1;
		return;
	}
	if ( sceCdSC(0xFFFFFFFF, &g_cdvdstm_last_error_for_ee) != 2 && !posszarg2_overrun_chunks2 && !g_cdvdstm_retryerr_ee )
		g_cdvdstm_retryerr_ee = 273;
	if ( g_cdvdstm_retryerr_ee )
	{
		posszarg2_overrun_chunks2 = (u16)posszarg2_overrun_chunks2 | (g_cdvdstm_retryerr_ee << 16);
		g_cdvdstm_retryerr_ee = 0;
	}
	*outres_ptr = posszarg2_overrun_chunks2;
}

static unsigned int ee_stream_intr_cb_cdda(void *userdata)
{
	int scres_unused;

	(void)userdata;

	VERBOSE_KPRINTF(1, "Intr EE DA Stm Read call\n");
	iCancelAlarm((unsigned int (*)(void *))stm_ee_read_timeout_alarm_cb, &g_cdvdstm_curclk_ee);
	iCancelAlarm((unsigned int (*)(void *))ee_stream_intr_cb_cdda, &g_cdvdstm_curclk_ee);
	sceCdSC(0xFFFFFFFF, &g_cdvdstm_last_error_for_ee);
	if ( !g_cdvdstm_last_error_for_ee )
	{
		switch ( sceCdGetDiskType() )
		{
			case SCECdPSCDDA:
			case SCECdPS2CDDA:
			case 0x21:
			case SCECdCDDA:
				break;
			default:
				g_cdvdstm_last_error_for_ee = SCECdErREADCF;
				break;
		}
	}
	g_cdvdstm_curclk_ee.hi = 0;
	if ( g_cdvdstm_stmstart_ee )
		g_cdvdstm_retrycnt_ee_cdda = 0;
	g_cdvdstm_curclk_ee.lo = (g_cdvdstm_stmstart_ee || g_cdvdstm_last_error_for_ee || g_cdvdstm_retrycnt_ee_cdda) ?
														 0x20f58000 :
														 (0x9000 * sceCdSC(0xFFFFFFEF, &scres_unused));
	if ( g_cdvdstm_last_error_for_ee )
	{
		VERBOSE_KPRINTF(
			1,
			"EE Stream read LBN= %d Error code= 0x%02x retry= %d\n",
			g_cdvdstm_readlbn_ee_cdda,
			g_cdvdstm_last_error_for_ee,
			g_cdvdstm_retrycnt_ee_cdda);
		if ( g_cdvdstm_last_error_for_ee == SCECdErREAD || g_cdvdstm_last_error_for_ee == SCECdErABRT )
		{
			if ( g_cdvdstm_retrycnt_ee_cdda )
			{
				VERBOSE_KPRINTF(1, "On Retry retry %d err %08x\n", g_cdvdstm_retrycnt_ee_cdda, g_cdvdstm_last_error_for_ee);
			}
			g_cdvdstm_retrycnt_ee_cdda = 4;
		}
		else
		{
			g_cdvdstm_retrycnt_ee_cdda = 1;
		}
		g_cdvdstm_retryerr_ee = g_cdvdstm_last_error_for_ee;
	}
	else
	{
		g_cdvdstm_retrycnt_ee_cdda = 0;
	}
	if ( g_cdvdstm_retrycnt_ee_cdda )
	{
		g_cdvdstm_retrycnt_ee_cdda -= 1;
		if (
			!g_cdvdstm_retrycnt_ee_cdda
			&& (g_cdvdstm_last_error_for_ee == SCECdErREAD || g_cdvdstm_last_error_for_ee == SCECdErABRT) )
		{
			g_cdvdstm_readlbn_ee_cdda =
				g_cdvdstm_lsn_ee + ((g_cdvdstm_sectorcount2 < 0x1D) ? (0x1D - g_cdvdstm_sectorcount2) : 0);
			g_cdvdstm_lsn_ee = g_cdvdstm_readlbn_ee_cdda + g_cdvdstm_sectorcount2;
		}
		if ( sceCdReadCDDA(
					 g_cdvdstm_readlbn_ee_cdda,
					 g_cdvdstm_sectorcount2,
					 (char *)g_cdvdstm_buffer2 + g_cdvdstm_bankgp_ee * g_cdvdstm_chunksz2,
					 &g_cdvdstm_mode_ee) )
		{
			iSetAlarm(&g_cdvdstm_curclk_ee, (unsigned int (*)(void *))stm_ee_read_timeout_alarm_cb, &g_cdvdstm_curclk_ee);
		}
		else
		{
			VERBOSE_KPRINTF(1, "Stm Read Call fail\n");
			g_cdvdstm_curclk_ee.lo = 0x708000;
			if (
				iSetAlarm(&g_cdvdstm_curclk_ee, (unsigned int (*)(void *))ee_stream_intr_cb_cdda, &g_cdvdstm_curclk_ee)
				&& !sceCdNop() )
			{
				sceCdSC(0, &g_cdvdstm_last_error_for_ee);
			}
			g_cdvdstm_retrycnt_ee_cdda += 1;
		}
	}
	else
	{
		int gptmp;

		if ( !g_cdvdstm_stmstart_ee )
		{
			g_cdvdstm_usedmap_ee[g_cdvdstm_bankgp_ee] = 1;
			gptmp = g_cdvdstm_bankgp_ee;
			g_cdvdstm_bankgp_ee += 1;
			if ( (unsigned int)g_cdvdstm_bankgp_ee >= (unsigned int)g_cdvdstm_bankcnt2 )
				g_cdvdstm_bankgp_ee = 0;
		}
		if (
			!g_cdvdstm_stmstart_ee
			&& (g_cdvdstm_usedmap_ee[g_cdvdstm_bankgp_ee] || g_cdvdstm_bankcur_ee == g_cdvdstm_bankgp_ee) )
		{
			g_cdvdstm_bankgp_ee = gptmp;
			g_cdvdstm_usedmap_ee[gptmp] = 0;
			VERBOSE_KPRINTF(
				1,
				"read Full %d %d %d %d %d gp %d pp %d spn %d\n",
				(u8)g_cdvdstm_usedmap_ee[0],
				(u8)g_cdvdstm_usedmap_ee[1],
				(u8)g_cdvdstm_usedmap_ee[2],
				(u8)g_cdvdstm_usedmap_ee[3],
				(u8)g_cdvdstm_usedmap_ee[4],
				g_cdvdstm_bankgp_ee,
				g_cdvdstm_bankcur_ee,
				g_cdvdstm_mode_ee.spindlctrl);
			g_cdvdstm_curclk_ee.lo = 0x48000;
			if (
				iSetAlarm(&g_cdvdstm_curclk_ee, (unsigned int (*)(void *))ee_stream_intr_cb_cdda, &g_cdvdstm_curclk_ee)
				&& !sceCdNop() )
			{
				sceCdSC(0, &g_cdvdstm_last_error_for_ee);
			}
		}
		else
		{
			if ( g_cdvdstm_stmstart_ee == 2 )
			{
				unsigned int i;

				g_cdvdstm_bankoffs_ee = 0;
				g_cdvdstm_bankcur_ee = 0;
				g_cdvdstm_bankgp_ee = 0;
				for ( i = 0; i < (unsigned int)g_cdvdstm_bankcnt2; i += 1 )
					g_cdvdstm_usedmap_ee[i] = 0;
			}
			g_cdvdstm_stmstart_ee = 0;
			g_cdvdstm_readlbn_ee_cdda = g_cdvdstm_lsn_ee;
			if ( sceCdReadCDDA(
						 g_cdvdstm_lsn_ee,
						 g_cdvdstm_sectorcount2,
						 (char *)g_cdvdstm_buffer2 + g_cdvdstm_bankgp_ee * g_cdvdstm_chunksz2,
						 &g_cdvdstm_mode_ee) )
			{
				iSetAlarm(&g_cdvdstm_curclk_ee, (unsigned int (*)(void *))stm_ee_read_timeout_alarm_cb, &g_cdvdstm_curclk_ee);
			}
			else
			{
				VERBOSE_KPRINTF(1, "Stm Read Call1 fail\n");
				g_cdvdstm_curclk_ee.lo = 0x708000;
				if (
					iSetAlarm(&g_cdvdstm_curclk_ee, (unsigned int (*)(void *))ee_stream_intr_cb_cdda, &g_cdvdstm_curclk_ee)
					&& !sceCdNop() )
				{
					sceCdSC(0, &g_cdvdstm_last_error_for_ee);
				}
				g_cdvdstm_retrycnt_ee_cdda = 1;
			}
			g_cdvdstm_lsn_ee += g_cdvdstm_sectorcount2;
		}
	}
	return 0;
}

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
