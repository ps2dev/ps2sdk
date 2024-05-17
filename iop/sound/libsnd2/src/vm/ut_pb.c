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

s16 SsUtPitchBend(s16 vc, s16 vab_id, s16 prog, s16 note, s16 pbend)
{
	(void)note;

	_SsVmVSetUp(vab_id, prog);
	_svm_cur.m_seq_sep_no = 33;
	if ( _SsVmPBVoice(vc, 33, vab_id, prog, pbend) == 0 )
		return -1;
	return 0;
}
