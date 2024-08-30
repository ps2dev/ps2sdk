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

void SpuGetVoiceAR(int v_num, u16 *ar)
{
	*ar = ((_spu_RXX[512 * _spu_core + 3 + 8 * (v_num & 0x1F)]) >> 8) & 0x3F;
}
