/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "accdvde_internal.h"

static int cde_op_ready(void *arg);
static int cde_op_type(void *arg);
static int cde_op_error(void *arg);
static int cde_op_getpos(void *arg);
static int cde_op_init(void *arg);
static int cde_op_pause(void *arg);
static int cde_op_read(void *arg);
static int cde_op_readtoc(void *arg);
static int cde_op_readi(void *arg);
static int cde_op_sync(void *arg);
static int cde_op_lookup(void *arg);
static int cde_op_seek(void *arg);
static int cde_op_standby(void *arg);
static int cde_op_stat(void *arg);
static int cde_op_stop(void *arg);
static int cde_op_tray(void *arg);
static int cde_op_inits(void *arg);
static int cde_op_reads(void *arg);
static int cde_op_seeks(void *arg);
static int cde_op_starts(void *arg);
static int cde_op_stats(void *arg);
static int cde_op_stops(void *arg);
static int cde_op_pauses(void *arg);
static int cde_op_resumes(void *arg);
static int cde_op_readrtc(void *arg);

static cde_ops_t Cde_ops[27] = {
	NULL,
	NULL,
	&cde_op_ready,
	&cde_op_type,
	&cde_op_error,
	&cde_op_getpos,
	&cde_op_readtoc,
	&cde_op_init,
	&cde_op_pause,
	&cde_op_read,
	&cde_op_readi,
	&cde_op_sync,
	&cde_op_lookup,
	&cde_op_seek,
	&cde_op_standby,
	&cde_op_stat,
	&cde_op_stop,
	&cde_op_tray,
	&cde_op_inits,
	&cde_op_reads,
	&cde_op_seeks,
	&cde_op_starts,
	&cde_op_stats,
	&cde_op_stops,
	&cde_op_pauses,
	&cde_op_resumes,
	&cde_op_readrtc};
static struct cde_softc Cdec;

static int cde_op_ready(void *arg)
{
	const struct ac_cdvdsif_ready *argt;
	struct ac_cdvdsif_reply *argr;

	argt = (struct ac_cdvdsif_ready *)arg;
	argr = (struct ac_cdvdsif_reply *)arg;
	argr->result = cdc_ready(argt->mode);
	return 1;
}

static int cde_op_type(void *arg)
{
	struct ac_cdvdsif_reply *argr;

	argr = (struct ac_cdvdsif_reply *)arg;
	argr->result = cdc_medium();
	return 1;
}

static int cde_op_error(void *arg)
{
	struct ac_cdvdsif_reply *argr;

	argr = (struct ac_cdvdsif_reply *)arg;
	argr->result = cdc_error();
	return 1;
}

static int cde_op_getpos(void *arg)
{
	struct ac_cdvdsif_reply *argr;

	argr = (struct ac_cdvdsif_reply *)arg;
	argr->result = cdc_getpos();
	return 0;
}

static int cde_op_init(void *arg)
{
	int mode;
	int ret;
	const struct ac_cdvdsif_init *argt;
	struct ac_cdvdsif_reply *argr;

	argt = (struct ac_cdvdsif_init *)arg;
	argr = (struct ac_cdvdsif_reply *)arg;
	mode = argt->mode;
	if ( mode < 0 )
	{
		ret = 0;
	}
	else if ( mode < 2 )
	{
		int ret_v2;

		if ( cdc_module_status() > 0 )
			cdc_module_stop();
		ret_v2 = cdc_module_start(0, 0);
		if ( ret_v2 >= 0 )
		{
			ret = 1;
			if ( !mode )
			{
				cdc_ready(0);
				ret = 1;
			}
		}
		else
		{
			printf("cde:init: START error %d\n", ret_v2);
			ret = 0;
		}
	}
	else
	{
		ret = mode == 5 && cdc_module_stop() >= 0;
	}
	argr->result = ret;
	return ret;
}

static int cde_op_pause(void *arg)
{
	int result;
	struct ac_cdvdsif_reply *argr;

	argr = (struct ac_cdvdsif_reply *)arg;
	result = cdc_pause(0);
	argr->result = result;
	return result;
}

static int cde_read_xfer(void *dst, void *src, int len, enum cdc_xfer_dir dir)
{
	if ( dir == CDC_XFER_SYNC )
		return len;
	if ( acMemWait(&Cdec.rd_mem, 100, 120) < 0 )
		return -116;
	acMemSetup(&Cdec.rd_mem, src, len);
	return acMemSend(&Cdec.rd_mem, (acMemEEaddr)dst, len, 10);
}

