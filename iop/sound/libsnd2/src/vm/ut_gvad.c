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

int SsUtGetVagAddr(s16 vab_id, s16 vag_id)
{
	int m_vag_spu_addr;

	if ( _SsVmVSetUp(vab_id, 0) == -1 )
		return -1;
	if ( (vag_id & 1) != 0 )
	{
		m_vag_spu_addr = _svm_pg[(vag_id - 1) / 2].m_vag_spu_addr_hi;
	}
	else
	{
		m_vag_spu_addr = _svm_pg[(vag_id - 1) / 2].m_vag_spu_addr_lo;
	}
	return (m_vag_spu_addr << 4) | (gVabOffet[_svm_cur.m_vab_id] << 20);
}
