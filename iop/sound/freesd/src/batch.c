/*
 * batch.c - Part of the IOP Sound Driver
 *
 * Copyright (c) 2004 TyRaNiD <tiraniddo@hotmail.com>
 * Copyright (c) 2004 Lukasz Bruun <ps2@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include "types.h"
#include "sifman.h"
#include "intrman.h"
#include "freesd.h"
#include "spu2regs.h"

u32 BatchData __attribute__((aligned(16)));

s32 SifDmaBatch(u32 ee_addr, void *iop_addr, u32 size)
{
	SifDmaTransfer_t dma;
	s32 intr_stat;
	s32 dma_id;

	dma.src = iop_addr;
	dma.dest = (void *)ee_addr;
	dma.size = size;
	dma.attr = 0;

	CpuSuspendIntr((int *)&intr_stat);
	dma_id = SifSetDma(&dma, SIF_DMA_TO_EE);
	CpuResumeIntr(intr_stat);
	
	while(SifDmaStat(dma_id) >= 0);

	if(dma_id == 0) return -1;

	return 0;
}

s32 SdProcBatch(SdBatch *batch, u32 *rets, u32 num)
{
	s32 loop;
	s32 ret;
	
	for(loop = 0; loop < num; loop++)
	{
		ret = 0;
		
		switch(batch[loop].func)
		{
		case BATCH_SETPARAM: 
			SdSetParam(batch[loop].entry, batch[loop].val);
			break;
		case BATCH_GETPARAM:
			ret = SdGetParam(batch[loop].entry);
			break;
		case BATCH_SETSWITCH:
			SdSetSwitch(batch[loop].entry, batch[loop].val);
			break;
		case BATCH_GETSWITCH:
			ret = SdGetSwitch(batch[loop].entry);
			break;
		case BATCH_SETADDR:
			SdSetAddr(batch[loop].entry, batch[loop].val);
			break;
		case BATCH_GETADDR:
			ret = SdGetAddr(batch[loop].entry);
			break;
		case BATCH_SETCORE:
			SdSetCoreAttr(batch[loop].entry, batch[loop].val);
			break;
		case BATCH_GETCORE:
			ret = SdGetCoreAttr(batch[loop].entry);
			break;
		case BATCH_WRITEIOP:
			*((u32 *) batch[loop].val) = batch[loop].entry;
			break;
		case BATCH_WRITEEE:
			BatchData = batch[loop].entry;
			SifDmaBatch(batch[loop].val, &BatchData, 4);
			break;
		case BATCH_EERETURN:
			SifDmaBatch(batch[loop].val, rets, batch[loop].entry);
			break;
		default:
			return -1 - loop;
		}

		if(rets) rets[loop] = ret;
	}

	return loop;
}

s32 SdProcBatchEx(SdBatch *batch, u32 *rets, u32 num, u32 voice)
{
	s32 loop;
	s32 ret;
	s32 voice_loop;
	s32 cmd_count;
	
	cmd_count = 0;
	
	for(loop = 0; loop < num; loop++)
	{
		ret = 0;
		switch(batch[loop].func)
		{
			case BATCH_SETPARAM:
			{
				if((batch[loop].entry & 0x3E) != 0x3E)
				{
					SdSetParam(batch[loop].entry, batch[loop].val);
				}
				else
				{
					for(voice_loop = 0; voice_loop < 24; voice_loop++)
					{
						if(voice & (1 << voice_loop))
						{
							SdSetParam((batch[loop].entry & 0xFFC1) | (1 << (voice_loop + 1)), batch[loop].val);
							cmd_count++;
						}
					}
					cmd_count--;
				}
			} break;
		
			case BATCH_GETPARAM:
			{
				if((batch[loop].entry & 0x3E) != 0x3E)
				{
					ret = SdGetParam(batch[loop].entry);
				}
				else
				{
					for(voice_loop = 0; voice_loop < 24; voice_loop++)
					{
						if(voice & (1 << voice_loop))
						{
							ret = SdGetParam((batch[loop].entry & 0xFFC1) | (1 << (voice_loop + 1)));
							cmd_count++;
						}

						if(rets)
						{
							rets[cmd_count] = ret;
						}
					}

					cmd_count--;
				}
			} break;

			case BATCH_SETSWITCH:
				SdSetSwitch(batch[loop].entry, batch[loop].val);
			break;
		
			case BATCH_GETSWITCH:
				ret = SdGetSwitch(batch[loop].entry);
			break;
		
			case BATCH_SETADDR:
			{
				if((batch[loop].entry & 0x3E) != 0x3E)
				{
					SdSetAddr(batch[loop].entry, batch[loop].val);
				}
				else
				{
					for(voice_loop = 0; voice_loop < 24; voice_loop++)
					{
						if(voice & (1 << voice_loop))
						{
							SdSetAddr((batch[loop].entry & 0xFFC1) | (1 << (voice_loop + 1)), batch[loop].val);
							cmd_count++;
						}
					}
				}
				cmd_count--;
			} break;

			case BATCH_GETADDR:
			{
				if((batch[loop].entry & 0x3E) != 0x3E)
				{
					ret = SdGetAddr(batch[loop].entry);
				}
				else
				{
					for(voice_loop = 0; voice_loop < 24; voice_loop++)
					{
						if(voice & (1 << voice_loop))
						{
						ret = SdGetAddr((batch[loop].entry & 0xFFC1) | (1 << (voice_loop + 1)));
						cmd_count++;
						}
				
						if(rets)
						{
						rets[cmd_count] = ret;
						}
					}
				}
				cmd_count--;
			} break;
		
			case BATCH_SETCORE:
				SdSetCoreAttr(batch[loop].entry, batch[loop].val);
				break;
		
			case BATCH_GETCORE:
				ret = SdGetCoreAttr(batch[loop].entry);
				break;
			
			case BATCH_WRITEIOP:
				*((u32 *) batch[loop].val) = batch[loop].entry;
				break;
		
			case BATCH_WRITEEE:
				BatchData = batch[loop].entry;
				SifDmaBatch(batch[loop].val, &BatchData, 4);
				break;
		
			case BATCH_EERETURN:
				SifDmaBatch(batch[loop].val, rets, batch[loop].entry);
				break;
			default:
				return -1 - cmd_count;
		}

		if(rets)
		{
			rets[cmd_count] = ret;
		}
		cmd_count++;
	}

	return cmd_count;
}
