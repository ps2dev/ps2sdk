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

void _SsVmDoAllocate(void)
{
	int v0;
	u16 vag_spu_addr;
	const VagAtr *v6;
	s16 damper;
	libsnd2_spu_voice_t *voice_struct;

	voice_struct = &_svm_voice[_svm_cur.m_voice_idx];
	voice_struct->m_key_stat = 0x7FFF;
	for ( v0 = 0; v0 < 16; v0 += 1 )
	{
		_svm_envx_hist[v0] &= ~(1 << (_svm_cur.m_voice_idx & 0xFF));
	}
	if ( (_svm_cur.m_vag_idx2 & 1) != 0 )
	{
		vag_spu_addr = _svm_pg[(_svm_cur.m_vag_idx2 - 1) / 2].m_vag_spu_addr_hi;
	}
	else
	{
		vag_spu_addr = _svm_pg[(_svm_cur.m_vag_idx2 - 1) / 2].m_vag_spu_addr_lo;
	}
	_svm_sreg_buf2[_svm_cur.m_voice_idx].m_vag_spu_addr = vag_spu_addr;
	_svm_sreg_dirty[_svm_cur.m_voice_idx] |= 8u;
	_svm_sreg_buf2[_svm_cur.m_voice_idx].m_vab_spu_offset = gVabOffet[_svm_cur.m_vab_id];
	v6 = &_svm_tn[16 * _svm_cur.m_fake_program + _svm_cur.m_tone];
	_svm_sreg_buf[_svm_cur.m_voice_idx].m_adsr1 = v6->adsr1;
	damper = _svm_damper + (v6->adsr2 & 0x1F);
	if ( damper >= 32 )
		damper = 31;
	_svm_sreg_buf[_svm_cur.m_voice_idx].m_adsr2 = damper | (v6->adsr2 & ~0x1F);
	_svm_sreg_dirty[_svm_cur.m_voice_idx] |= 0x30u;
}
