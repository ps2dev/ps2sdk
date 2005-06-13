/*
 * freesd.c - IOP Sound Driver
 *
 * Copyright (c) 2004 TyRaNiD <tiraniddo@hotmail.com>
 * Copyright (c) 2004 Lukasz Bruun <ps2@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include "types.h"
#include "irx.h"
#include "loadcore.h"
#include "stdio.h"
#include "intrman.h"
#include "freesd.h"
#include "spu2regs.h"

struct irx_export_table _exp_libsd;

#define BANNER "FREESD %s\n"
#define VERSION "v0.9"

#define MODNAME "freesd"

IRX_ID(MODNAME, 1, 1);

#define M_PRINTF(format, args...)	printf(MODNAME ": " format, ## args)

// block.c
extern u32 BlockTransBuff[2]; 
extern u32 BlockTransAddr[2];
extern u32 BlockTransSize[2];

// effect.c
extern u32 EffectSizes[10];
extern void SetESA(s32 core, u32 value);
extern u32 GetEEA(int chan);
extern SdEffectAttr EffectAttr[2];
extern u32 EffectAddr[2];

// voice.c
extern u32 VoiceTransStatus[2];
extern volatile u16 VoiceTransComplete[2];
extern u32 VoiceTransIoMode[2];

// Global
u16 SpdifSettings;
void *Spu2IntrData;
SdTransIntrHandler	TransIntrHandlers[2];
IntrCallback		TransIntrCallbacks[2];
SdSpu2IntrHandler	Spu2IntrHandler;
IntrCallback		Spu2IrqCallback;
IntrData			TransIntrData[2];


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


s32 _start(char **argv, int argc)
{	
	printf(BANNER, VERSION);
	
	if(RegisterLibraryEntries(&_exp_libsd) != 0) return 1;

	InitSpu2();

	return 0;
}

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
		// SD_C_STATX(core)
		// If done elsewise, it doesn't work, havn't figured out why yet.
		volatile u16 *statx = U16_REGISTER(0x344 + (core * 1024));

		if(!(*statx & 0x80))	while(!(*statx & 0x80));
		
		*SD_CORE_ATTR(core) &= ~SD_CORE_DMA;

		if(*SD_CORE_ATTR(core) & 0x30)	while((*SD_CORE_ATTR(core) & 0x30));
	
		if(TransIntrHandlers[core])		goto intr_handler;
		if(TransIntrCallbacks[core])	goto IntrCallback;
	
		VoiceTransComplete[core] = 1;
	}
	else
	{	// Block Transfer
		if(intr->mode & (SD_BLOCK_TRANS_LOOP << 8))
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
			*SD_CORE_ATTR(core) &= ~SD_CORE_DMA;
			*SD_P_MMIX(core)	&= 0xFF3F;
			*U16_REGISTER(0x1B0 + (core * 1024)) = 0;
		}
	
		if(TransIntrHandlers[core])		
		{	
			intr_handler:
			TransIntrHandlers[core](core, intr->data);
		}
		else
		{	
			if(TransIntrCallbacks[core])
			{
				IntrCallback:
				TransIntrCallbacks[core](0);
			}
		}
	}
	
	if(dir == SD_DMA_DIR_SPU2IOP)	FlushDcache();
	
	return 1;
}

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

void RegisterInterrupts()
{
	s32 ret;

	DisableIntr(0x24, (int *)&ret);
	DisableIntr(0x28, (int *)&ret);
	DisableIntr(0x9, (int *)&ret);
	
	ReleaseIntrHandler(0x24);
	ReleaseIntrHandler(0x28);

	RegisterIntrHandler(0x24, 1, TransInterrupt, &TransIntrData[0]);
	RegisterIntrHandler(0x28, 1, TransInterrupt, &TransIntrData[1]);

	VoiceTransComplete[0] = 0;
	VoiceTransComplete[1] = 0;
	
	ReleaseIntrHandler(0x9);
	RegisterIntrHandler(0x9, 1, Spu2Interrupt, &Spu2IntrData);
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
		VoiceTransIoMode[core]	= 0;
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



void Reset(s32 flag)
{
	s32 core;
	
	if(flag == 0) ResetAll();

	VoiceTransStatus[0]	= 1;
	VoiceTransStatus[1]	= 1;
	TransIntrCallbacks[0]	= NULL;
	TransIntrCallbacks[1]	= NULL;
	TransIntrHandlers[0]	= NULL;
	TransIntrHandlers[1]	= NULL;
	TransIntrData[0].mode	= 0;
	TransIntrData[1].mode	= 1;
	TransIntrData[0].data	= NULL;
	TransIntrData[1].data	= NULL;
	VoiceTransIoMode[0]		= 0;
	VoiceTransIoMode[1]		= 0;
	Spu2IntrHandler			= NULL;
	Spu2IrqCallback			= NULL;
	Spu2IntrData			= NULL;

	RegisterInterrupts();
	
	for(core = 0; core < 2; core++)
	{
		EffectAttr[core].core = core;
		EffectAttr[core].mode = 0;
		EffectAttr[core].depth_l = 0;
		EffectAttr[core].depth_r = 0;
		EffectAttr[core].delay = 0;
		EffectAttr[core].feedback = 0;
	}

	if(flag == 0)
	{
		for(core = 0; core < 2; core++)
		{
			EffectAddr[core] = GetEEA(core) - ((EffectSizes[0] << 4) - 2);
			SetESA(core, EffectAddr[core]);
		}
	}
}

SdSpu2IntrHandler SdSetSpu2IntrHandler(SdSpu2IntrHandler handler, void *data)
{
	SdSpu2IntrHandler old_handler;

	old_handler = Spu2IntrHandler;
	Spu2IntrHandler = handler;
	Spu2IntrData = data;

	return old_handler;
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
		*SD_VP_VOLL(0, voice)	= 0;
		*SD_VP_VOLR(0, voice)	= 0;
		*SD_VP_PITCH(0, voice)	= 0x3FFF;
		*SD_VP_ADSR1(0, voice)	= 0;
		*SD_VP_ADSR2(0, voice)	= 0;

		*SD_VP_VOLL(1, voice)	= 0;
		*SD_VP_VOLR(1, voice)	= 0;
		*SD_VP_PITCH(1, voice)	= 0x3FFF;
		*SD_VP_ADSR1(1, voice)	= 0;
		*SD_VP_ADSR2(1, voice)	= 0;
		
		// Top address of waveform data
		*SD_VA_SSA_HI(0, voice)	= 0;
		*SD_VA_SSA_LO(0, voice)	= 0x5000 >> 1;
		*SD_VA_SSA_HI(1, voice)	= 0;
		*SD_VA_SSA_LO(1, voice)	= 0x5000 >> 1;
	}

	// Set all voices to ON
	*SD_A_KON_HI(0) = 0xFFFF;
	*SD_A_KON_LO(0) = 0xFF;
	*SD_A_KON_HI(1) = 0xFFFF;
	*SD_A_KON_LO(1) = 0xFF;
	
	// There is no guarantee that voices will be turn on at once.
	// So we wait to make sure.
	nopdelay();

	// Set all voices to OFF
	*SD_A_KOFF_HI(0) = 0xFFFF;
	*SD_A_KOFF_LO(0) = 0xFF;
	*SD_A_KOFF_HI(1) = 0xFFFF;
	*SD_A_KOFF_LO(1) = 0xFF;

	// There is no guarantee that voices will be turn off at once.
	// So we wait to make sure.
	nopdelay();
	
	*SD_S_ENDX_HI(0) = 0;
	*SD_S_ENDX_LO(0) = 0;
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
	*SD_S_VMIXL_HI(0)	= 0xFFFF;
	*SD_S_VMIXL_LO(0)	= 0xFF;
	*SD_S_VMIXR_HI(0)	= 0xFFFF;
	*SD_S_VMIXR_LO(0)	= 0xFF;
	*SD_S_VMIXEL_HI(0)	= 0xFFFF;
	*SD_S_VMIXEL_LO(0)	= 0xFF;
	*SD_S_VMIXER_HI(0)	= 0xFFFF;
	*SD_S_VMIXER_LO(0)	= 0xFF;

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
		*SD_P_MVOLL(0) = 0;
		*SD_P_MVOLR(0) = 0;
		*SD_P_MVOLL(1) = 0;
		*SD_P_MVOLR(1) = 0;

		*SD_P_EVOLL(0) = 0;
		*SD_P_EVOLL(1) = 0;
		
		*SD_P_EVOLR(0) = 0;
		*SD_P_EVOLR(1) = 0;
		
		// Effect End Address, Upper part
		*SD_A_EEA_HI(0) = 0xE;
		*SD_A_EEA_HI(1) = 0xF;
	}

	*SD_P_AVOLL(0) = 0;
	*SD_P_AVOLR(0) = 0;
	// Core 1 External Input Volume.
	// The external Input is Core 0's output.
	*SD_P_AVOLL(1) = 0x7FFF;
	*SD_P_AVOLR(1) = 0x7FFF;

	*SD_P_BVOLL(0) = 0;
	*SD_P_BVOLR(0) = 0;
	*SD_P_BVOLL(1) = 0;
	*SD_P_BVOLR(1) = 0;
}

void InitSpdif()
{
	*SD_C_SPDIF_MODE	= 0x900;
	*SD_C_SPDIF_MEDIA	= 0x200;
	*U16_REGISTER(0x7CA) = 8;
}

s32 SdInit(s32 flag)
{
	flag &= 1;

	InitSpu2();
	InitSpdif();
	
	Reset(flag);
	
	InitVoices();
	InitCoreVolume(flag);
	
	EnableIntr(0x24); 
	EnableIntr(0x28);
	EnableIntr(9);

	return 0;
}

void SdSetParam(u16 reg, u16 val)
{
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
}

u16 SdGetParam(u16 reg)
{
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

	return *reg_p;
}

void SdSetSwitch(u16 reg, u32 val)
{
	u32 reg_index;
	volatile u16 *reg_p;

	reg_index = (reg >> 8) & 0xFF;
	reg_p = ParamRegList[reg_index] + ((reg & 1) << 9);

	reg_p[0] = (u16)val;
	reg_p[1] = (u16)((val >> 16) & 0xFF);
}

u32 SdGetSwitch(u16 reg)
{
	u32 reg_index;
	volatile u16 *reg_p;
	u32 ret;
	
	reg_index = (reg >> 8) & 0xFF;
	reg_p = ParamRegList[reg_index] + ((reg & 1) << 9);
	
	ret =  reg_p[0];
	ret |= reg_p[1] << 16;

	return ret;
}


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

u16 SdNote2Pitch(u16 centre_note, u16 centre_fine, u16 note, s16 fine)
{
	return 0;
}

u16 SdPitch2Note(u16 centre_note, u16 centre_fine, u16 pitch)
{
	return 0;
}

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

// Enable/disable bits in SD_CORE_ATTR
u8 CoreAttrShifts[4] = {7, 6, 14, 8};

void SdSetCoreAttr(u16 entry, u16 val)
{
	u16 core_attr = *SD_CORE_ATTR(entry & 1);

	switch(entry & ~1)
	{
		case SD_CORE_NOISE_CLK: // 0x8 
			*SD_CORE_ATTR(entry & 1) = (core_attr-0x3F01) | ((val & 0x3F) << 8);
			break;
		case SD_CORE_SPDIF_MODE: // 0xA 
			SetSpdifMode(val); // sub1
			break;
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

u16 SdGetCoreAttr(u16 entry)
{
	switch(entry & ~1)
	{ 
		case SD_CORE_NOISE_CLK:
			return (*SD_CORE_ATTR(entry & 1) >> 8) & 0x3F;
		case SD_CORE_SPDIF_MODE:
			return SpdifSettings;
		default:
		{
			u32 core = entry & 1;
			entry = (entry >> 1)-1;
			return (*SD_CORE_ATTR(core) >> CoreAttrShifts[entry]) & 1;
		}
	}
}


SdTransIntrHandler SdSetTransIntrHandler(s32 chan, SdTransIntrHandler func, void *data)
{
	SdTransIntrHandler old_handler;

	old_handler = TransIntrHandlers[chan & 1];
	TransIntrHandlers[chan & 1] = func;
	TransIntrData[chan & 1].data = data;
	
	return 0;
}



s32 SdQuit()
{
	s32 ret;

	DmaStop(0);
	DmaStop(1);

	DisableIntr(0x28, (int *)&ret);
	DisableIntr(0x24, (int *)&ret);
	DisableIntr(0x9, (int *)&ret);
	ReleaseIntrHandler(0x28);
	ReleaseIntrHandler(0x24);
	ReleaseIntrHandler(0x9);

	return 0;
}

void SdSetAddr(u16 reg, u32 val)
{
	volatile u16 *reg1;
	u16 voice;

	reg1 = ParamRegList[(reg >> 8) & 0xFF] + ((reg & 1) << 9);
	voice = reg & 0x3E;
	
	reg1 += ((voice << 1) + voice);
	
	*reg1++ = (val >> 17) & 0xFFFF;
	
	if((reg & 0xFF00) != 0x1D00)
	{
		*reg1 = (val >> 1) & 0xFFF8;
	}
}

u32 SdGetAddr(u16 reg)

{
	volatile u16 *reg1;
	u16 voice;
	u32 retlo, rethi;

	reg1 = ParamRegList[(reg >> 8) & 0xFF] + ((reg & 1) << 9);
	voice = reg & 0x3E;
	reg1 += ((voice << 1) + voice);
	rethi = *reg1 << 17;
	retlo = 0x1FFFF;

	reg &= 0xFF00;
	
	if(reg != 0x1D00)
	{
		retlo = *(reg1 + 1) << 1;

		if((reg == 0x2100) || (reg == 0x2200))
		{
			u32 lo, hi;
		
			hi = *reg1 << 17;
			lo = *(reg1 + 1) << 1;
		
			if((rethi == hi) || (retlo != lo))
			{
				retlo = lo;
				rethi = hi;
			}
		}
	}

	return rethi | retlo;
}

IntrCallback SdSetTransCallback(s32 core, IntrCallback cb)
{
	IntrCallback old_cb;

	old_cb = TransIntrCallbacks[core & 1];
	TransIntrCallbacks[core & 1] = cb;

	return old_cb;
}

IntrCallback SdSetIRQCallback(IntrCallback cb)
{
	IntrCallback old_cb;
	
	old_cb = Spu2IrqCallback;
	Spu2IrqCallback = cb;

	return old_cb;
}
