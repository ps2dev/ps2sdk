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

int SsVoKeyOn(int vab_pro, int pitch, u16 voll, u16 volr)
{
	return _SsVmSeKeyOn(vab_pro >> 8, (u8)vab_pro, (unsigned int)pitch >> 8, (u8)pitch, voll, volr);
}
