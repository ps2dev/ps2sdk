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

int _SsVmVSetUp(s16 vab_id, s16 prog)
{
	if ( (u16)vab_id >= 0x10u )
		return -1;
	if ( _svm_vab_used[vab_id] != 1 )
		return -1;
	if ( prog >= kMaxPrograms )
		return -1;
	_svm_cur.m_vab_id = vab_id;
	_svm_cur.m_program = prog;
	_svm_tn = _svm_vab_tn[vab_id];
	_svm_vh = _svm_vab_vh[vab_id];
	_svm_pg = _svm_vab_pg[vab_id];
	_svm_cur.m_fake_program = _svm_pg[prog].m_fake_prog_idx;
	return 0;
}
