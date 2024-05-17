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

s16 SsUtSetVabHdr(s16 vab_id, const VabHdr *vab_hdr_ptr)
{
	if ( _svm_vab_used[vab_id] != 1 )
		return -1;
	_svm_vh = _svm_vab_vh[vab_id];
	_svm_vh->mvol = vab_hdr_ptr->mvol;
	_svm_vh->pan = vab_hdr_ptr->pan;
	_svm_vh->attr1 = vab_hdr_ptr->attr2;
	_svm_vh->attr2 = vab_hdr_ptr->attr2;
	return 0;
}