static int cde_op_read(void *arg)
{
	int result;
	struct ac_cdvdsif_read *argt;
	struct ac_cdvdsif_reply *argr;

	argt = (struct ac_cdvdsif_read *)arg;
	argr = (struct ac_cdvdsif_reply *)arg;
	acMemSetup(&Cdec.rd_mem, 0, 0);
	result = cdc_read(argt->lsn, argt->buf, argt->sectors, &argt->rmode, (cdc_xfer_t)cde_read_xfer, 0);
	argr->result = result;
	return result;
}

static int cde_op_readtoc(void *arg)
{
	int result;
	struct ac_cdvdsif_readtoc *argt;
	struct ac_cdvdsif_reply *argr;

	argt = (struct ac_cdvdsif_readtoc *)arg;
	argr = (struct ac_cdvdsif_reply *)arg;
	acMemSetup(&Cdec.rd_mem, 0, 0);
	result = cdc_readtoc(argt->toc, (cdc_xfer_t)cde_read_xfer);
	argr->result = result;
	return result;
}

static void cde_read_acram_cb(acRamT ram, void *arg, int result)
{
	int thid;

	(void)ram;
	(void)result;
	thid = *(int *)arg;
	*(int *)arg = 0;
	// cppcheck-suppress knownConditionTrueFalse
	if ( thid )
		WakeupThread(thid);
}

static int cde_read_acram(void *dst, void *src, int len, enum cdc_xfer_dir dir)
{
	acRamData *v7;
	int ret;
	acRamData ram_data;
	int thid;

	(void)dir;
	thid = GetThreadId();
	v7 = acRamSetup(&ram_data, cde_read_acram_cb, &thid, 0);
	ret = acRamWrite(v7, (acRamAddr)dst, src, len);
	if ( ret < 0 )
	{
		return ret;
	}
	while ( thid )
	{
		SleepThread();
	}
	return ret;
}

static int cde_op_readi(void *arg)
{
	unsigned int v2;
	int result;
	struct ac_cdvdsif_read *argt;
	struct ac_cdvdsif_reply *argr;
	int flg;

	argt = (struct ac_cdvdsif_read *)arg;
	argr = (struct ac_cdvdsif_reply *)arg;
	flg = 0;
	v2 = (unsigned int)argt->buf;
	if ( (v2 & 0xF0000001) == 0x40000000 )
	{
		v2 &= 0xFFFFFFEu;
		flg = 1;
	}
	result = cdc_read(argt->lsn, (void *)v2, argt->sectors, &argt->rmode, flg ? cde_read_acram : 0, 0);
	argr->result = result;
	return result;
}

static int cde_op_sync(void *arg)
{
	int mode;
	int v3;
	const struct ac_cdvdsif_sync *argt;
	struct ac_cdvdsif_sync_rpl *argr;

	argt = (struct ac_cdvdsif_sync *)arg;
	argr = (struct ac_cdvdsif_sync_rpl *)arg;
	mode = argt->mode;
	argr->fno = cdc_getfno();
	v3 = cdc_sync(mode);
	argr->result = v3;
	if ( mode < 0 && v3 )
		argr->rpos = cdc_getpos();
	else
		argr->rpos = 0;
	return 0;
}

static int cde_lookup_xfer(void *dst, void *src, int len, enum cdc_xfer_dir dir)
{
	int v5;
	acMemData mem_data;

	v5 = len;
	if ( !dir )
	{
		return v5;
	}
	acMemSetup(&mem_data, dst, len);
	return acMemReceive(&mem_data, (acMemEEaddr)src, v5);
}

static int cde_op_lookup(void *arg)
{
	int result;
	const struct ac_cdvdsif_lookup *argt;
	struct ac_cdvdsif_lookup_rpl *argr;

	argt = (struct ac_cdvdsif_lookup *)arg;
	argr = (struct ac_cdvdsif_lookup_rpl *)arg;
	result = cdc_lookup(&argr->file, (char *)argt->name, argt->namlen, (cdc_xfer_t)cde_lookup_xfer);
	argr->result = result;
	return result;
}

static int cde_op_seek(void *arg)
{
	int result;
	const struct ac_cdvdsif_seek *argt;
	struct ac_cdvdsif_reply *argr;

	argt = (struct ac_cdvdsif_seek *)arg;
	argr = (struct ac_cdvdsif_reply *)arg;
	result = cdc_seek(argt->lsn, 0);
	argr->result = result;
	return result;
}

static int cde_op_standby(void *arg)
{
	int result;
	struct ac_cdvdsif_reply *argr;

	argr = (struct ac_cdvdsif_reply *)arg;
	result = cdc_standby(0);
	argr->result = result;
	return result;
}

