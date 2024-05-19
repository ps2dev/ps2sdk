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

s16 SsUtGetVagAtr(s16 vab_id, s16 prog, s16 tone, VagAtr *vag_attr_ptr)
{
	const VagAtr *pVag;

	if ( _svm_vab_used[vab_id] != 1 )
		return -1;
	_SsVmVSetUp(vab_id, prog);
	pVag = &_svm_tn[16 * _svm_cur.m_fake_program + tone];
	vag_attr_ptr->prior = pVag->prior;
	vag_attr_ptr->mode = pVag->mode;
	vag_attr_ptr->vol = pVag->vol;
	vag_attr_ptr->pan = pVag->pan;
	vag_attr_ptr->center = pVag->center;
	vag_attr_ptr->shift = pVag->shift;
	vag_attr_ptr->max = pVag->max;
	vag_attr_ptr->min = pVag->min;
	vag_attr_ptr->vibW = pVag->vibW;
	vag_attr_ptr->vibT = pVag->vibT;
	vag_attr_ptr->porW = pVag->porW;
	vag_attr_ptr->porT = pVag->porT;
	vag_attr_ptr->pbmin = pVag->pbmin;
	vag_attr_ptr->pbmax = pVag->pbmax;
	vag_attr_ptr->adsr1 = pVag->adsr1;
	vag_attr_ptr->adsr2 = pVag->adsr2;
	vag_attr_ptr->prog = pVag->prog;
	vag_attr_ptr->vag = pVag->vag;
	return 0;
}
