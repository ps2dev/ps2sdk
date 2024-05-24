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

s16 SsUtGetDetVVol(s16 vc, s16 *detvoll, s16 *detvolr)
{
	if ( (u16)vc >= 0x18u )
		return -1;
	SpuGetVoiceVolume(vc, detvoll, detvolr);
	return 0;
}

s16 SsUtSetDetVVol(s16 vc, s16 detvoll, s16 detvolr)
{
	if ( (u16)vc >= 0x18u )
		return -1;
	_svm_sreg_buf[vc].m_vol_left = detvoll;
	_svm_sreg_buf[vc].m_vol_right = detvolr;
	_svm_sreg_dirty[vc] |= 3;
	return 0;
}

s16 SsUtGetVVol(s16 vc, s16 *voll, s16 *volr)
{
	s16 voll_tmp;
	s16 volr_tmp;

	if ( (u16)vc >= 0x18u )
		return -1;
	SpuGetVoiceVolume(vc, &voll_tmp, &volr_tmp);
	*voll = voll_tmp / 129;
	*volr = volr_tmp / 129;
	return 0;
}

s16 SsUtSetVVol(s16 vc, s16 voll, s16 volr)
{
	if ( (u16)vc >= 0x18u )
		return -1;
	_svm_sreg_buf[vc].m_vol_left = 129 * voll;
	_svm_sreg_buf[vc].m_vol_right = 129 * volr;
	_svm_sreg_dirty[vc] |= 3;
	return 0;
}
