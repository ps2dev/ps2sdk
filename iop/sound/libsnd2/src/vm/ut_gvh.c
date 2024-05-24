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

s16 SsUtGetVabHdr(s16 vab_id, VabHdr *vab_hdr_ptr)
{
	VabHdr *vab_hdr_temp;

	if ( _svm_vab_used[vab_id] != 1 )
		return -1;
	vab_hdr_temp = _svm_vab_vh[vab_id];
	vab_hdr_ptr->form = vab_hdr_temp->form;
	vab_hdr_ptr->id = vab_hdr_temp->id;
	vab_hdr_ptr->ver = vab_hdr_temp->ver;
	vab_hdr_ptr->ps = vab_hdr_temp->ps;
	vab_hdr_ptr->ts = vab_hdr_temp->ts;
	vab_hdr_ptr->vs = vab_hdr_temp->vs;
	vab_hdr_ptr->mvol = vab_hdr_temp->mvol;
	vab_hdr_ptr->pan = vab_hdr_temp->pan;
	vab_hdr_ptr->attr1 = vab_hdr_temp->attr1;
	vab_hdr_ptr->attr2 = vab_hdr_temp->attr2;
	_svm_vh = vab_hdr_temp;
	return 0;
}
