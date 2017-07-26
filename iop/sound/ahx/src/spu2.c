/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Based on freesd source
*/

#ifndef LIBSD

#include "types.h"
#include <stdarg.h>
#include "spu2regs.h"
#include "freesd.h"
#include "thbase.h"
#include "intrman.h"
#include "loadcore.h"

// block
u32 BlockTransBuff[2];
u32 BlockTransAddr[2];
u32 BlockTransSize[2];

// voice
#ifndef ISJPCM
u32 VoiceTransStatus[2];
volatile u16 VoiceTransComplete[2];
u32 VoiceTransIoMode[2];
#endif
// Global

sceSdTransIntrHandler	TransIntrHandlers[2];
SdIntrCallback		TransIntrCallbacks[2];
#ifndef ISJPCM
u16 SpdifSettings;
void *Spu2IntrData;
sceSdSpu2IntrHandler	Spu2IntrHandler;
SdIntrCallback		Spu2IrqCallback;
#endif
IntrData			TransIntrData[2];

#ifndef ISJPCM
volatile u16 *ParamRegList[] =
{
	SD_VP_VOLL(0, 0),	SD_VP_VOLR(0, 0),	SD_VP_PITCH(0, 0),	SD_VP_ADSR1(0, 0),
	SD_VP_ADSR2(0, 0),	SD_VP_ENVX(0, 0),	SD_VP_VOLXL(0, 0),	SD_VP_VOLXR(0, 0),
	SD_P_MMIX(0),		SD_P_MVOLL(0),		SD_P_MVOLR(0),		SD_P_EVOLL(0),
	SD_P_EVOLR(0),		SD_P_AVOLL(0),		SD_P_AVOLR(0),		SD_P_BVOLL(0),
	SD_P_BVOLR(0),		SD_P_MVOLXL(0),		SD_P_MVOLXR(0),		SD_S_PMON_HI(0),
	SD_S_NON_HI(0),		SD_A_KON_HI(0),		SD_A_KON_HI(0),		SD_S_ENDX_HI(0),
	SD_S_VMIXL_HI(0),	SD_S_VMIXEL_HI(0),	SD_S_VMIXR_HI(0),	SD_S_VMIXER_HI(0),
	SD_A_ESA_HI(0),		SD_A_EEA_HI(0),		SD_A_TSA_HI(0),		SD_CORE_IRQA(0),
	SD_VA_SSA_HI(0, 0),	SD_VA_LSAX(0,0),	SD_VA_NAX(0, 0),	SD_CORE_ATTR(0),
	SD_A_TSA_HI(0),		SD_A_STD(0),
	// 1AE & 1B0 are both related to core attr & dma somehow
	U16_REGISTER(0x1AE),	U16_REGISTER(0x1B0),
	(u16*)0xBF900334

};
#endif

int TransInterrupt(void *data)
{
	IntrData *intr = (IntrData*) data;
	s32 dir;
	s32 core;

	dir = ((intr->mode & 0x200) < 1);
	core = intr->mode & 0xFF;

	// Voice Transfer
	if((intr->mode & 0x100) == 0)
	{
		#ifndef ISJPCM
		// SD_C_STATX(core)
		// If done elsewise, it doesn't work, havn't figured out why yet.
		volatile u16 *statx = U16_REGISTER(0x344 + (core * 1024));

		if(!(*statx & 0x80))	while(!(*statx & 0x80));

		*SD_CORE_ATTR(core) &= ~SD_CORE_DMA;

		if(*SD_CORE_ATTR(core) & 0x30)	while((*SD_CORE_ATTR(core) & 0x30));

		if(TransIntrHandlers[core])		goto intr_handler;
		if(TransIntrCallbacks[core])	goto SdIntrCallback;

		VoiceTransComplete[core] = 1;
		#endif
	}
	else
	{	// Block Transfer
		if(intr->mode & (SD_TRANS_LOOP << 8))
		{
			// Switch buffers
			BlockTransBuff[core] = 1 - BlockTransBuff[core];
			// Setup DMA & send
			*SD_DMA_ADDR(core) = (BlockTransSize[core] * BlockTransBuff[core])+BlockTransAddr[core];
			*SD_DMA_SIZE(core) = (BlockTransSize[core]/64)+((BlockTransSize[core]&63)>0);
			*SD_DMA_CHCR(core) = SD_DMA_START | SD_DMA_CS | dir;
		}
		else
		{
			#ifndef ISJPCM
			*SD_CORE_ATTR(core) &= ~SD_CORE_DMA;
			*SD_P_MMIX(core)	&= 0xFF3F;
			*U16_REGISTER(0x1B0 + (core * 1024)) = 0;
			#endif
		}

		if(TransIntrHandlers[core])
		{
			#ifndef ISJPCM
			intr_handler:
			#endif
			TransIntrHandlers[core](core, intr->data);
		}
		else
		{
			if(TransIntrCallbacks[core])
			{
				#ifndef ISJPCM
				SdIntrCallback:
				#endif
				TransIntrCallbacks[core](0);
			}
		}
	}

	if(dir == SD_DMA_DIR_SPU2IOP)	FlushDcache();

	return 1;
}

