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
 * audsrv adpcm module
 */

#include <stdio.h>
#include <thbase.h>
#include <thsemap.h>
#include <loadcore.h>
#include <sysmem.h>
#include <intrman.h>
#include <libsd.h>
#include <sysclib.h>

#include <audsrv.h>
#include "audsrv_internal.h"
#include "common.h"
#include "rpc_server.h"
#include "spu.h"

typedef struct adpcm_list_t
{
	struct adpcm_list_t *next;

	int id;
	int pitch;
	int loop;
	int channels;
	int size;
	int spu2_addr;
} adpcm_list_t;

static adpcm_list_t *adpcm_list_head = 0;
static adpcm_list_t *adpcm_list_tail = 0;

static u32 sbuffer[16] __attribute__((aligned(64)));

/** Allocates memory for a new sample. */
static adpcm_list_t *alloc_new_sample(void)
{
	void *buffer;
	int OldState;

	CpuSuspendIntr(&OldState);
	buffer = AllocSysMemory(ALLOC_FIRST, sizeof(adpcm_list_t), NULL);
	CpuResumeIntr(OldState);

	return buffer;
}

/** Frees up all memory taken by the linked list of samples */
static void free_all_samples()
{
	adpcm_list_t *p, *q;
	int OldState;

	CpuSuspendIntr(&OldState);
	for (p = adpcm_list_head; p != NULL; p = q)
	{
		q = p->next;
		FreeSysMemory(p);
	}

	adpcm_list_head = NULL;
	adpcm_list_tail = NULL;
	CpuResumeIntr(OldState);
}

/** Looks up the given identifier in list of loaded samples
 * @param id    sample identifier
 * @returns node entry from container, NULL on failure
 */
static adpcm_list_t *adpcm_loaded(int id)
{
	adpcm_list_t *cur = adpcm_list_head;

	while (cur != 0)
	{
		if (cur->id == id)
		{
			return cur;
		}

		cur = cur->next;
	}

	return NULL;
}

/** Extracts adpcm parameters from header
 * @param adpcm   node entry
 * @param buffer  pointer to adpcm header
 */
static void audsrv_read_adpcm_header(adpcm_list_t *adpcm, u32 *buffer)
{
	adpcm->pitch = buffer[2];
	adpcm->loop = (buffer[1] >> 16) & 0xFF;
	adpcm->channels = (buffer[1] >> 8) & 0xFF;
}

/** Uploads a sample to SPU2 memory
 * @param buffer    pointer to adpcm sample header
 * @param size      size of sample in bytes
 * @param id        sample identifier (to be later used in play())
 * @returns         pointer to local adpcm buffer
 */
void *audsrv_load_adpcm(u32 *buffer, int size, int id)
{
	adpcm_list_t *adpcm;

	adpcm = adpcm_loaded(id);
	if (adpcm == NULL)
	{
		if (adpcm_list_head == NULL)
		{
			/* first entry ever! yay! */
			adpcm = alloc_new_sample();
			adpcm->id = id;
			adpcm->spu2_addr = 0x5010; /* Need to change this so it considers to PCM streaming space usage :) */
			adpcm->size = size - 16; /* header is 16 bytes */
			adpcm->next = NULL;

			audsrv_read_adpcm_header(adpcm, buffer);

			adpcm_list_head = adpcm;
			adpcm_list_tail = adpcm_list_head;
		}
		else
		{
			/* add at the end of the list */
			adpcm = alloc_new_sample();
			adpcm->id = id;
			adpcm->spu2_addr = adpcm_list_tail->spu2_addr + adpcm_list_tail->size;
			adpcm->size = size - 16; /* header is 16 bytes */
			adpcm->next = NULL;

			audsrv_read_adpcm_header(adpcm, buffer);

			adpcm_list_tail->next = adpcm;
			adpcm_list_tail = adpcm;
		}

		/* DMA from IOP to SPU2 */
		sceSdVoiceTrans(AUDSRV_VOICE_DMA_CH, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, ((u8*)buffer)+16, (u32*)adpcm->spu2_addr, adpcm->size);
		sceSdVoiceTransStatus(AUDSRV_VOICE_DMA_CH, 1);
	}

	sbuffer[0] = 0;
	sbuffer[1] = adpcm->pitch;
	sbuffer[2] = adpcm->loop;
	sbuffer[3] = adpcm->channels;
	return sbuffer;
}

