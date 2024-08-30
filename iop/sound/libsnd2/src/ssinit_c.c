/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "libsnd2_internal.h"

s16 gPitchCorrect = 0;

void SsInit(void)
{
	SpuInit();
	_SsInit();
}

void SsPitchCorrect(s16 pitch_correct)
{
	gPitchCorrect = pitch_correct;
}
