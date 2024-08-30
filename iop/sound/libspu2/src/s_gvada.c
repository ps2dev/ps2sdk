/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "libspu2_internal.h"

void SpuGetVoiceADSRAttr(
	int v_num, u16 *ar, u16 *dr, u16 *sr, u16 *rr, u16 *sl, int *ar_mode, int *sr_mode, int *rr_mode)
{
	const vu16 *v9;
	u16 v10;
	unsigned int v11;

	v9 = &_spu_RXX[512 * _spu_core + 8 * (v_num & 0x1F)];
	v10 = v9[3];
	v11 = v9[4];
	*ar = (v10 >> 8) & 0x3F;
	*ar_mode = SPU_VOICE_LINEARIncN;
	if ( (v10 & 0x8000) != 0 )
		*ar_mode = SPU_VOICE_EXPIncN;
	*dr = (u8)(v10 & 0xF0) >> 4;
	*sr = (v11 >> 6) & 0x7F;
	switch ( v11 & 0xE000 )
	{
		case 0xc000:
			*sr_mode = SPU_VOICE_EXPDec;
			break;
		case 0x8000:
			*sr_mode = SPU_VOICE_EXPIncN;
			break;
		case 0x4000:
			*sr_mode = SPU_VOICE_LINEARDecN;
			break;
		default:
			*sr_mode = SPU_VOICE_LINEARIncN;
			break;
	}
	*rr = v11 & 0x1F;
	*rr_mode = SPU_VOICE_LINEARDecN;
	if ( (v11 & 0x20) != 0 )
		*rr_mode = SPU_VOICE_EXPDec;
	*sl = v10 & 0xF;
}