static int cde_op_stat(void *arg)
{
	struct ac_cdvdsif_reply *argr;

	argr = (struct ac_cdvdsif_reply *)arg;
	argr->result = cdc_stat();
	return 1;
}

static int cde_op_stop(void *arg)
{
	int result;
	struct ac_cdvdsif_reply *argr;

	argr = (struct ac_cdvdsif_reply *)arg;
	result = cdc_stop(0);
	argr->result = result;
	return result;
}

static int cde_op_tray(void *arg)
{
	int result;
	const struct ac_cdvdsif_tray *argt;
	struct ac_cdvdsif_tray_rpl *argr;

	argt = (struct ac_cdvdsif_tray *)arg;
	argr = (struct ac_cdvdsif_tray_rpl *)arg;
	result = cdc_tray(argt->mode, &argr->status);
	argr->result = result;
	return result;
}

static int cde_op_inits(void *arg)
{
	int result;
	struct ac_cdvdsif_inits *argt;
	struct ac_cdvdsif_reply *argr;

	argt = (struct ac_cdvdsif_inits *)arg;
	argr = (struct ac_cdvdsif_reply *)arg;
	result = cdc_inits(argt->buf, argt->size, argt->bsize);
	argr->result = result;
	return result;
}

static int cde_reads_xfer(void *dst, void *src, int len, enum cdc_xfer_dir dir)
{
	int index;
	acMemT mem;
	int index_v2;

	if ( !dir )
	{
		int index_v3;

		index_v3 = Cdec.st_index - 1;
		if ( Cdec.st_index - 1 < 0 )
			index_v3 = 1;
		if ( acMemWait(&Cdec.st_mem[index_v3], 100, 120) < 0 )
			return -116;
		return 9;
	}
	index = Cdec.st_index;
	mem = &Cdec.st_mem[Cdec.st_index];
	if ( acMemWait(mem, 100, 120) < 0 )
		return -116;
	acMemSetup(mem, src, len);
	index_v2 = index + 1;
	if ( acMemSend(mem, (acMemEEaddr)dst, len, 10) < 0 )
	{
		return -116;
	}
	if ( (unsigned int)index_v2 >= 2 )
		index_v2 = 0;
	Cdec.st_index = index_v2;
	return len;
}

static int cde_op_reads(void *arg)
{
	int result;
	struct ac_cdvdsif_reads *argt;
	struct ac_cdvdsif_reads_rpl *argr;

	argt = (struct ac_cdvdsif_reads *)arg;
	argr = (struct ac_cdvdsif_reads_rpl *)arg;
	result = cdc_reads(argt->buf, argt->sectors, argt->mode, (int *)&argr->error, (cdc_xfer_t)cde_reads_xfer);
	argr->result = result;
	return result;
}

static int cde_op_seeks(void *arg)
{
	int result;
	const struct ac_cdvdsif_seeks *argt;
	struct ac_cdvdsif_reply *argr;

	argt = (struct ac_cdvdsif_seeks *)arg;
	argr = (struct ac_cdvdsif_reply *)arg;
	result = cdc_seeks(argt->lsn);
	argr->result = result;
	return result;
}

static int cde_op_starts(void *arg)
{
	int result;
	struct ac_cdvdsif_starts *argt;
	struct ac_cdvdsif_reply *argr;

	argt = (struct ac_cdvdsif_starts *)arg;
	argr = (struct ac_cdvdsif_reply *)arg;
	result = cdc_starts(argt->lsn, &argt->rmode);
	argr->result = result;
	return result;
}

static int cde_op_stats(void *arg)
{
	int result;
	struct ac_cdvdsif_reply *argr;

	argr = (struct ac_cdvdsif_reply *)arg;
	result = cdc_stats();
	argr->result = result;
	return result;
}

static int cde_op_stops(void *arg)
{
	int result;
	struct ac_cdvdsif_reply *argr;

	argr = (struct ac_cdvdsif_reply *)arg;
	result = cdc_stops();
	argr->result = result;
	return result;
}

static int cde_op_pauses(void *arg)
{
	int result;
	struct ac_cdvdsif_reply *argr;

	argr = (struct ac_cdvdsif_reply *)arg;
	result = cdc_pauses();
	argr->result = result;
	return result;
}

static int cde_op_resumes(void *arg)
{
	int result;
	struct ac_cdvdsif_reply *argr;

	argr = (struct ac_cdvdsif_reply *)arg;
	result = cdc_resumes();
	argr->result = result;
	return result;
}

static int cde_op_readrtc(void *arg)
{
	int result;
	struct ac_cdvdsif_readrtc_rpl *argr;

	argr = (struct ac_cdvdsif_readrtc_rpl *)arg;
	result = sceCdReadClock(&argr->rtc);
	argr->result = result;
	return result;
}

