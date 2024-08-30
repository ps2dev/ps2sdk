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

void _SsSetNrpnVabAttr8(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	u16 resolved_adsr[12];

	(void)fn_idx;

	SsUtGetVagAtr(vab_id, prog, tone, &vag_attr);
	_SsUtResolveADSR(vag_attr.adsr1, vag_attr.adsr2, resolved_adsr);
	resolved_adsr[6] = 0;
	resolved_adsr[3] = attribute_value;
	_SsUtBuildADSR(resolved_adsr, &vag_attr.adsr1, &vag_attr.adsr2);
	SsUtSetVagAtr(vab_id, prog, tone, &vag_attr);
}
