/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# audsrv EE-side RPC code.
*/

/**
 * \file audsrv_rpc.c
 * \author gawd (Gil Megidish)
 * \date 04-24-05
 */

#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>
#include <tamtypes.h>
#include <string.h>
#include <iopheap.h>

#include <audsrv.h>
#include "audsrv_rpc.c.h"

#define MIN(a,b) ((a) <= (b)) ? (a) : (b)

/* rpc mambo jambo */
static struct t_SifRpcClientData cd0;
static unsigned int sbuff[4096] __attribute__((aligned (64)));

static int initialized = 0;
static int audsrv_error = AUDSRV_ERR_NOERROR;

/* nasty forwards */
static void fillbuf_requested(void *pkt, void *arg);
static void cdda_stopped(void *pkt, void *arg);

/** Internal function to set last error
    @param err
*/
static void set_error(int err)
{
	audsrv_error = err;
}

/** Returns the last error audsrv raised
    @returns error code
*/
int audsrv_get_error()
{
	return audsrv_error;
}

/** Internal function to simplify RPC calling
    @param func    procedure to invoke
    @param arg     optional argument
    @returns value returned by RPC server
*/
static int call_rpc_1(int func, int arg)
{
	sbuff[0] = arg;
	SifCallRpc(&cd0, func, 0, sbuff, 1*4, sbuff, 4, 0, 0);
	FlushCache(0);

	set_error(sbuff[0]);
	return sbuff[0];
}

/** Internal function to simplify RPC calling
    @param func    procedure to invoke
    @param arg1    optional argument
    @param arg2    optional argument
    @returns value returned by RPC server
*/
static int call_rpc_2(int func, int arg1, int arg2)
{
	sbuff[0] = arg1;
	sbuff[1] = arg2;
	SifCallRpc(&cd0, func, 0, sbuff, 2*4, sbuff, 4, 0, 0);
	FlushCache(0);

	set_error(sbuff[0]);
	return sbuff[0];
}

/** Shutdowns audsrv
    @returns AUDSRV_ERR_NOERROR
*/
int audsrv_quit()
{
	SifCallRpc(&cd0, AUDSRV_QUIT, 0, sbuff, 1*4, sbuff, 4, 0, 0);
	set_error(AUDSRV_ERR_NOERROR);
	return 0;
}

/** Configures audio stream
    @param fmt output specification structure
    @returns 0 on success, or one of the error codes otherwise

    This sets up audsrv to accept stream in this format and convert
    it to SPU2's native format if required. Note: it is possible to
    change the format at any point. You might want to stop audio prior
    to that, to prevent mismatched audio output.
*/
int audsrv_set_format(struct audsrv_fmt_t *fmt)
{
	sbuff[0] = fmt->freq;
	sbuff[1] = fmt->bits;
	sbuff[2] = fmt->channels;
	SifCallRpc(&cd0, AUDSRV_SET_FORMAT, 0, sbuff, 3*4, sbuff, 4, 0, 0);
	FlushCache(0);
	set_error(sbuff[0]);
	return sbuff[0];
}

/** Blocks until there is enough space to enqueue chunk
    @param bytes size of chunk requested to be enqueued (in bytes)
    @returns error code
    
    Blocks until there are enough space to store the upcoming chunk
    in audsrv's internal ring buffer.
*/
int audsrv_wait_audio(int bytes)
{
	return call_rpc_1(AUDSRV_WAIT_AUDIO, bytes);
}

/** Sets output volume
    @param vol volume in SPU2 units [MIN_VOLUME .. MAX_VOLUME]
    @returns error code
*/
int audsrv_set_volume(int volume)
{
	return call_rpc_1(AUDSRV_SET_VOLUME, volume);
}

/** Starts playing the request track
    @param track segment to play
    @returns status code
*/
int audsrv_play_cd(int track)
{
	return call_rpc_1(AUDSRV_PLAY_CD, track);
}

/** Starts playing at a specific sector
    @param start first sector to play
    @param end   last sector to play
    @returns status code
*/
int audsrv_play_sectors(int start, int end)
{
	return call_rpc_2(AUDSRV_PLAY_SECTORS, start, end);
}