#ifndef ISJPCM
int Spu2Interrupt(void *data)

{
	volatile u16 *reg1 = U16_REGISTER(0x7C2);

	if(Spu2IntrHandler != NULL)
	{
		Spu2IntrHandler((*reg1 & 0xC) >> 2, Spu2IntrData);
	}
	else
	{
		if(Spu2IrqCallback) Spu2IrqCallback(0);
	}

	return 1;
}
#endif

void nopdelay()
{
	s32 i;

	for(i=0; i < 0x10000; i++)
		asm volatile("nop\nnop\nnop\nnop\nnop");
}

void InitSpu2()
{

	*U32_REGISTER(0x1404)  = 0xBF900000;
	*U32_REGISTER(0x140C)  = 0xBF900800;
	*U32_REGISTER(0x10F0) |= 0x80000;
	*U32_REGISTER(0x1570) |= 8;
	*U32_REGISTER(0x1014)  = 0x200B31E1;
	*U32_REGISTER(0x1414)  = 0x200B31E1;
}


void RegisterInterrupts()
{
	s32 ret;

	DisableIntr(0x24, (int *)&ret);
	DisableIntr(0x28, (int *)&ret);
	#ifndef ISJPCM
	DisableIntr(0x9, (int *)&ret);
	#endif

	ReleaseIntrHandler(0x24);
	ReleaseIntrHandler(0x28);

	RegisterIntrHandler(0x24, 1, TransInterrupt, &TransIntrData[0]);
	RegisterIntrHandler(0x28, 1, TransInterrupt, &TransIntrData[1]);

	#ifndef ISJPCM
	VoiceTransComplete[0] = 0;
	VoiceTransComplete[1] = 0;

	ReleaseIntrHandler(0x9);
	RegisterIntrHandler(0x9, 1, Spu2Interrupt, &Spu2IntrData);
	#endif
}

void ResetAll()
{
	u32 core;
	volatile u16 *statx;

	*SD_C_SPDIF_OUT = 0;
	nopdelay();
	*SD_C_SPDIF_OUT = 0x8000;
	nopdelay();

	*U32_REGISTER(0x10F0) |= 0xB0000;

	for(core=0; core < 2; core++)
	{
		#ifndef ISJPCM
		VoiceTransIoMode[core]	= 0;
		#endif
		*U16_REGISTER(0x1B0)	= 0;
		*SD_CORE_ATTR(core)		= 0;
		nopdelay();
		*SD_CORE_ATTR(core)		= SD_SPU2_ON;

		*SD_P_MVOLL(core)		= 0;
		*SD_P_MVOLR(core)		= 0;

		statx = U16_REGISTER(0x344 + (core * 1024));

		while(*statx & 0x7FF);

		*SD_A_KOFF_HI(core)		= 0xFFFF;
		*SD_A_KOFF_LO(core)		= 0xFFFF; // Should probably only be 0xFF
	}

	*SD_S_PMON_HI(1)	= 0;
	*SD_S_PMON_LO(1)	= 0;
	*SD_S_NON_HI(1)		= 0;
	*SD_S_NON_LO(1)		= 0;
}


u16 VoiceDataInit[16] = { 0x707, 0x707, 0x707, 0x707, 0x707, 0x707, 0x707, 0x707,
						0,     0,     0,     0,     0,     0,     0,     0 };

