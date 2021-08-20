/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * audsrv EE-side RPC code.
 */

#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>
#include <tamtypes.h>
#include <string.h>
#include <iopheap.h>

#include <audsrv.h>
#include "audsrv_rpc.h"

#define MIN(a,b) ((a) <= (b)) ? (a) : (b)

/* rpc mambo jambo */
static struct t_SifRpcClientData cd0;
static unsigned int sbuff[4096] __attribute__((aligned (64)));
static struct t_SifRpcDataQueue cb_queue;
static struct t_SifRpcServerData cb_srv;
static unsigned char rpc_server_stack[0x1800] __attribute__((aligned (16)));

extern void *_gp;

static int initialized = 0;
static int rpc_server_thread_id;
static int audsrv_error = AUDSRV_ERR_NOERROR;
static int completion_sema;

static audsrv_callback_t on_cdda_stop = NULL;
static void *on_cdda_stop_arg = NULL;

static audsrv_callback_t on_fillbuf = NULL;
static void *on_fillbuf_arg = NULL;

static const unsigned short vol_values[26] =
{
	0x0000,
	0x0000, 0x0096, 0x0190, 0x0230, 0x0320,
	0x042E, 0x0532, 0x05FA, 0x06C2, 0x088E,
	0x09F6, 0x0BC2, 0x0DC0, 0x0FF0, 0x118A,
	0x1482, 0x1752, 0x1B4E, 0x1F40, 0x2378,
	0x28D2, 0x2EFE, 0x34F8, 0x3A5C, 0x3FFF
};

/** Internal function to set last error
 * @param err
 */
static void set_error(int err)
{
	audsrv_error = err;
}

int audsrv_get_error()
{
	return audsrv_error;
}

/** Internal function to simplify RPC calling
 * @param func    procedure to invoke
 * @param arg     optional argument
 * @returns value returned by RPC server
*/
static int call_rpc_1(int func, int arg)
{
	int ret;

	WaitSema(completion_sema);

	sbuff[0] = arg;
	SifCallRpc(&cd0, func, 0, sbuff, 1*4, sbuff, 4, NULL, NULL);

	ret = sbuff[0];
	SignalSema(completion_sema);

	set_error(ret);

	return ret;
}

/** Internal function to simplify RPC calling
 * @param func    procedure to invoke
 * @param arg1    optional argument
 * @param arg2    optional argument
 * @returns value returned by RPC server
*/
static int call_rpc_2(int func, int arg1, int arg2)
{
	int ret;

	WaitSema(completion_sema);

	sbuff[0] = arg1;
	sbuff[1] = arg2;
	SifCallRpc(&cd0, func, 0, sbuff, 2*4, sbuff, 4, NULL, NULL);

	ret = sbuff[0];
	SignalSema(completion_sema);

	set_error(ret);

	return ret;
}

int audsrv_quit()
{
	WaitSema(completion_sema);

	SifCallRpc(&cd0, AUDSRV_QUIT, 0, sbuff, 1*4, sbuff, 4, NULL, NULL);
	set_error(AUDSRV_ERR_NOERROR);

	SifRemoveRpc(&cb_srv, &cb_queue);
	SifRemoveRpcQueue(&cb_queue);
	TerminateThread(rpc_server_thread_id);
	DeleteThread(rpc_server_thread_id);

	DeleteSema(completion_sema);
	return 0;
}

int audsrv_set_format(struct audsrv_fmt_t *fmt)
{
	int ret;

	WaitSema(completion_sema);

	sbuff[0] = fmt->freq;
	sbuff[1] = fmt->bits;
	sbuff[2] = fmt->channels;
	SifCallRpc(&cd0, AUDSRV_SET_FORMAT, 0, sbuff, 3*4, sbuff, 4, NULL, NULL);

	ret = sbuff[0];
	SignalSema(completion_sema);

	set_error(ret);

	return ret;
}

int audsrv_wait_audio(int bytes)
{
	return call_rpc_1(AUDSRV_WAIT_AUDIO, bytes);
}

int audsrv_set_volume(int volume)
{
	if (volume > MAX_VOLUME)
	{
		volume = MAX_VOLUME;
	}
	else if (volume < MIN_VOLUME)
	{
		volume = MIN_VOLUME;
	}

	return call_rpc_1(AUDSRV_SET_VOLUME, vol_values[volume/4]);
}

int audsrv_play_cd(int track)
{
	return call_rpc_1(AUDSRV_PLAY_CD, track);
}

