/*
 * effect.c - Part of the IOP Sound Driver
 *
 * Copyright (c) 2004 TyRaNiD <tiraniddo@hotmail.com>
 * Copyright (c) 2004 Lukasz Bruun <ps2@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include "types.h"
#include "freesd.h"
#include "spu2regs.h"


extern u32 VoiceTransIoMode[2];
extern SdTransIntrHandler TransIntrHandlers[2];
extern IntrCallback TransIntrCallbacks[2];

SdEffectAttr EffectAttr[2];
u32 EffectAddr[2];

// The values are more or less the same as on PSX (SPU)
u16 EffectSizes[10] = { 0x1, 0x26C, 0x1F4, 0x484, 0x6FE, 0xADE, 0xF6C, 0x1804, 0x1804, 0x3C0 };

u32 EffectParams[160] = {
	// SD_EFFECT_MODE_OFF 
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00010001, 0x00000000, 0x00000000,
	0x00000000, 0x00010001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
	// SD_EFFECT_MODE_ROOM	
	0x005B007D, 0x54B86D80, 0x0000BED0, 0xBA800000, 0x53005800, 0x033304D6, 0x022703F0, 0x01EF0374, 
	0x01B70336, 0x01B60335, 0x01B50334, 0x01B50334, 0x01B50334, 0x013601B4, 0x005C00B8, 0x80008000, 
	// SD_EFFECT_MODE_STUDIO_1
	0x00250033, 0x4FA870F0, 0x4410BCE0, 0x9C00C0F0, 0x4EC05280, 0x031B03E4, 0x02AF03A4, 0x02660372, 
	0x025D031C, 0x018E025C, 0x0135022F, 0x00B701D2, 0x00B5018F, 0x008000B4, 0x0026004C, 0x80008000,	
	// SD_EFFECT_MODE_STUDIO_2
	0x007F00B1, 0x4FA870F0, 0x4510BCE0, 0xB4C0BEF0, 0x4EC05280, 0x076B0904, 0x065F0824, 0x061607A2, 
	0x05ED076C, 0x042E05EC, 0x0305050F, 0x02B70462, 0x0265042F, 0x01B20264, 0x00800100, 0x80008000,	
	// SD_EFFECT_MODE_STUDIO_3
	0x00A900E3, 0x4FA86F60, 0x4510BCE0, 0xA680BEF0, 0x52C05680, 0x0B580DFB, 0x0A3C0D09, 0x09730BD9, 
	0x08DA0B59, 0x05E908D9, 0x04B007EC, 0x03D206EF, 0x031D05EA, 0x0238031C, 0x00AA0154, 0x80008000,	
	// SD_EFFECT_MODE_HALL
	0x013901A5, 0x50006000, 0xB8004C00, 0xC000BC00, 0x5C006000, 0x11BB15BA, 0x10BD14C2, 0x0DC111BC, 
	0x0DC311C0, 0x09C10DC0, 0x07C10BC4, 0x06CD0A00, 0x05C109C2, 0x041A05C0, 0x013A0274, 0x80008000, 
	// SD_EFFECT_MODE_SPACE 
	0x0231033D, 0x50007E00, 0xB000B400, 0xB0004C00, 0x54006000, 0x1A311ED6, 0x183B1D14, 0x16B21BC2, 
	0x15EF1A32, 0x105515EE, 0x0F2D1334, 0x0C5D11F6, 0x0AE11056, 0x07A20AE0, 0x02320464, 0x80008000, 
	// SD_EFFECT_MODE_ECHO
	0x00030003, 0x7FFF7FFF,	0x00000000, 0x81000000, 0x00000000, 0x0FFD1FFD, 0x00091009, 0x00000000, 
	0x00091009, 0x1FFF1FFF, 0x1FFE1FFE, 0x1FFE1FFE, 0x1FFE1FFE, 0x10041008, 0x00040008, 0x80008000,	
	// SD_EFFECT_MODE_DELAY
	0x00030003, 0x7FFF7FFF,	0x00000000,	0x00000000,	0x00000000, 0x0FFD1FFD, 0x00091009, 0x00000000, 
	0x00091009, 0x1FFF1FFF, 0x1FFE1FFE, 0x1FFE1FFE, 0x1FFE1FFE, 0x10041008, 0x00040008, 0x80008000,	 
	// SD_EFFECT_MODE_CLEAR
	0x00130017, 0x4FA870F0, 0x4510BCE0, 0x8500BEF0, 0x54C05F80, 0x02AF0371, 0x01DF02E5, 0x01D702B0,
	0x026A0358, 0x011E01D6, 0x00B1012D, 0x0059011F, 0x00E301A0, 0x00400058, 0x00140028, 0x80008000,	
};


const u32 ClearEffectData[256] __attribute__((aligned(16))) = 
{ 
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};


u32 GetEEA(s32 core)
{
	return (*SD_A_EEA_HI(core) << 17) | 0x1FFFF; 
}

void SetESA(s32 core, u32 value)
{
	volatile u16 *reg = SD_A_ESA_HI(core);

	reg[0] = value >> 17;
	reg[1] = value >> 1;

}

void SetEffectRegister(volatile u16* reg, u32 val)
{
	reg[0] = (u16)(val >> 14);
	reg[1] = (u16)(val << 2);
}

void SetEffectData(u16 *mode_data, u32 core)
{
	s32 i;

	SetEffectRegister(SD_R_FB_SRC_A(core), mode_data[0]);
	SetEffectRegister(SD_R_FB_SRC_B(core), mode_data[1]);

	for(i=0; i < 8; i++)
		*(SD_R_IIR_ALPHA(core)+i) = mode_data[2+i];

	for(i=0; i < 20; i++)
		SetEffectRegister(SD_R_IIR_DEST_A0(core)+(i*2), mode_data[10+i]);
	
	*SD_R_IN_COEF_L(core) = mode_data[30];
	*SD_R_IN_COEF_R(core) = mode_data[31];
}

s32 SdSetEffectAttr(s32 core, SdEffectAttr *attr)
{
	u32 mode = attr->mode;
	u32 clearram = 0;
	u32 effects_disabled = 0;
	u32 last_mode = 0;
	u16	mode_data[32];

	core &= 1;

	if(mode & SD_EFFECT_MODE_CLEAR)
	{
		mode &= ~SD_EFFECT_MODE_CLEAR;
		clearram = 1;
		last_mode = EffectAttr[core].mode;
	}

	// Check if valid mode
	if(mode > 9) return -1;
	
	EffectAddr[core] = GetEEA(core) - ((EffectSizes[mode] << 4) - 2);

	memcpy(mode_data, &EffectParams[mode * 16], 64);
	
	memcpy(&EffectAttr[core], attr, sizeof(SdEffectAttr));
	EffectAttr[core].core = core;

	switch(mode)
	{
		case SD_EFFECT_MODE_ECHO: 
			EffectAttr[core].feedback = 0x80;
			EffectAttr[core].delay = 0x80;
			break;
		case SD_EFFECT_MODE_DELAY: 
			EffectAttr[core].feedback = 0;
			EffectAttr[core].delay = 0x80;
			break;
		default:
			EffectAttr[core].feedback = 0;
			EffectAttr[core].delay = 0;
			break;
	}

	if((mode == SD_EFFECT_MODE_DELAY) || (mode == SD_EFFECT_MODE_ECHO))
	{ 
		u16 delay		= ((attr->delay+1) << 5);
		u16 feedback	= ((attr->feedback << 7) + attr->feedback) << 1;
	
		mode_data[7]  = feedback;
		mode_data[10] = (delay << 1) - mode_data[0];
		mode_data[11] = delay - mode_data[1];
		
		mode_data[16] = mode_data[17] + delay;
		mode_data[12] = mode_data[13] + delay;
		mode_data[27] = mode_data[29] + delay;
		mode_data[26] = mode_data[28] + delay;
	}

	// Disable effects
	if(*SD_CORE_ATTR(core) & SD_ENABLE_EFFECTS)
	{
		effects_disabled = 1;
		*SD_CORE_ATTR(core) &= ~SD_ENABLE_EFFECTS;
	}
	
	// Clean up after last mode
	if((effects_disabled) && (clearram))
		SdClearEffectWorkArea(core, 0, last_mode);
	
	// Depth / Volume
	*SD_P_EVOLL(core) = attr->depth_l;
	*SD_P_EVOLR(core) = attr->depth_r;

	SetEffectData(mode_data, core);

	// Set effect start addr (ESA)
	SetESA(core, EffectAddr[core]);
	

	if(clearram)	
		SdClearEffectWorkArea(core, 0, mode);

	// Enable effects
	if(effects_disabled)	
		*SD_CORE_ATTR(core) |= SD_ENABLE_EFFECTS;

	return 0;
}

void SdGetEffectAttr(int core, SdEffectAttr *attr)
{
	*attr = EffectAttr[core & 1];
}

s32 SdClearEffectWorkArea(s32 core, s32 chan, s32 effect_type)
{
	if(effect_type > 9) return -1;
	
	if(effect_type != SD_EFFECT_MODE_OFF)
	{
		u32 aligned_addr = 0;
		u32 effect_addr;
		s32 effect_size;
		s32 old_iomode;
		SdTransIntrHandler old_handler = NULL;
		IntrCallback old_callback = NULL;
		
		effect_size = EffectSizes[effect_type] << 4; 
		
		effect_addr = (GetEEA(core) - (effect_size - 2));

		if(effect_size & 127)
		{
			effect_size &= ~127;
			aligned_addr = (GetEEA(core) - (effect_size - 2));
		}
			
		old_iomode = VoiceTransIoMode[chan];
	
		if(VoiceTransIoMode[chan] == 1) VoiceTransIoMode[chan] = 0;
		
		// Disable intr_handlers by removing them
		if(TransIntrHandlers[chan])
		{
			old_handler = TransIntrHandlers[chan];
			TransIntrHandlers[chan] = 0;
		}

		if(TransIntrCallbacks[chan])
		{
			old_callback = TransIntrCallbacks[chan];
			TransIntrCallbacks[chan] = NULL;
		}

		if(aligned_addr)
		{
			SdVoiceTrans(chan, 0, (u8*)ClearEffectData, (u32*)effect_addr, 64);
			SdVoiceTransStatus(chan, 1);
			effect_addr = aligned_addr;
		}


		while(effect_size > 0)
		{
			u32 size;

			if(effect_size < 1024)
				size = effect_size;
			else
				size = 1024;
		
			SdVoiceTrans(chan, 0, (u8*)ClearEffectData, (u32*)effect_addr, size);
			SdVoiceTransStatus(chan, 1); // Wait for completion
			
			effect_size -= 1024;
			effect_addr += 1024;
		
		}

		VoiceTransIoMode[chan] = old_iomode;
	
		// Enable intr_handlers by adding them again
		if(old_handler) TransIntrHandlers[chan] = old_handler;
		if(old_callback) TransIntrCallbacks[chan] = old_callback;
	}

	return 0;
}