void InitVoices()
{
	s32 voice, i;
	volatile u16 *statx;

	// Set Start Address of data to transfer.
	*SD_A_TSA_HI(0) = 0;
	*SD_A_TSA_LO(0) = 0x5000 >> 1;

	// Fill with data.
	// First 16 bytes are reserved.
	for(i = 0; i < 16; i++) *SD_A_STD(0) = VoiceDataInit[i];

	// Set Transfer mode to IO
	*SD_CORE_ATTR(0) = (*SD_CORE_ATTR(0) & ~SD_CORE_DMA) | SD_DMA_IO;

	statx = U16_REGISTER(0x344);

	// Wait for transfer to complete;
	while(*statx & SD_IO_IN_PROCESS);

	// Reset DMA settings
	*SD_CORE_ATTR(0) &= ~SD_CORE_DMA;

	// Init voices
	for(voice = 0; voice < 24; voice++)
	{
		#ifndef ISJPCM
		*SD_VP_VOLL(0, voice)	= 0;
		*SD_VP_VOLR(0, voice)	= 0;
		*SD_VP_PITCH(0, voice)	= 0x3FFF;
		*SD_VP_ADSR1(0, voice)	= 0;
		*SD_VP_ADSR2(0, voice)	= 0;
		#endif

		*SD_VP_VOLL(1, voice)	= 0;
		*SD_VP_VOLR(1, voice)	= 0;
		*SD_VP_PITCH(1, voice)	= 0x3FFF;
		*SD_VP_ADSR1(1, voice)	= 0;
		*SD_VP_ADSR2(1, voice)	= 0;

		#ifndef ISJPCM
		// Top address of waveform data
		*SD_VA_SSA_HI(0, voice)	= 0;
		*SD_VA_SSA_LO(0, voice)	= 0x5000 >> 1;
		#endif
		*SD_VA_SSA_HI(1, voice)	= 0;
		*SD_VA_SSA_LO(1, voice)	= 0x5000 >> 1;
	}

	// Set all voices to ON
	#ifndef ISJPCM
	*SD_A_KON_HI(0) = 0xFFFF;
	*SD_A_KON_LO(0) = 0xFF;
	#endif
	*SD_A_KON_HI(1) = 0xFFFF;
	*SD_A_KON_LO(1) = 0xFF;

	// There is no guarantee that voices will be turn on at once.
	// So we wait to make sure.
	nopdelay();

	// Set all voices to OFF
	#ifndef ISJPCM
	*SD_A_KOFF_HI(0) = 0xFFFF;
	*SD_A_KOFF_LO(0) = 0xFF;
	#endif
	*SD_A_KOFF_HI(1) = 0xFFFF;
	*SD_A_KOFF_LO(1) = 0xFF;

	// There is no guarantee that voices will be turn off at once.
	// So we wait to make sure.
	nopdelay();

	#ifndef ISJPCM
	*SD_S_ENDX_HI(0) = 0;
	*SD_S_ENDX_LO(0) = 0;
	#endif
}

// Core / Volume Registers
void InitCoreVolume(s32 flag)
{
	*SD_C_SPDIF_OUT = 0xC032;

	if(flag)
	{
		*SD_CORE_ATTR(0) = SD_SPU2_ON | SD_ENABLE_EFFECTS | SD_MUTE;
		*SD_CORE_ATTR(1) = SD_SPU2_ON | SD_ENABLE_EFFECTS | SD_MUTE | SD_ENABLE_EX_INPUT;
	}
	else
	{
		*SD_CORE_ATTR(0) = SD_SPU2_ON | SD_MUTE;
		*SD_CORE_ATTR(1) = SD_SPU2_ON | SD_MUTE | SD_ENABLE_EX_INPUT;
	}

	// HIgh is voices 0-15, LOw is 16-23, representing voices 0..23 (24)
	#ifndef ISJPCM
	*SD_S_VMIXL_HI(0)	= 0xFFFF;
	*SD_S_VMIXL_LO(0)	= 0xFF;
	*SD_S_VMIXR_HI(0)	= 0xFFFF;
	*SD_S_VMIXR_LO(0)	= 0xFF;
	*SD_S_VMIXEL_HI(0)	= 0xFFFF;
	*SD_S_VMIXEL_LO(0)	= 0xFF;
	*SD_S_VMIXER_HI(0)	= 0xFFFF;
	*SD_S_VMIXER_LO(0)	= 0xFF;
	#endif

	*SD_S_VMIXL_HI(1)	= 0xFFFF;
	*SD_S_VMIXL_LO(1)	= 0xFF;
	*SD_S_VMIXR_HI(1)	= 0xFFFF;
	*SD_S_VMIXR_LO(1)	= 0xFF;
	*SD_S_VMIXEL_HI(1)	= 0xFFFF;
	*SD_S_VMIXEL_LO(1)	= 0xFF;
	*SD_S_VMIXER_HI(1)	= 0xFFFF;
	*SD_S_VMIXER_LO(1)	= 0xFF;


	*SD_P_MMIX(0) = 0xFF0;
	*SD_P_MMIX(1) = 0xFFC;

	if(flag == 0)
	{
		#ifndef ISJPCM
		*SD_P_MVOLL(0) = 0;
		*SD_P_MVOLR(0) = 0;
		#endif

		*SD_P_MVOLL(1) = 0;
		*SD_P_MVOLR(1) = 0;

		#ifndef ISJPCM
		*SD_P_EVOLL(0) = 0;
		#endif
		*SD_P_EVOLL(1) = 0;

		#ifndef ISJPCM
		*SD_P_EVOLR(0) = 0;
		#endif
		*SD_P_EVOLR(1) = 0;

		// Effect End Address, Upper part
		#ifndef ISJPCM
		*SD_A_EEA_HI(0) = 0xE;
		#endif
		*SD_A_EEA_HI(1) = 0xF;
	}
	#ifndef ISJPCM
	*SD_P_AVOLL(0) = 0;
	*SD_P_AVOLR(0) = 0;
	#endif
	// Core 1 External Input Volume.
	// The external Input is Core 0's output.
	*SD_P_AVOLL(1) = 0x7FFF;
	*SD_P_AVOLR(1) = 0x7FFF;

	#ifndef ISJPCM
	*SD_P_BVOLL(0) = 0;
	*SD_P_BVOLR(0) = 0;
	#endif
	*SD_P_BVOLL(1) = 0;
	*SD_P_BVOLR(1) = 0;
}

