/*
 * Copyright (c) 2004 TyRaNiD <tiraniddo@hotmail.com>
 * Copyright (c) 2004,2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

/**
 * @file
 * Part of the IOP Sound Driver
 */

#include "types.h"
#include "sifman.h"
#include "intrman.h"
#include "libsd.h"
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

int sceSdProcBatch(sceSdBatch *batch, u32 *rets, u32 num)
{
	s32 loop;
	s32 ret;

	for(loop = 0; loop < num; loop++)
	{
		ret = 0;

		switch(batch[loop].func)
		{
		case SD_BATCH_SETPARAM:
			sceSdSetParam(batch[loop].entry, batch[loop].value);
			break;
		case SD_BATCH_GETPARAM:
			ret = sceSdGetParam(batch[loop].entry);
			break;
		case SD_BATCH_SETSWITCH:
			sceSdSetSwitch(batch[loop].entry, batch[loop].value);
			break;
		case SD_BATCH_GETSWITCH:
			ret = sceSdGetSwitch(batch[loop].entry);
			break;
		case SD_BATCH_SETADDR:
			sceSdSetAddr(batch[loop].entry, batch[loop].value);
			break;
		case SD_BATCH_GETADDR:
			ret = sceSdGetAddr(batch[loop].entry);
			break;
		case SD_BATCH_SETCORE:
			sceSdSetCoreAttr(batch[loop].entry, batch[loop].value);
			break;
		case SD_BATCH_GETCORE:
			ret = sceSdGetCoreAttr(batch[loop].entry);
			break;
		case SD_BATCH_WRITEIOP:
			*((u32 *) batch[loop].value) = batch[loop].entry;
			break;
		case SD_BATCH_WRITEEE:
			BatchData = batch[loop].entry;
			SifDmaBatch(batch[loop].value, &BatchData, 4);
			break;
		case SD_BATCH_EERETURN:
			SifDmaBatch(batch[loop].value, rets, batch[loop].entry);
			break;
		default:
			return -1 - loop;
		}

		if(rets) rets[loop] = ret;
	}

	return loop;
}

int sceSdProcBatchEx(sceSdBatch *batch, u32 *rets, u32 num, u32 voice)
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
			case SD_BATCH_SETPARAM:
			{
				if((batch[loop].entry & 0x3E) != 0x3E)
				{
					sceSdSetParam(batch[loop].entry, batch[loop].value);
				}
				else
				{
					for(voice_loop = 0; voice_loop < 24; voice_loop++)
					{
						if(voice & (1 << voice_loop))
						{
							sceSdSetParam((batch[loop].entry & 0xFFC1) | (1 << (voice_loop + 1)), batch[loop].value);
							cmd_count++;
						}
					}
					cmd_count--;
				}
			} break;

			case SD_BATCH_GETPARAM:
			{
				if((batch[loop].entry & 0x3E) != 0x3E)
				{
					ret = sceSdGetParam(batch[loop].entry);
				}
				else
				{
					for(voice_loop = 0; voice_loop < 24; voice_loop++)
					{
						if(voice & (1 << voice_loop))
						{
							ret = sceSdGetParam((batch[loop].entry & 0xFFC1) | (1 << (voice_loop + 1)));
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

			case SD_BATCH_SETSWITCH:
				sceSdSetSwitch(batch[loop].entry, batch[loop].value);
			break;

			case SD_BATCH_GETSWITCH:
				ret = sceSdGetSwitch(batch[loop].entry);
			break;

			case SD_BATCH_SETADDR:
			{
				if((batch[loop].entry & 0x3E) != 0x3E)
				{
					sceSdSetAddr(batch[loop].entry, batch[loop].value);
				}
				else
				{
					for(voice_loop = 0; voice_loop < 24; voice_loop++)
					{
						if(voice & (1 << voice_loop))
						{
							sceSdSetAddr((batch[loop].entry & 0xFFC1) | (1 << (voice_loop + 1)), batch[loop].value);
							cmd_count++;
						}
					}
				}
				cmd_count--;
			} break;

			case SD_BATCH_GETADDR:
			{
				if((batch[loop].entry & 0x3E) != 0x3E)
				{
					ret = sceSdGetAddr(batch[loop].entry);
				}
				else
				{
					for(voice_loop = 0; voice_loop < 24; voice_loop++)
					{
						if(voice & (1 << voice_loop))
						{
						ret = sceSdGetAddr((batch[loop].entry & 0xFFC1) | (1 << (voice_loop + 1)));
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

			case SD_BATCH_SETCORE:
				sceSdSetCoreAttr(batch[loop].entry, batch[loop].value);
				break;

			case SD_BATCH_GETCORE:
				ret = sceSdGetCoreAttr(batch[loop].entry);
				break;

			case SD_BATCH_WRITEIOP:
				*((u32 *) batch[loop].value) = batch[loop].entry;
				break;

			case SD_BATCH_WRITEEE:
				BatchData = batch[loop].entry;
				SifDmaBatch(batch[loop].value, &BatchData, 4);
				break;

			case SD_BATCH_EERETURN:
				SifDmaBatch(batch[loop].value, rets, batch[loop].entry);
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
