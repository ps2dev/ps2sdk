/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
#
# $Id$
# audsrv IOP server
*/

#include <stdio.h>
#include <thbase.h>
#include <thsemap.h>
#include <loadcore.h>
#include <sysmem.h>
#include <intrman.h>
#include <sifcmd.h>
#include <libsd.h>
#include <sysclib.h>

#include <audsrv.h>
#include "audsrv.c.h"
#include "common.h"
#include "rpc_server.h"
#include "spu.h"

#define NEW(x)		 AllocSysMemory(0, sizeof(x), 0);

typedef struct adpcm_list_t
{
	int id;
	int pitch;
	int loop;
	int channels;
	int size;
	int spu2_addr;
	struct adpcm_list_t* next;
} adpcm_list_t;

adpcm_list_t *adpcm_list_head = 0;
adpcm_list_t *adpcm_list_tail = 0;

u32 sbuffer[16]	__attribute__((aligned(64)));

adpcm_list_t* adpcm_loaded(int id)
{
	adpcm_list_t *cur = adpcm_list_head;

	while(cur)
	{
		if(cur->id == id) return cur;

		cur = cur->next;
	}
	return 0;
}

void audsrv_read_adpcm_header(adpcm_list_t* adpcm, u32* buffer)
{
	adpcm->pitch = buffer[2];
	adpcm->loop = (buffer[1] >> 16) & 0xFF;
	adpcm->channels = (buffer[1] >> 8) & 0xFF;
}

/* Load adpcm sample from iop memory to spu2 memory */
void* audsrv_load_adpcm(u32* buffer, int size, int id)
{
	adpcm_list_t *adpcm;

	if((adpcm = adpcm_loaded(id)) == 0)
	{
		if(adpcm_list_head == 0)
		{
			adpcm = NEW(adpcm_list_t);
			adpcm->id = id;
			adpcm->spu2_addr = 0x5010; /* Need to change this so it considers to PCM streaming space usage :) */
			adpcm->size = size - 16; /* header is 16 bytes */
				
			audsrv_read_adpcm_header(adpcm, buffer);

			adpcm_list_head = adpcm;
			adpcm_list_head->next = 0;
			adpcm_list_tail = adpcm_list_head;
		}
		else
		{
			adpcm = NEW(adpcm_list_t);
			adpcm->id = id;
			adpcm->spu2_addr = adpcm_list_tail->spu2_addr+adpcm_list_tail->size;
			adpcm->size = size - 16; /* header is 16 bytes */

			audsrv_read_adpcm_header(adpcm, buffer);

			adpcm_list_tail->next = adpcm;
			adpcm_list_tail = adpcm;			

		}	
		
		/* DMA from IOP to SPU2 */
		sceSdVoiceTrans(0, SD_VOICE_TRANS_WRITE | SD_VOICE_TRANS_MODE_DMA, ((u8*)buffer)+16, (u8*)adpcm->spu2_addr, adpcm->size);
		sceSdVoiceTransStatus(0, 1);
	}

	sbuffer[0] = 0;
	sbuffer[1] = adpcm->pitch;
	sbuffer[2] = adpcm->loop;
	sbuffer[3] = adpcm->channels;
	return sbuffer;
}

/* Play adpcm sample already in spu2 memory */
void* audsrv_play_adpcm(u32 id)
{
	adpcm_list_t* a = adpcm_list_head;

	while(a)
	{	
		// sample is in spu2 memory
		if(a->id == id)
		{
			s32 channel = -1;
			u32 endx = sceSdGetSwitch(SD_CORE_1 | SD_S_ENDX);
			u32 i = 1;

			// all channels are occupied
			if(endx == 0) 
			{	
				sbuffer[0] = -1;
				return sbuffer;
			}
			// Find first available
			while((channel < 0) && (i < 24))
			{
				if(endx & (1 << i))
				{
					channel = i;
				}
				i++;
			}

			if(channel != -1)
			{
				sceSdSetParam(SD_CORE_1  | (channel << 1) | SD_VOICE_PITCH ,  a->pitch );
				sceSdSetAddr(SD_CORE_1   | (channel << 1) | SD_VOICE_START, a->spu2_addr );
				sceSdSetSwitch(SD_CORE_1 | SD_VOICE_KEYON, (1 << channel));
				sbuffer[0] = 0;
				return sbuffer;
			}
		}
		a = a->next;
	}
	sbuffer[0] = 1;
	return sbuffer;
}


void audsrv_adpcm_init()
{
	u32 voice;

	sceSdInit(0);

	sceSdSetParam(SD_CORE_1 | SD_P_MVOLL ,0x3fff);
	sceSdSetParam(SD_CORE_1 | SD_P_MVOLR ,0x3fff);

	for(voice = 1; voice < 24; voice++)
	{
		sceSdSetParam(SD_CORE_1 | (voice << 1) | SD_VOICE_VOLL , 0x3fff );
		sceSdSetParam(SD_CORE_1 | (voice << 1) | SD_VOICE_VOLR , 0x3fff );
	}
}