/** Stops CD from playing.
    @returns status code
*/
int audsrv_stop_cd()
{
	int ret; 
	ret = call_rpc_1(AUDSRV_STOP_CD, 0);
	return ret;
}

/** Returns the current playing sector
    @returns sector number

    CDDA type discs have sector size of 2352 bytes. There are 75
    such sectors per second.
*/
int audsrv_get_cdpos()
{
	return call_rpc_1(AUDSRV_GET_CDPOS, 0);
}

/** Returns the current playing sector, relative to track
    @returns sector number

    There are 75 sectors a second. To translate this position to mm:ss:ff 
    use the following:
    mm = sector / (75*60)
    ss = (sector / 75) % 60
    ff = sector % 75 

    where ff is the frame number, 1/75th of a second.
*/
int audsrv_get_trackpos()
{
	return call_rpc_1(AUDSRV_GET_TRACKPOS, 0);
}

/** Returns the number of tracks available on the CD in tray
    @returns positive track count, or negative error status code
*/
int audsrv_get_numtracks()
{
	return call_rpc_1(AUDSRV_GET_NUMTRACKS, 0);
}

/** Returns the first sector for the given track
    @param track   track index, must be between 1 and the trackcount
    @returns sector number, or negative status code
*/
int audsrv_get_track_offset(int track)
{
	return call_rpc_1(AUDSRV_GET_TRACKOFFSET, track);
}

/** Pauses CDDA playing
    @returns error status code

    If CDDA is paused, no operation is taken
*/
int audsrv_pause_cd()
{
	return call_rpc_1(AUDSRV_PAUSE_CD, 0);
}

/** Resumes CDDA playing
    @returns error status code

    If CDDA was not paused, no operation is taken
*/
int audsrv_resume_cd()
{
	return call_rpc_1(AUDSRV_RESUME_CD, 0);
}

/** Returns the type of disc currently in tray
    @returns value as defined in libcdvd, negative on error
*/
int audsrv_get_cd_type()
{
	return call_rpc_1(AUDSRV_GET_CD_TYPE, 0);
}

/** Returns the status of the CD tray (open, closed, seeking etc.)
    @returns value as defined in libcdvd, negative on error
*/
int audsrv_get_cd_status()
{
	return call_rpc_1(AUDSRV_GET_CD_STATUS, 0);
}

/** Uploads audio buffer to SPU
    @param chunk   audio buffer
    @param bytes   size of chunk in bytes
    @returns positive number of bytes sent to processor or negative error status

    Plays an audio buffer; It will not interrupt a playing
    buffer, rather queue it up and play it as soon as possible without
    interfering with fluent streaming. The buffer and buflen are given
    in host format (i.e, 11025hz 8bit stereo.)
*/
int audsrv_play_audio(const char *chunk, int bytes)
{
	int copy, maxcopy;
	int packet_size;
	int sent = 0;

	set_error(AUDSRV_ERR_NOERROR);
	maxcopy = sizeof(sbuff) - sizeof(int);
	while (bytes > 0)
	{
		copy = MIN(bytes, maxcopy);
		sbuff[0] = copy;
		memcpy(&sbuff[1], chunk, copy);
		packet_size = copy + sizeof(int);
		SifCallRpc(&cd0, AUDSRV_PLAY_AUDIO, 0, sbuff, packet_size, sbuff, 1*4, 0, 0);
		FlushCache(0);

		if (sbuff[0] < 0)
		{
			/* there was an error */
			set_error(-sbuff[0]);
			break;
		}

		chunk = chunk + copy;
		bytes = bytes - copy;
		sent = sent + sbuff[0];
	}

	return sent;
}

/** Stops audio from playing.
    @returns status code
*/
int audsrv_stop_audio()
{
	int ret; 
	ret = call_rpc_1(AUDSRV_STOP_AUDIO, 0);
	return ret;
}

