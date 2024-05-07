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

void SpuGetVoiceRRAttr(int v_num, u16 *rr, int *rr_mode)
{
	u16 v3;

	v3 = _spu_RXX[512 * _spu_core + 4 + 8 * (v_num & 0x1F)];
	*rr = v3 & 0x1F;
	*rr_mode = SPU_VOICE_LINEARDecN;
	if ( (v3 & 0x20) != 0 )
		*rr_mode = SPU_VOICE_EXPDec;
}
