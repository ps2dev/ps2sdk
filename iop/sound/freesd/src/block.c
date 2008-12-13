/*
 * block.c - Part of the IOP Sound Driver
 *
 * Copyright (c) 2004 TyRaNiD <tiraniddo@hotmail.com>
 * Copyright (c) 2004,2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include "types.h"
#include "freesd.h"
#include "spu2regs.h"

u32 BlockTransBuff[2]; 
u32 BlockTransAddr[2];
u32 BlockTransSize[2];

extern void SetDmaWrite(s32 chan);
extern void SetDmaRead(s32 chan);
extern IntrData TransIntrData[2];
extern u32 DmaStop(u32 core);


s32 BlockTransWriteFrom(u8 *iopaddr, u32 size, s32 chan, u16 mode, u8 *startaddr)
{
	s32 core = chan & 1;
	s32 offset;

	BlockTransBuff[core] = 0;
	BlockTransSize[core] = size;
	BlockTransAddr[core] = (u32)iopaddr;

	if(startaddr == 0)
		startaddr = iopaddr;

	offset = startaddr - iopaddr;

	if(offset > size)
	{
		if(mode & SD_BLOCK_TRANS_LOOP)
		{
			offset -= size;
			BlockTransBuff[core] = 1;
		}
		else
		{
			return -1;
		}
	}
	
	if(offset & 1023) offset += 1024;

	iopaddr += (BlockTransSize[core] * BlockTransBuff[core]) + offset;

	if(*SD_CORE_ATTR(core) & SD_DMA_IN_PROCESS) return -1;
	if(*SD_DMA_CHCR(core) & SD_DMA_START) return -1;

	*SD_CORE_ATTR(core) &= 0xFFCF;
	
	*SD_A_TSA_HI(core) = 0;
	*SD_A_TSA_LO(core) = 0;
	
	*U16_REGISTER(0x1B0+(core*1024)) = 1 << core;

	SetDmaWrite(core);
	
	*SD_DMA_ADDR(core) = (u32)iopaddr;
	*SD_DMA_MODE(core) = 0x10;
	*SD_DMA_SIZE(core) = (size/64)+((size&63)>0); 
	*SD_DMA_CHCR(core) = SD_DMA_CS | SD_DMA_START | SD_DMA_DIR_IOP2SPU;

	return 0;
}

s32 BlockTransWrite(u8 *iopaddr, u32 size, s32 chan)
{
	s32 core = chan & 1;
	
	BlockTransBuff[core] = 0;
	BlockTransSize[core] = size;
	BlockTransAddr[core] = (u32)iopaddr;
	
	if(*SD_CORE_ATTR(core) & SD_DMA_IN_PROCESS) return -1;
	if(*SD_DMA_CHCR(core) & SD_DMA_START) return -1;

	*SD_CORE_ATTR(core) &= 0xFFCF;
	
	*SD_A_TSA_HI(core) = 0;
	*SD_A_TSA_LO(core) = 0;
	
	*U16_REGISTER(0x1B0+(core*1024)) = 1 << core;

	SetDmaWrite(core);
	
	*SD_DMA_ADDR(core) = (u32)iopaddr;
	*SD_DMA_MODE(core) = 0x10;
	*SD_DMA_SIZE(core) = (size/64)+((size&63)>0); 
	*SD_DMA_CHCR(core) = SD_DMA_CS | SD_DMA_START | SD_DMA_DIR_IOP2SPU;
	
	return 0;
}


s32 BlockTransRead(u8 *iopaddr, u32 size, s32 chan, s16 mode)
{
	u32 i;

	s32 core = chan & 1;
	
	BlockTransBuff[core] = 0;
	BlockTransSize[core] = size;
	BlockTransAddr[core] = (u32)iopaddr;
	
	if(*SD_CORE_ATTR(core) & SD_DMA_IN_PROCESS) return -1;
	if(*SD_DMA_CHCR(core) & SD_DMA_START) return -1;

	*SD_CORE_ATTR(core) &= 0xFFCF;
	
	*SD_A_TSA_HI(core) = 0;
	*SD_A_TSA_LO(core) = ((mode & 0xF00) << 1) + 0x400;
	
	*U16_REGISTER(0x1AE + (core*1024)) = (mode & 0xF000) >> 11;

	i = 0x4937;

	while(i--);

	*U16_REGISTER(0x1B0+(core*1024)) = 4;

	SetDmaRead(core);
	
	*SD_DMA_ADDR(core) = (u32)iopaddr;
	*SD_DMA_MODE(core) = 0x10;
	*SD_DMA_SIZE(core) = (size/64)+((size&63)>0); 
	*SD_DMA_CHCR(core) = SD_DMA_CS | SD_DMA_START | SD_DMA_DIR_SPU2IOP;


	return 0;
}


s32 SdBlockTrans(s16 chan, u16 mode, u8 *iopaddr, u32 size, u8 *startaddr)
{
	int transfer_dir = mode & 3;
	int core = chan & 1;
	int _size = size;

	switch(transfer_dir)
	{
		case SD_BLOCK_TRANS_WRITE:
		{
			TransIntrData[core].mode = 0x100 | core;

			if(mode & SD_BLOCK_TRANS_LOOP)
			{
				TransIntrData[core].mode |= SD_BLOCK_TRANS_LOOP << 8;
				_size /= 2;
			}

			if(BlockTransWrite(iopaddr, _size, core) >= 0)
				return 0;
		
		} break;

		case SD_BLOCK_TRANS_READ:
		{
			TransIntrData[core].mode = 0x300 | core;

			if(mode & SD_BLOCK_TRANS_LOOP)
			{
				TransIntrData[core].mode |= SD_BLOCK_TRANS_LOOP << 8;
				_size /= 2;
			}

			if(BlockTransRead(iopaddr, _size, chan, mode) >= 0)
				return 0;
		
		} break;

		case SD_BLOCK_TRANS_STOP:
		{
			return DmaStop(core);

		} break;

		case SD_BLOCK_TRANS_WRITE_FROM:
		{
			TransIntrData[core].mode = 0x100 | core;
			
			if(mode & SD_BLOCK_TRANS_LOOP)
			{
				TransIntrData[core].mode |= SD_BLOCK_TRANS_LOOP << 8;
				_size /= 2;
			}

			if(BlockTransWriteFrom(iopaddr, _size, core, mode, startaddr) >= 0)
				return 0;


		} break;

	}
	return -1;
}

u32 SdBlockTransStatus(s16 chan, s16 flag)
{
	u32 retval;

	chan &= 1;

	if(*U16_REGISTER(0x1B0 + (chan * 1024)) == 0)
		retval = 0;
	else
		retval = *SD_DMA_ADDR(chan);

	retval = (BlockTransBuff[chan] << 24) | (retval & 0xFFFFFF);
	
	return retval;
}