/** Initializes audsrv library
    @returns error code
*/
int audsrv_init()
{
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

	SifCallRpc(&cd0, AUDSRV_INIT, 0, sbuff, 64, sbuff, 64, 0, 0);
	FlushCache(0);
	ret = sbuff[0];
	if (ret != 0)
	{
		set_error(ret);
		return ret;
	}

	/* register a callback handler */
	DI();
	SifAddCmdHandler(AUDSRV_CDDA_CALLBACK, cdda_stopped, 0);
	SifAddCmdHandler(AUDSRV_FILLBUF_CALLBACK, fillbuf_requested, 0);
	EI();

	/* initialize IOP heap (for adpcm samples) */
	SifInitIopHeap();

	set_error(AUDSRV_ERR_NOERROR);
	return AUDSRV_ERR_NOERROR;
}

/** Initializes adpcm unit of audsrv

    @returns zero on success, negative value on error
*/
int audsrv_adpcm_init()
{
	return call_rpc_1(AUDSRV_INIT_ADPCM, 0);
}

/** Uploads a sample to SPU2 memory
    @param adpcm    adpcm descriptor structure
    @param buffer   pointer to adpcm sample
    @param size     size of sample (including the header)
    @returns zero on success, negative error code otherwise
*/
int audsrv_load_adpcm(audsrv_adpcm_t *adpcm, void *buffer, int size)
{
	void* iop_addr;
	SifDmaTransfer_t sifdma;
	int id;

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
	id = SifSetDma(&sifdma, 1);
	while(SifDmaStat(id) >= 0);
	
	sbuff[0] = (int)iop_addr;
	sbuff[1] = size;
	sbuff[2] = (int)adpcm; /* use as id */

	SifCallRpc(&cd0, AUDSRV_LOAD_ADPCM, 0, sbuff, 12, sbuff, 16, 0, 0);
	SifFreeIopHeap(iop_addr);

	if(sbuff[0] != 0) 
	{
		adpcm->buffer = 0;	
		return sbuff[0];
	}
	else
	{
		adpcm->buffer = buffer;
		adpcm->size = size;	
		adpcm->pitch = sbuff[1];
		adpcm->loop = sbuff[2];
		adpcm->channels = sbuff[3];
		return AUDSRV_ERR_NOERROR;
	}
}

/** Plays an adpcm sample already uploaded with audsrv_load_adpcm()
    @param adpcm   exact same adpcm descriptor used in load()
    @returns zero on success, negative value on error

    The sample will be played in an unoccupied channel. If all 24 channels
    are used, then -AUDSRV_ERR_NO_MORE_CHANNELS is returned. Trying to play
    a sample which is unavailable will result in -AUDSRV_ERR_ARGS
*/
int audsrv_play_adpcm(audsrv_adpcm_t *adpcm)
{
	/* on iop side, the sample id is like the pointer on ee side */
	return call_rpc_1(AUDSRV_PLAY_ADPCM, (u32)adpcm);
}

/** Translates audsrv_get_error() response to readable string
    @returns string representation of error code
*/
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

static audsrv_callback_t on_cdda_stop = 0;
static void *on_cdda_stop_arg = 0;

static audsrv_callback_t on_fillbuf = 0;
static void *on_fillbuf_arg = 0;

/** Installs a callback function upon completion of a cdda track
    @param cb your callback
    @param arg extra parameter to pass to callback function later
    @returns status code
*/
int audsrv_on_cdda_stop(audsrv_callback_t cb, void *arg)
{
	on_cdda_stop = cb;
	on_cdda_stop_arg = arg;
	return AUDSRV_ERR_NOERROR;
}

/** Installs a callback function to be called when ringbuffer has enough
    space to transmit the request number of bytes.
    @param bytes request a callback when this amount of bytes is available
    @param cb your callback
    @param arg extra parameter to pass to callback function later
    @returns AUDSRV_ERR_NOERROR, AUDSRV_ERR_ARGS if amount is greater than sizeof(ringbuf)
*/
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

/** SIF command handler */
static void cdda_stopped(void *pkt, void *arg)
{
	if (on_cdda_stop != 0)
	{
		on_cdda_stop(on_cdda_stop_arg);
	}
}

/** SIF command handler */
static void fillbuf_requested(void *pkt, void *arg)
{
	if (on_fillbuf != 0)
	{
		on_fillbuf(on_fillbuf_arg);
	}
}
