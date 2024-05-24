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

void _SsVmKeyOffNow(void)
{
	int bits_upper;
	int bits_lower;
	int m_voice_idx;
	u16 okof1_tmp;
	u16 okof2_tmp;
	libsnd2_spu_voice_t *voice_struct;

	m_voice_idx = (u16)_svm_cur.m_voice_idx;
	if ( (u16)m_voice_idx >= 0x10u )
	{
		bits_upper = 0;
		bits_lower = 1 << ((m_voice_idx & 0xFF) - 16);
	}
	else
	{
		bits_upper = 1 << (m_voice_idx & 0xFF);
		bits_lower = 0;
	}
	voice_struct = &_svm_voice[m_voice_idx];
	voice_struct->m_unk1d = 0;
	okof1_tmp = _svm_okof1;
	okof2_tmp = _svm_okof2;
	voice_struct->m_pitch = 0;
	voice_struct->m_vag_idx = 0;
	_svm_okof1 = okof1_tmp | bits_upper;
	_svm_okon1 &= ~(okof1_tmp | bits_upper);
	_svm_okof2 = okof2_tmp | bits_lower;
	_svm_okon2 &= ~(okof2_tmp | bits_lower);
}
