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

s16 _SsVmSelectToneAndVag(u8 *vag_attr_idx_ptr, u8 *vag_nums_ptr)
{
	u8 idx;
	int v4;
	const VagAtr *v6;

	idx = 0;
	for ( v4 = 0; (char)v4 < _svm_cur.m_sep_sep_no_tonecount; v4 += 1 )
	{
		v6 = &_svm_tn[16 * _svm_cur.m_fake_program + v4];
		if ( _svm_cur.m_note >= (int)v6->min )
		{
			if ( v6->max >= _svm_cur.m_note )
			{
				vag_nums_ptr[idx] = v6->vag;
				vag_attr_idx_ptr[(u8)idx] = v4;
				idx += 1;
			}
		}
	}
	return idx;
}