static int audsrv_adpcm_alloc_channel(void)
{
	int i, channel;
	u32 endx;

	endx = sceSdGetSwitch(SD_CORE_1 | SD_SWITCH_ENDX);
	if (endx == 0)
	{
		/* all channels are occupied */
		return -AUDSRV_ERR_NO_MORE_CHANNELS;
	}

	/* find first channel available */
	i = 1;
	channel = -1;
	while ((channel < 0) && (i < 24))
	{
		if (endx & (1 << i))
		{
			channel = i;
		}

        	i++;
	}

	if (channel == -1)
	{
		/* cannot find a single channel free */
		return -AUDSRV_ERR_NO_MORE_CHANNELS;
	}

	return channel;
}

/** Plays an adpcm sample already uploaded with audsrv_load_adpcm()
 * @param ch    channel identifier. Specifies one of the 24 voice channel to play the ADPCM channel on. If set to an invalid channel ID, an unoccupied channel will be selected. 
 * @param id    sample identifier, as specified in load()
 * @returns zero on success, negative value on error
 *
 * When ch is set to an invalid channel ID, the sample will be played in an unoccupied channel.
 * If all 24 channels are used, then -AUDSRV_ERR_NO_MORE_CHANNELS is returned.
 * When ch is set to a valid channel ID, -AUDSRV_ERR_NO_MORE_CHANNELS is returned if the channel is currently in use.
 * Trying to play a sample which is unavailable will result in -AUDSRV_ERR_ARGS
 */
int audsrv_ch_play_adpcm(int ch, u32 id)
{
	int channel;
	u32 endx;
	adpcm_list_t *a;

	a = adpcm_loaded(id);
	if (a == NULL)
	{
		/* bad joke */
		return AUDSRV_ERR_ARGS;
	}

	/* sample was loaded */
	if (ch >= 0 && ch < 24)
	{
		endx = sceSdGetSwitch(SD_CORE_1 | SD_SWITCH_ENDX);
		if (!(endx & (1 << ch)))
		{
			/* Channel in use. */
			return -AUDSRV_ERR_NO_MORE_CHANNELS;
		}

		channel = ch;
	}
	else
	{
		channel = audsrv_adpcm_alloc_channel();
		if (channel < 0)
			return channel;
	}

	sceSdSetParam(SD_CORE_1 | (channel << 1) | SD_VPARAM_PITCH, a->pitch);
	sceSdSetAddr(SD_CORE_1 | (channel << 1) | SD_VOICE_START, a->spu2_addr);
	sceSdSetSwitch(SD_CORE_1 | SD_SWITCH_KON, (1 << channel));
	return AUDSRV_ERR_NOERROR;
}

/** Initializes adpcm unit of audsrv
 *
 * @returns zero on success, negative value on error
 */
int audsrv_adpcm_init()
{
	int voice;

	printf("audsrv_adpcm_init()\n");

	for (voice = 0; voice < 24; voice++)
	{
		sceSdSetSwitch(SD_CORE_1 | SD_SWITCH_KOFF, (1 << voice));
		sceSdSetParam(SD_CORE_1 | (voice << 1) | SD_VPARAM_VOLL, 0x3fff);
		sceSdSetParam(SD_CORE_1 | (voice << 1) | SD_VPARAM_VOLR, 0x3fff);
	}

	if (adpcm_list_head != NULL)
	{
		/* called second time, after samples were already uploaded */
		free_all_samples();
	}

	return AUDSRV_ERR_NOERROR;
}

/** Sets output volume for the specified voice channel.
 * @param ch       Voice channel ID
 * @param vol      volume in SPU2 units [0 .. 0x3fff]
 * @returns 0 on success, negative otherwise
 */
int audsrv_adpcm_set_volume(int ch, int vol)
{
	if (vol < 0 || vol > MAX_VOLUME)
	{
		/* bad joke */
		return AUDSRV_ERR_ARGS;
	}

	sceSdSetParam(SD_CORE_1 | (ch << 1) | SD_VPARAM_VOLL, vol);
	sceSdSetParam(SD_CORE_1 | (ch << 1) | SD_VPARAM_VOLR, vol);

	return AUDSRV_ERR_NOERROR;
}

