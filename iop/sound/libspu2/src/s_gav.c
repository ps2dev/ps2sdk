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

unsigned int _SpuGetAnyVoice(int word_idx1, int word_idx2)
{
	return _spu_RXX[512 * _spu_core + word_idx1] | ((u8)_spu_RXX[512 * _spu_core + word_idx2] << 16);
}