int audsrv_play_sectors(int start, int end)
{
	return call_rpc_2(AUDSRV_PLAY_SECTORS, start, end);
}

int audsrv_stop_cd()
{
	int ret;
	ret = call_rpc_1(AUDSRV_STOP_CD, 0);
	return ret;
}

int audsrv_get_cdpos()
{
	return call_rpc_1(AUDSRV_GET_CDPOS, 0);
}

int audsrv_get_trackpos()
{
	return call_rpc_1(AUDSRV_GET_TRACKPOS, 0);
}

int audsrv_get_numtracks()
{
	return call_rpc_1(AUDSRV_GET_NUMTRACKS, 0);
}

int audsrv_get_track_offset(int track)
{
	return call_rpc_1(AUDSRV_GET_TRACKOFFSET, track);
}

int audsrv_pause_cd()
{
	return call_rpc_1(AUDSRV_PAUSE_CD, 0);
}

int audsrv_resume_cd()
{
	return call_rpc_1(AUDSRV_RESUME_CD, 0);
}

int audsrv_get_cd_type()
{
	return call_rpc_1(AUDSRV_GET_CD_TYPE, 0);
}

int audsrv_get_cd_status()
{
	return call_rpc_1(AUDSRV_GET_CD_STATUS, 0);
}

int audsrv_play_audio(const char *chunk, int bytes)
{
	int copy, maxcopy, copied;
	int packet_size;
	int sent = 0;

	set_error(AUDSRV_ERR_NOERROR);
	maxcopy = sizeof(sbuff) - sizeof(int);
	while (bytes > 0)
	{
		WaitSema(completion_sema);

		copy = MIN(bytes, maxcopy);
		sbuff[0] = copy;
		memcpy(&sbuff[1], chunk, copy);
		packet_size = copy + sizeof(int);
		SifCallRpc(&cd0, AUDSRV_PLAY_AUDIO, 0, sbuff, packet_size, sbuff, 1*4, NULL, NULL);

		copied = sbuff[0];
		SignalSema(completion_sema);

		if (copied < 0)
		{
			/* there was an error */
			set_error(-copied);
			break;
		}

		chunk = chunk + copy;
		bytes = bytes - copy;
		sent = sent + copied;
	}

	return sent;
}

int audsrv_stop_audio()
{
	int ret;
	ret = call_rpc_1(AUDSRV_STOP_AUDIO, 0);
	return ret;
}

static void *audsrv_ee_rpc_handler(int fnum, void *buffer, int len)
{
	switch(fnum){
		case AUDSRV_FILLBUF_CALLBACK:
			if (on_fillbuf != NULL)
				on_fillbuf(on_fillbuf_arg);
			break;
		case AUDSRV_CDDA_CALLBACK:
			if (on_cdda_stop != NULL)
				on_cdda_stop(on_cdda_stop_arg);
			break;
	}

	return buffer;
}

static void rpc_server_thread(void *arg)
{
	static unsigned char cb_rpc_buffer[64] __attribute__((aligned(64)));

	SifSetRpcQueue(&cb_queue, GetThreadId());
	SifRegisterRpc(&cb_srv, AUDSRV_IRX, &audsrv_ee_rpc_handler, cb_rpc_buffer, NULL, NULL, &cb_queue);
	SifRpcLoop(&cb_queue);
}

int audsrv_init()
{
	ee_sema_t compSema;
	ee_thread_t rpcThread;
	int ret;

	if (initialized)
	{
		/* already done */
		return 0;
	}

	memset(&cd0, '\0', sizeof(cd0));

	while (1)
	{
		if (SifBindRpc(&cd0, AUDSRV_IRX, 0) < 0)
		{
			set_error(AUDSRV_ERR_RPC_FAILED);
			return -1;
		}

 		if (cd0.server != 0)
		{
			break;
		}

		nopdelay();
	}

	compSema.init_count = 1;
	compSema.max_count = 1;
	compSema.option = 0;
	completion_sema = CreateSema(&compSema);
	if (completion_sema < 0)
	{
		set_error(AUDSRV_ERR_FAILED_TO_CREATE_SEMA);
		return -1;
	}

	/* Create RPC server */
	rpcThread.attr = 0;
	rpcThread.option = 0;
	rpcThread.func = &rpc_server_thread;
	rpcThread.stack = rpc_server_stack;
	rpcThread.stack_size = sizeof(rpc_server_stack);
	rpcThread.gp_reg = &_gp;
	rpcThread.initial_priority = 0x60;
	rpc_server_thread_id = CreateThread(&rpcThread);
	StartThread(rpc_server_thread_id, NULL);

	SifCallRpc(&cd0, AUDSRV_INIT, 0, sbuff, 64, sbuff, 64, NULL, NULL);
	ret = sbuff[0];
	if (ret != 0)
	{
		set_error(ret);
		return ret;
	}

	/* initialize IOP heap (for adpcm samples) */
	SifInitIopHeap();

	set_error(AUDSRV_ERR_NOERROR);
	return AUDSRV_ERR_NOERROR;
}

