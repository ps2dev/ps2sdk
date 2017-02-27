/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
*/

/**
 * @file
 * audsrv iop rpc server.
 */

#include <stdio.h>
#include <thbase.h>
#include <thsemap.h>
#include <loadcore.h>
#include <sysmem.h>
#include <intrman.h>
#include <sifcmd.h>

#include <audsrv.h>
#include "audsrv_internal.h"
#include "common.h"
#include "rpc_client.h"

/* rpc server variables */
/** buffer for RPC DMA */
static int rpc_buffer[18000/4];
/** RPC thread variables */
static SifRpcDataQueue_t qd;
/** RPC thread variables */
static SifRpcServerData_t sd0;


/** RPC command handler.
 * @param func     command (one of AUDSRV_x)
 * @param data     pointer to data array
 * @param size     size of data array (in bytes)
 * @returns value depends on function invoked
 *
 * This is a single rpc handler, it unpacks the data array and calls
 * local functions.
 */
static void *rpc_command(int func, unsigned *data, int size)
{
	int ret;

	/* printf("audsrv: rpc command %d\n", func); */
	switch(func)
	{
		case AUDSRV_INIT:
		ret = audsrv_init();
		initialize_rpc_client();
		break;

		case AUDSRV_FORMAT_OK:
		ret = audsrv_format_ok(data[0], data[1], data[2]);
		break;

		case AUDSRV_SET_FORMAT:
		ret = audsrv_set_format(data[0], data[1], data[2]);
		break;

		case AUDSRV_WAIT_AUDIO:
		ret = audsrv_wait_audio(data[0]);
		break;

		case AUDSRV_PLAY_AUDIO:
		ret = audsrv_play_audio((const char *)&data[1], data[0]);
		break;

		case AUDSRV_STOP_AUDIO:
		ret = audsrv_stop_audio();
		break;

		case AUDSRV_SET_VOLUME:
		ret = audsrv_set_volume(data[0]);
		break;

		case AUDSRV_QUIT:
		ret = audsrv_quit();
		break;

		case AUDSRV_PLAY_CD:
		ret = audsrv_play_cd(data[0]);
		break;

		case AUDSRV_STOP_CD:
		ret = audsrv_stop_cd();
		break;

		case AUDSRV_GET_CDPOS:
		ret = audsrv_get_cdpos();
		break;

		case AUDSRV_GET_TRACKPOS:
		ret = audsrv_get_trackpos();
		break;

		case AUDSRV_SET_THRESHOLD:
		ret = audsrv_set_threshold(data[0]);
		break;

		case AUDSRV_GET_NUMTRACKS:
		ret = audsrv_get_numtracks();
		break;

		case AUDSRV_GET_TRACKOFFSET:
		ret = audsrv_get_track_offset(data[0]);
		break;

		case AUDSRV_PLAY_SECTORS:
		ret = audsrv_cd_play_sectors(data[0], data[1]);
		break;

		case AUDSRV_GET_CD_STATUS:
		ret = audsrv_get_cd_status();
		break;

		case AUDSRV_GET_CD_TYPE:
		ret = audsrv_get_cd_type();
		break;

		case AUDSRV_PAUSE_CD:
		ret = audsrv_cd_pause();
		break;

		case AUDSRV_RESUME_CD:
		ret = audsrv_cd_resume();
		break;

		case AUDSRV_INIT_ADPCM:
		ret = audsrv_adpcm_init();
		break;

		case AUDSRV_LOAD_ADPCM:
		return audsrv_load_adpcm((u32*)data[0], data[1], data[2]);

		case AUDSRV_PLAY_ADPCM:
		ret = audsrv_play_adpcm(data[0]);
		break;

		default:
		ret = -1;
		break;
	}

	data[0] = ret;
	return data;
}

/** RPC listener thread
 * @param arg   not used

 * This is the main RPC thread. Nothing fancy here.
 */
static void rpc_server_thread(void *arg)
{
	SifInitRpc(0);

	printf("audsrv: creating rpc server\n");

	SifSetRpcQueue(&qd, GetThreadId());
	SifRegisterRpc(&sd0, AUDSRV_IRX, (void *)rpc_command, rpc_buffer, NULL, NULL, &qd);
	SifRpcLoop(&qd);
}

int initialize_rpc_thread()
{
	int rpc_tid;

	rpc_tid = create_thread(rpc_server_thread, 40, 0);
	printf("audsrv: rpc server thread 0x%x started\n", rpc_tid);
	return 0;
}
