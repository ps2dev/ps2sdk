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

#include <audsrv.h>
#include "audsrv_rpc.c.h"

#define MIN(a,b) ((a) <= (b)) ? (a) : (b)

/* rpc mambo jambo */
static struct t_SifRpcClientData cd0;
static unsigned int sbuff[4096] __attribute__((aligned (64)));

static int initialized = 0;
static int audsrv_error = AUDSRV_ERR_NOERROR;

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
	set_error(ret);
	return ret;
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
	}

	return "Unknown error";	
}
