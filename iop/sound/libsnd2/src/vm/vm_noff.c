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

void vmNoiseOff(u8 vc)
{
	libsnd2_spu_voice_t *voice_struct;

	voice_struct = &_svm_voice[vc];
	voice_struct->m_unk1d = 0;
	voice_struct->m_vag_idx = 0;
	voice_struct->m_pitch = 0;
}