int audsrv_adpcm_init()
{
	return call_rpc_1(AUDSRV_INIT_ADPCM, 0);
}

int audsrv_adpcm_set_volume(int ch, int volume)
{
	if (volume > MAX_VOLUME)
	{
		volume = MAX_VOLUME;
	}
	else if (volume < MIN_VOLUME)
	{
		volume = MIN_VOLUME;
	}

	return call_rpc_2(AUDSRV_ADPCM_SET_VOLUME, ch, vol_values[volume/4]);
}

int audsrv_load_adpcm(audsrv_adpcm_t *adpcm, void *buffer, int size)
{
	void* iop_addr;
	SifDmaTransfer_t sifdma;
	int id, ret;

	iop_addr = SifAllocIopHeap(size);
	if (iop_addr == 0)
	{
		return -AUDSRV_ERR_OUT_OF_MEMORY;
	}

	sifdma.src = buffer;
	sifdma.dest = iop_addr;
	sifdma.size = size;
	sifdma.attr = 0;

	/* send by dma */
	while((id = SifSetDma(&sifdma, 1)) == 0);
	while(SifDmaStat(id) >= 0);

	WaitSema(completion_sema);

	sbuff[0] = (int)iop_addr;
	sbuff[1] = size;
	sbuff[2] = (int)adpcm; /* use as id */

	SifCallRpc(&cd0, AUDSRV_LOAD_ADPCM, 0, sbuff, 12, sbuff, 16, NULL, NULL);

	if(sbuff[0] != 0)
	{
		adpcm->buffer = 0;
		ret = sbuff[0];
	}
	else
	{
		adpcm->buffer = buffer;
		adpcm->size = size;
		adpcm->pitch = sbuff[1];
		adpcm->loop = sbuff[2];
		adpcm->channels = sbuff[3];
		ret = AUDSRV_ERR_NOERROR;
	}

	SignalSema(completion_sema);

	SifFreeIopHeap(iop_addr);

	return ret;
}

int audsrv_ch_play_adpcm(int ch, audsrv_adpcm_t *adpcm)
{
	/* on iop side, the sample id is like the pointer on ee side */
	return call_rpc_2(AUDSRV_PLAY_ADPCM, ch, (u32)adpcm);
}

const char *audsrv_get_error_string()
{
	switch(audsrv_get_error())
	{
		case AUDSRV_ERR_NOERROR:
		return "No error";

		case AUDSRV_ERR_NOT_INITIALIZED:
		return "Not initialized";

		case AUDSRV_ERR_OUT_OF_MEMORY:
		return "Out of IOP memory";

		case AUDSRV_ERR_RPC_FAILED:
		return "RPC operation failed";

		case AUDSRV_ERR_FORMAT_NOT_SUPPORTED:
		return "Format not supported";

		case AUDSRV_ERR_NO_DISC:
		return "No disc in drive";
	}

	return "Unknown error";
}

int audsrv_on_cdda_stop(audsrv_callback_t cb, void *arg)
{
	on_cdda_stop = cb;
	on_cdda_stop_arg = arg;
	return AUDSRV_ERR_NOERROR;
}

int audsrv_on_fillbuf(int amount, audsrv_callback_t cb, void *arg)
{
	int err;

	on_fillbuf = 0;
	on_fillbuf_arg = 0;

	err = call_rpc_1(AUDSRV_SET_THRESHOLD, amount);
	if (err != 0)
	{
		return err;
	}

	on_fillbuf = cb;
	on_fillbuf_arg = arg;
	return AUDSRV_ERR_NOERROR;
}

int audsrv_available()
{
	return call_rpc_1(AUDSRV_AVAILABLE, 0);
}

int audsrv_queued()
{
	return call_rpc_1(AUDSRV_QUEUED, 0);
}

