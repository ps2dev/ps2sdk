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

s16 SsUtSetProgAtr(s16 vab_id, s16 prog, const ProgAtr *prog_attr_ptr)
{
	ProgAtr *pProg;

	if ( _svm_vab_used[vab_id] != 1 )
		return -1;
	_SsVmVSetUp(vab_id, prog);
	pProg = &_svm_pg[prog];
	pProg->mvol = prog_attr_ptr->mvol;
	pProg->prior = prog_attr_ptr->prior;
	pProg->mode = prog_attr_ptr->mode;
	pProg->mpan = prog_attr_ptr->mpan;
	pProg->attr = prog_attr_ptr->attr;
	return 0;
}