void InitSpdif()
{
	*SD_C_SPDIF_MODE	= 0x900;
	*SD_C_SPDIF_MEDIA	= 0x200;
	*U16_REGISTER(0x7CA) = 8;
}

int sceSdInit(int flag)
{
	flag &= 1;

	InitSpu2();
	InitSpdif();

	if(flag == 0) ResetAll();
	RegisterInterrupts();

	InitVoices();
	InitCoreVolume(flag);

	EnableIntr(0x24);
	EnableIntr(0x28);
	#ifndef ISJPCM
	EnableIntr(9);
	#endif

	return 0;
}

#ifndef ISJPCM
void SetSpdifMode(u16 val)
{
	u16 out, mode;

	out  = *SD_C_SPDIF_OUT;
	mode = *SD_C_SPDIF_MODE;

	switch(val & 0xF)
	{
		case 0:
			mode &= 0xFFFD;
			out = (val & 0xFEF7) | 0x20;
			break;
		case 1:
			mode |= 2;
			out = (out & 0xFFD7) | 0x100;
			break;
		case 2:
			out &= 0xFED7;
			break;
		case 0xF:
			out = (out & 0xFEDF) | 8;
			break;
		default: return;
	}

	if(val & 0x80)
		mode |= 0x8000;
	else
		mode &= 0x7FFF;

	switch(val & 0xF00)
	{
		case 0x800:
			*SD_C_SPDIF_MEDIA = 0x200;
			mode |= 0x1800;
			break;
		case 0x400 :
			*SD_C_SPDIF_MEDIA = 0;
			mode &= 0xE7FF;
			break;
		default:
			*SD_C_SPDIF_MEDIA = 0x200;
			mode = (mode & 0xE7FF) | 0x800;
			break;
	}

	*SD_C_SPDIF_OUT = out;
	*SD_C_SPDIF_MODE = mode;

	SpdifSettings = val;
}
#endif


// Enable/disable bits in SD_CORE_ATTR
u8 CoreAttrShifts[4] = {7, 6, 14, 8};

void sceSdSetCoreAttr(u16 entry, u16 val)
{
	u16 core_attr = *SD_CORE_ATTR(entry & 1);

	switch(entry & ~1)
	{
		case SD_CORE_NOISE_CLK: // 0x8
			*SD_CORE_ATTR(entry & 1) = (core_attr-0x3F01) | ((val & 0x3F) << 8);
			break;

		#ifndef ISJPCM
		case SD_CORE_SPDIF_MODE: // 0xA
			SetSpdifMode(val); // sub1
			break;
		#endif
		default:
		{
			u32 core = entry & 1;
			entry = (entry >> 1)-1;
			core_attr &= ~(1 << CoreAttrShifts[entry]);
			core_attr |= (val & 1) << CoreAttrShifts[entry];
			*SD_CORE_ATTR(core) = core_attr;
		}
		break;
	}
}

