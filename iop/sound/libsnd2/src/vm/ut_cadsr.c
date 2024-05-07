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

s16 SsUtChangeADSR(s16 vc, s16 vab_id, s16 prog, s16 old_note, u16 adsr1, u16 adsr2)
{
	const libsnd2_spu_voice_t *voice_struct;

	if ( (u16)vc >= 24u )
		return -1;
	voice_struct = &_svm_voice[vc];
	if ( voice_struct->m_vab_id != vab_id || voice_struct->m_prog != prog || voice_struct->m_note != old_note )
	{
		return -1;
	}
	_svm_sreg_buf[vc].m_adsr1 = adsr1;
	_svm_sreg_buf[vc].m_adsr2 = adsr2;
	_svm_sreg_dirty[vc] |= 0x30;
	return 0;
}
