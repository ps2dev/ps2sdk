/*
 * voice.c - Part of the IOP Sound Driver
 *
 * Copyright (c) 2004 TyRaNiD <tiraniddo@hotmail.com>
 * Copyright (c) 2004 Lukasz Bruun <ps2@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include "types.h"
#include "irx.h"
#include "sifman.h"
#include "freesd.h"
#include "spu2regs.h"


volatile u16 VoiceTransComplete[2];
u32 VoiceTransStatus[2];
u32 VoiceTransIoMode[2];

extern void SetDmaWrite(s32 chan);
extern void SetDmaRead(s32 chan);
extern IntrData TransIntrData[2];
extern void DmaStop(u32 core);

s32 VoiceTransDma(s16 chan, u16 mode, u8 *iop_addr, u32 *spu_addr, u32 size)
{
	u32 direction;

	if(*SD_CORE_ATTR(chan) & SD_DMA_IN_PROCESS)
		return -1;

	if(*SD_DMA_CHCR(chan) & SD_DMA_START)
		return -1;		

	*SD_A_TSA_HI(chan) = (u32)spu_addr >> 17;
	*SD_A_TSA_LO(chan) = (u32)spu_addr >> 1;
	
	if(mode == SD_VOICE_TRANS_WRITE)
	{
		TransIntrData[chan].mode = chan;
		direction = SD_DMA_DIR_IOP2SPU;
		*SD_CORE_ATTR(chan) = (*SD_CORE_ATTR(chan) & ~SD_CORE_DMA) | SD_DMA_WRITE;
		SetDmaWrite(chan);
	}
	else
	{
		if(mode == SD_VOICE_TRANS_READ)
		{
			TransIntrData[chan].mode = (chan << 2) | 0x200;
			direction = SD_DMA_DIR_SPU2IOP;
			*SD_CORE_ATTR(chan) = (*SD_CORE_ATTR(chan) & ~SD_CORE_DMA) | SD_DMA_READ;
			SetDmaRead(chan);
		}
		else
		{
			return -1;
		}
	}

	*SD_DMA_ADDR(chan)	= (u32)iop_addr;
	*SD_DMA_MSIZE(chan) = ((size/64) << 16) | 0x10; 
	*SD_DMA_CHCR(chan)	= SD_DMA_CS | SD_DMA_START | direction;

	return size;
}

s32 VoiceTrans_Write_IOMode(u32 iopaddr, u32 spu_addr, s32 size, s16 chan)
{
	if(*SD_CORE_ATTR(chan) & SD_DMA_IN_PROCESS) 
		return -1;
	
	if(*SD_DMA_CHCR(chan) & SD_DMA_START) 
		return -1;

	*SD_A_TSA_HI(chan) = (u16)spu_addr >> 17;
	*SD_A_TSA_LO(chan) = (u16)spu_addr >> 1;

	TransIntrData[chan].mode = chan;

	if(size)
	{
		s32 loop, count;
		volatile u16* iop_mem = (volatile u16 *) iopaddr;

		while(size > 0)
		{
			if(size > 64)
				count = 64;
			else
				count = size;
		
			if(count > 0)
			{
				for(loop = 0; loop < count; loop += 2)
					*SD_A_STD(chan) = *iop_mem++;
			}

			// Set Transfer mode to IO
			*SD_CORE_ATTR(0) = (*SD_CORE_ATTR(0) & ~SD_CORE_DMA) | SD_DMA_IO;

			// Wait for transfer to complete;
			while(*SD_C_STATX(0) & SD_IO_IN_PROCESS);

			size -= count;
		}
	}

	// Reset DMA settings
	*SD_CORE_ATTR(0) &= ~SD_CORE_DMA;

	return 0;
}

s32 SdVoiceTrans(s16 chan, u16 mode, u8 *iopaddr, u32 *spuaddr, u32 size)
{	
	s32 res = size;

	chan &= 1;

	if(mode & SD_VOICE_TRANS_MODE_IO)
		VoiceTransIoMode[chan] = 1;
	else
		VoiceTransIoMode[chan] = 0;

	if(VoiceTransIoMode[chan] == 0)
	{
		res = VoiceTransDma(chan, mode & 3, iopaddr, spuaddr, size);
	}
	else
	{	
		if((mode & 3) == SD_VOICE_TRANS_WRITE)
		{
			res = VoiceTrans_Write_IOMode( (u32)iopaddr, (u32)spuaddr, size, chan);
		}
	}

	VoiceTransStatus[chan] = 0;

	return res;
}

u32 SdVoiceTransStatus(s16 chan, s16 flag)
{
	chan &= 1;
	flag &= 1;
	
	if((VoiceTransIoMode[chan] == 1) || (VoiceTransStatus[chan] == 1))
		return 1;
	
	if(flag) 
	{
		while(!VoiceTransComplete[chan]);
		VoiceTransStatus[chan] = 1;
		VoiceTransComplete[chan] = 0;
		
		return 1;
	}
	else
	{
		if(VoiceTransComplete[chan] == 0) return 0;
		VoiceTransStatus[chan] = 1;
		VoiceTransComplete[chan] = 0;
	}

	return 0;
}
