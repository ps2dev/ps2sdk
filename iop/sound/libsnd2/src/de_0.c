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

void _SsSetNrpnVabAttr0(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)fn_idx;

	SsUtGetVagAtr(vab_id, prog, tone, &vag_attr);
	vag_attr.prior = attribute_value;
	SsUtSetVagAtr(vab_id, prog, tone, &vag_attr);
}
