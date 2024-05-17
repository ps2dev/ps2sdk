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

void vmNoiseOn2(u8 vc, u16 voll, u16 volr, u16 arg3, u16 arg4)
{
	int vc_mask_tmp1;
	int vc_mask_tmp2;
	u16 okon1_tmp;
	u16 okon2_tmp;
	libsnd2_spu_voice_t *voice_struct;

	(void)arg3;
	(void)arg4;

	voice_struct = &_svm_voice[vc];
	_svm_sreg_buf[vc].m_vol_left = voll;
	_svm_sreg_buf[vc].m_vol_right = volr;
	_svm_sreg_dirty[vc] |= 3;
	if ( vc >= 0x10u )
	{
		vc_mask_tmp1 = 0;
		vc_mask_tmp2 = 1 << (vc - 16);
	}
	else
	{
		vc_mask_tmp1 = 1 << vc;
		vc_mask_tmp2 = 0;
	}
	voice_struct->m_pitch = 10;
	voice_struct->m_unk1d = 2;
	okon1_tmp = _svm_okon1;
	okon2_tmp = _svm_okon2;
	voice_struct->m_unk02 = 0;
	_svm_okon1 = okon1_tmp | vc_mask_tmp1;
	_svm_okon2 = okon2_tmp | vc_mask_tmp2;
	_svm_okof1 &= ~(okon1_tmp | vc_mask_tmp1);
	_svm_okof2 &= ~(okon2_tmp | vc_mask_tmp2);
	SpuSetNoiseVoice(SPU_ON, ((u8)vc_mask_tmp2 << 16) | (u16)vc_mask_tmp1);
}