void sceSdSetParam(u16 reg, u16 val)
{
	#ifndef ISJPCM
	u32 offs;
	u32 voice;
	u32 reg_index;
	u32 core;
	volatile u16 *reg_p;


	core = reg & 1;

	// Determine the channel offset
	if(reg & 0x80)
		offs = (40 * core) >> 1;
	else
		offs = (1024 * core) >> 1;

	reg_index = (reg >> 8) & 0xFF;
	voice = (reg & 0x3E) << 2;
	reg_p = ParamRegList[reg_index] + offs + voice;

	*reg_p = val;
	#else
	u32 core;


	core = reg & 1;
	reg &= ~1;

	switch(reg)
	{
		case SD_PARAM_MVOLL:
			*SD_P_MVOLL(core) = val;
		break;

		case SD_PARAM_MVOLR:
			*SD_P_MVOLR(core) = val;
		break;

		case SD_PARAM_BVOLL:
			*SD_P_BVOLL(core) = val;
		break;

		case SD_PARAM_BVOLR:
			*SD_P_BVOLR(core) = val;
		break;
	}
	#endif

}

SdIntrCallback sceSdSetTransCallback(s32 core, SdIntrCallback cb)
{
	SdIntrCallback old_cb;

	old_cb = TransIntrCallbacks[core & 1];
	TransIntrCallbacks[core & 1] = cb;

	return old_cb;
}

// DMA

u32 DmaStop(u32 core)
{
	u32 retval;

	core &= 1;

	if(*U16_REGISTER(0x1B0 + (core * 1024)) == 0)
		retval = 0;
	else
		retval = *SD_DMA_ADDR(core);

	*SD_DMA_CHCR(core) &= ~SD_DMA_START;
	*U16_REGISTER(0x1B0 + (core * 1024)) = 0;

	retval = (BlockTransBuff[core] << 24) | (retval & 0xFFFFFF);

	return retval;
}

void SetDmaWrite(s32 chan)
{
	volatile u32 *reg = U32_REGISTER(0x1014 + (chan << 10));
	*reg = (*reg & 0xF0FFFFFF) | 0x20000000;
}

void SetDmaRead(s32 chan)
{
	volatile u32 *reg = U32_REGISTER(0x1014 + (chan << 10));
	*reg = (*reg & 0xF0FFFFFF) | 0x22000000;
}

// Block Transfer
#ifndef ISJPCM
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
		if(mode & SD_TRANS_LOOP)
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

	// 0x26B8
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
#endif

s32 BlockTransWrite(u8 *iopaddr, u32 size, s32 chan)
{
	s32 core = chan & 1;

	BlockTransBuff[core] = 0;
	BlockTransSize[core] = size;
	BlockTransAddr[core] = (u32)iopaddr;

	if(*SD_CORE_ATTR(core) & SD_DMA_IN_PROCESS) return -1;
	if(*SD_DMA_CHCR(core) & SD_DMA_START) return -1;

	// 0x26B8
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


#ifndef ISJPCM
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
#endif


int sceSdBlockTrans(s16 chan, u16 mode, u8 *iopaddr, u32 size, ...)
{
	#ifndef ISJPCM
	int transfer_dir = mode & 3;
	#endif
	int core = chan & 1;
	int _size = size;
	va_list alist;
	u8* startaddr;

	#ifndef ISJPCM
	switch(transfer_dir)
	{
		case SD_TRANS_WRITE:
		{
			TransIntrData[core].mode = 0x100 | core;

			if(mode & SD_TRANS_LOOP)
			{
				TransIntrData[core].mode |= SD_TRANS_LOOP << 8;
				_size /= 2;
			}

			if(BlockTransWrite(iopaddr, _size, core) >= 0)
				return 0;

		} break;


		case SD_TRANS_READ:
		{
			TransIntrData[core].mode = 0x300 | core;

			if(mode & SD_TRANS_LOOP)
			{
				TransIntrData[core].mode |= SD_TRANS_LOOP << 8;
				_size /= 2;
			}

			if(BlockTransRead(iopaddr, _size, chan, mode) >= 0)
				return 0;

		} break;

		case SD_TRANS_STOP:
		{
			return DmaStop(core);

		} break;

		case SD_TRANS_WRITE_FROM:
		{
			va_start(alist, size);
			startaddr = va_arg(alist, u8*);
			va_end(alist);

			TransIntrData[core].mode = 0x100 | core;

			if(mode & SD_TRANS_LOOP)
			{
				TransIntrData[core].mode |= SD_TRANS_LOOP << 8;
				_size /= 2;
			}

			if(BlockTransWriteFrom(iopaddr, _size, core, mode, startaddr) >= 0)
				return 0;


		} break;


	}
	#else
	TransIntrData[core].mode = 0x100 | core;
	TransIntrData[core].mode |= SD_TRANS_LOOP << 8;
	_size /= 2;

	if(BlockTransWrite(iopaddr, _size, core) >= 0)
		return 0;
	#endif

	return -1;
}

u32 sceSdBlockTransStatus(s16 chan, s16 flag)
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

#endif