static void *cde_request(unsigned int fno, struct cde_softc *data, int size)
{
	struct ac_cdvdsif_reply *rpl;
	cde_ops_t func;
	int ret;
	int v11;
	acSpl state;

	rpl = (struct ac_cdvdsif_reply *)data->rpl;
	if ( size != 16 )
	{
		rpl->error = 33;
		rpl->result = -1;
		return rpl;
	}
	if ( fno >= (sizeof(Cde_ops) / sizeof(Cde_ops[0])) || (func = Cde_ops[fno]) == 0 )
	{
		rpl->error = 16;
		rpl->result = -1;
		return rpl;
	}
	CpuSuspendIntr(&state);
	if ( data->fno )
	{
		ret = 1;
	}
	else
	{
		data->fno = fno;
		ret = 0;
	}
	CpuResumeIntr(state);
	if ( ret )
	{
		rpl->error = 19;
		rpl->result = -1;
		return rpl;
	}
	rpl->error = *(acUint32 *)data->cal;
	rpl->result = *(acUint32 *)&data->cal[4];
	rpl->padding[0] = *(acUint32 *)&data->cal[8];
	rpl->padding[1] = *(acUint32 *)&data->cal[12];
	v11 = func(rpl);
	if ( v11 >= 0 )
	{
		if ( v11 )
			rpl->error = 0;
		else
			rpl->error = cdc_error();
	}
	data->fno = AC_CDVDSIF_ID_NOP;
	return rpl;
}

static void cde_thread(void *arg)
{
	int thid;
	SifRpcDataQueue_t queue_data;
	SifRpcServerData_t serv_data;
	struct cde_softc *argt;

	argt = (struct cde_softc *)arg;
	thid = GetThreadId();
	sceSifSetRpcQueue(&queue_data, thid);
	sceSifRegisterRpc(&serv_data, 0x76500002, (SifRpcFunc_t)cde_request, argt, 0, 0, &queue_data);
	argt->thid = thid;
	while ( 1 )
	{
		SifRpcServerData_t *s;

		s = sceSifGetNextRequest(&queue_data);
		if ( s )
		{
			sceSifExecRequest(s);
		}
		else
		{
			SleepThread();
			if ( thid != argt->thid )
				ExitThread();
		}
	}
}

static void cde_term_thread(struct cde_softc *cdec)
{
	int thid;

	thid = cdec->thid;
	if ( thid > 0 )
	{
		int retry;
		int ret;

		cdec->thid = 0;
		WakeupThread(thid);
		DelayThread(1000);
		for ( retry = 9; retry >= 0; --retry )
		{
			ret = DeleteThread(thid);
			if ( !ret )
				break;
			DelayThread(1000000);
		}
		if ( retry < 0 )
			printf("accdvde:term_thread: TIMEDOUT %d\n", ret);
	}
}

static int cde_init_thread(struct cde_softc *cdec, int prio)
{
	int th;
	iop_thread_t param;

	param.attr = 0x2000000;
	param.thread = cde_thread;
	param.priority = prio;
	param.stacksize = 4096;
	param.option = 0;
	th = CreateThread(&param);
	if ( th > 0 )
		StartThread(th, cdec);
	return th;
}

int acCdvdeModuleStatus()
{
	int ret;
	int state;

	CpuSuspendIntr(&state);
	if ( Cdec.fno )
		ret = 2;
	else
		ret = Cdec.thid != 0;
	CpuResumeIntr(state);
	return ret;
}

int acCdvdeModuleStart(int argc, char **argv)
{
	int index;
	int prio;
	char **v8;
	char *opt;
	int v10;
	const char *opt_v7;
	int value;
	int inited;
	char *next;

	if ( acCdvdeModuleStatus() != 0 )
	{
		return -16;
	}
	index = 1;
	prio = 80;
	v8 = argv + 1;
	while ( index < argc )
	{
		opt = *v8;
		if ( **v8 == 45 )
		{
			v10 = opt[1];
			opt_v7 = opt + 2;
			if ( v10 == 112 )
			{
				value = strtol(opt_v7, &next, 0);
				if ( next != opt_v7 )
					prio = value;
			}
		}
		++index;
		++v8;
	}
	memset(&Cdec, 0, sizeof(Cdec));
	inited = cde_init_thread(&Cdec, prio);
	if ( inited <= 0 )
		return -6;
	return 0;
}

int acCdvdeModuleStop()
{
	if ( acCdvdeModuleStatus() != 0 )
	{
		cde_term_thread(&Cdec);
	}
	return 0;
}

int acCdvdeModuleRestart(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	return -88;
}
