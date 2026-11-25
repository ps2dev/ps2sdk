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

unsigned int SsUtGetVagAddrFromTone(s16 vab_id, s16 prog, s16 tone)
{
	s16 vag;

	if ( _SsVmVSetUp(vab_id, prog) == -1 )
		return -1;
	vag = _svm_tn[16 * _svm_cur.m_fake_program + tone].vag;
	return ((( (vag & 1) != 0 ) ? _svm_pg[(vag - 1) / 2].m_vag_spu_addr_hi : _svm_pg[(vag - 1) / 2].m_vag_spu_addr_lo) << 4) | (gVabOffet[_svm_cur.m_vab_id] << 20);
}
