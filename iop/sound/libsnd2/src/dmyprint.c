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

static int is_first_dmy_SsNoteOn = 0;
static int is_first_dmy_SsSetProgramChange = 0;
static int is_first_dmy_SsGetMetaEvent = 0;
static int is_first_dmy_SsSetPitchBend = 0;
static int is_first_dmy_SsSetControlChange = 0;
static int is_first_dmy_SsContBankChange = 0;
static int is_first_dmy_SsContDataEntry = 0;
static int is_first_dmy_SsContMainVol = 0;
static int is_first_dmy_SsContPanpot = 0;
static int is_first_dmy_SsContExpression = 0;
static int is_first_dmy_SsContDamper = 0;
static int is_first_dmy_SsContExternal = 0;
static int is_first_dmy_SsContNrpn1 = 0;
static int is_first_dmy_SsContNrpn2 = 0;
static int is_first_dmy_SsContRpn1 = 0;
static int is_first_dmy_SsContRpn2 = 0;
static int is_first_dmy_SsContResetAll = 0;
static int is_first_dmy_SsSetNrpnVabAttr0 = 0;
static int is_first_dmy_SsSetNrpnVabAttr1 = 0;
static int is_first_dmy_SsSetNrpnVabAttr2 = 0;
static int is_first_dmy_SsSetNrpnVabAttr3 = 0;
static int is_first_dmy_SsSetNrpnVabAttr4 = 0;
static int is_first_dmy_SsSetNrpnVabAttr5 = 0;
static int is_first_dmy_SsSetNrpnVabAttr6 = 0;
static int is_first_dmy_SsSetNrpnVabAttr7 = 0;
static int is_first_dmy_SsSetNrpnVabAttr8 = 0;
static int is_first_dmy_SsSetNrpnVabAttr9 = 0;
static int is_first_dmy_SsSetNrpnVabAttr10 = 0;
static int is_first_dmy_SsSetNrpnVabAttr11 = 0;
static int is_first_dmy_SsSetNrpnVabAttr12 = 0;
static int is_first_dmy_SsSetNrpnVabAttr13 = 0;
static int is_first_dmy_SsSetNrpnVabAttr14 = 0;
static int is_first_dmy_SsSetNrpnVabAttr15 = 0;
static int is_first_dmy_SsSetNrpnVabAttr16 = 0;
static int is_first_dmy_SsSetNrpnVabAttr17 = 0;
static int is_first_dmy_SsSetNrpnVabAttr18 = 0;
static int is_first_dmy_SsSetNrpnVabAttr19 = 0;

void dmy_nothing1(s16 seq_no, s16 sep_no, u8 note, u8 vollr)
{
	(void)sep_no;
	(void)seq_no;
	(void)note;
	(void)vollr;
}

void dmy_SsNoteOn(s16 sep_no, s16 seq_no, u8 note, u8 vollr)
{
	(void)sep_no;
	(void)seq_no;
	(void)note;
	(void)vollr;
	if ( !is_first_dmy_SsNoteOn )
	{
		printf("_SsNoteOn\n");
		is_first_dmy_SsNoteOn = 1;
	}
}

void dmy_SsSetProgramChange(s16 sep_no, s16 seq_no, u8 prog)
{
	(void)sep_no;
	(void)seq_no;
	(void)prog;
	if ( !is_first_dmy_SsSetProgramChange )
	{
		printf("_SsSetProgramChange\n");
		is_first_dmy_SsSetProgramChange = 1;
	}
}

void dmy_SsGetMetaEvent(s16 sep_no, s16 seq_no, u8 meta_event)
{
	(void)sep_no;
	(void)seq_no;
	(void)meta_event;
	if ( !is_first_dmy_SsGetMetaEvent )
	{
		printf("_SsGetMetaEvent\n");
		is_first_dmy_SsGetMetaEvent = 1;
	}
}

void dmy_SsSetPitchBend(s16 sep_no, s16 seq_no)
{
	(void)sep_no;
	(void)seq_no;
	if ( !is_first_dmy_SsSetPitchBend )
	{
		printf("_SsSetPitchBend\n");
		is_first_dmy_SsSetPitchBend = 1;
	}
}

void dmy_SsSetControlChange(s16 sep_no, s16 seq_no, u8 control_value)
{
	(void)sep_no;
	(void)seq_no;
	(void)control_value;
	if ( !is_first_dmy_SsSetControlChange )
	{
		printf("_SsSetControlChange\n");
		is_first_dmy_SsSetControlChange = 1;
	}
}

void dmy_SsContBankChange(s16 sep_no, s16 seq_no, u8 control_value)
{
	(void)sep_no;
	(void)seq_no;
	(void)control_value;
	if ( !is_first_dmy_SsContBankChange )
	{
		printf("_SsContBankChange\n");
		is_first_dmy_SsContBankChange = 1;
	}
}

void dmy_SsContDataEntry(s16 sep_no, s16 seq_no, u8 control_value)
{
	(void)sep_no;
	(void)seq_no;
	(void)control_value;
	if ( !is_first_dmy_SsContDataEntry )
	{
		printf("_SsContDataEntry\n");
		is_first_dmy_SsContDataEntry = 1;
	}
}

void dmy_SsContMainVol(s16 sep_no, s16 seq_no, u8 control_value)
{
	(void)sep_no;
	(void)seq_no;
	(void)control_value;
	if ( !is_first_dmy_SsContMainVol )
	{
		printf("_SsContMainVol\n");
		is_first_dmy_SsContMainVol = 1;
	}
}

void dmy_SsContPanpot(s16 sep_no, s16 seq_no, u8 control_value)
{
	(void)sep_no;
	(void)seq_no;
	(void)control_value;
	if ( !is_first_dmy_SsContPanpot )
	{
		printf("_SsContPanpot\n");
		is_first_dmy_SsContPanpot = 1;
	}
}

void dmy_SsContExpression(s16 sep_no, s16 seq_no, u8 control_value)
{
	(void)sep_no;
	(void)seq_no;
	(void)control_value;
	if ( !is_first_dmy_SsContExpression )
	{
		printf("_SsContExpression\n");
		is_first_dmy_SsContExpression = 1;
	}
}

void dmy_SsContDamper(s16 sep_no, s16 seq_no, u8 control_value)
{
	(void)sep_no;
	(void)seq_no;
	(void)control_value;
	if ( !is_first_dmy_SsContDamper )
	{
		printf("_SsContDamper\n");
		is_first_dmy_SsContDamper = 1;
	}
}

void dmy_SsContExternal(s16 sep_no, s16 seq_no, u8 control_value)
{
	(void)sep_no;
	(void)seq_no;
	(void)control_value;
	if ( !is_first_dmy_SsContExternal )
	{
		printf("_SsContExternal\n");
		is_first_dmy_SsContExternal = 1;
	}
}

void dmy_SsContNrpn1(s16 sep_no, s16 seq_no, u8 control_value)
{
	(void)sep_no;
	(void)seq_no;
	(void)control_value;
	if ( !is_first_dmy_SsContNrpn1 )
	{
		printf("_SsContNrpn1\n");
		is_first_dmy_SsContNrpn1 = 1;
	}
}

void dmy_SsContNrpn2(s16 sep_no, s16 seq_no, u8 control_value)
{
	(void)sep_no;
	(void)seq_no;
	(void)control_value;
	if ( !is_first_dmy_SsContNrpn2 )
	{
		printf("_SsContNrpn2\n");
		is_first_dmy_SsContNrpn2 = 1;
	}
}

void dmy_SsContRpn1(s16 sep_no, s16 seq_no, u8 control_value)
{
	(void)sep_no;
	(void)seq_no;
	(void)control_value;
	if ( !is_first_dmy_SsContRpn1 )
	{
		printf("_SsContRpn1\n");
		is_first_dmy_SsContRpn1 = 1;
	}
}

void dmy_SsContRpn2(s16 sep_no, s16 seq_no, u8 control_value)
{
	(void)sep_no;
	(void)seq_no;
	(void)control_value;
	if ( !is_first_dmy_SsContRpn2 )
	{
		printf("_SsContRpn2\n");
		is_first_dmy_SsContRpn2 = 1;
	}
}

void dmy_SsContResetAll(s16 sep_no, s16 seq_no, u8 control_value)
{
	(void)sep_no;
	(void)seq_no;
	(void)control_value;
	if ( !is_first_dmy_SsContResetAll )
	{
		printf("_SsContResetAll\n");
		is_first_dmy_SsContResetAll = 1;
	}
}

void dmy_SsSetNrpnVabAttr0(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr0 )
	{
		printf("_SsSetNrpnVabAttr0\n");
		is_first_dmy_SsSetNrpnVabAttr0 = 1;
	}
}

void dmy_SsSetNrpnVabAttr1(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr1 )
	{
		printf("_SsSetNrpnVabAttr1\n");
		is_first_dmy_SsSetNrpnVabAttr1 = 1;
	}
}

void dmy_SsSetNrpnVabAttr2(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr2 )
	{
		printf("_SsSetNrpnVabAttr2\n");
		is_first_dmy_SsSetNrpnVabAttr2 = 1;
	}
}

void dmy_SsSetNrpnVabAttr3(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr3 )
	{
		printf("_SsSetNrpnVabAttr3\n");
		is_first_dmy_SsSetNrpnVabAttr3 = 1;
	}
}

void dmy_SsSetNrpnVabAttr4(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr4 )
	{
		printf("_SsSetNrpnVabAttr4\n");
		is_first_dmy_SsSetNrpnVabAttr4 = 1;
	}
}

void dmy_SsSetNrpnVabAttr5(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr5 )
	{
		printf("_SsSetNrpnVabAttr5\n");
		is_first_dmy_SsSetNrpnVabAttr5 = 1;
	}
}

void dmy_SsSetNrpnVabAttr6(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr6 )
	{
		printf("_SsSetNrpnVabAttr6\n");
		is_first_dmy_SsSetNrpnVabAttr6 = 1;
	}
}

void dmy_SsSetNrpnVabAttr7(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr7 )
	{
		printf("_SsSetNrpnVabAttr7\n");
		is_first_dmy_SsSetNrpnVabAttr7 = 1;
	}
}

void dmy_SsSetNrpnVabAttr8(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr8 )
	{
		printf("_SsSetNrpnVabAttr8\n");
		is_first_dmy_SsSetNrpnVabAttr8 = 1;
	}
}

void dmy_SsSetNrpnVabAttr9(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr9 )
	{
		printf("_SsSetNrpnVabAttr9\n");
		is_first_dmy_SsSetNrpnVabAttr9 = 1;
	}
}

void dmy_SsSetNrpnVabAttr10(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr10 )
	{
		printf("_SsSetNrpnVabAttr10\n");
		is_first_dmy_SsSetNrpnVabAttr10 = 1;
	}
}

void dmy_SsSetNrpnVabAttr11(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr11 )
	{
		printf("_SsSetNrpnVabAttr11\n");
		is_first_dmy_SsSetNrpnVabAttr11 = 1;
	}
}

void dmy_SsSetNrpnVabAttr12(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr12 )
	{
		printf("_SsSetNrpnVabAttr12\n");
		is_first_dmy_SsSetNrpnVabAttr12 = 1;
	}
}

void dmy_SsSetNrpnVabAttr13(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr13 )
	{
		printf("_SsSetNrpnVabAttr13\n");
		is_first_dmy_SsSetNrpnVabAttr13 = 1;
	}
}

void dmy_SsSetNrpnVabAttr14(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr14 )
	{
		printf("_SsSetNrpnVabAttr14\n");
		is_first_dmy_SsSetNrpnVabAttr14 = 1;
	}
}

void dmy_SsSetNrpnVabAttr15(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr15 )
	{
		printf("_SsSetNrpnVabAttr15\n");
		is_first_dmy_SsSetNrpnVabAttr15 = 1;
	}
}

void dmy_SsSetNrpnVabAttr16(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr16 )
	{
		printf("_SsSetNrpnVabAttr16\n");
		is_first_dmy_SsSetNrpnVabAttr16 = 1;
	}
}

void dmy_SsSetNrpnVabAttr17(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr17 )
	{
		printf("_SsSetNrpnVabAttr17\n");
		is_first_dmy_SsSetNrpnVabAttr17 = 1;
	}
}

void dmy_SsSetNrpnVabAttr18(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr18 )
	{
		printf("_SsSetNrpnVabAttr18\n");
		is_first_dmy_SsSetNrpnVabAttr18 = 1;
	}
}

void dmy_SsSetNrpnVabAttr19(s16 vab_id, s16 prog, s16 tone, VagAtr vag_attr, s16 fn_idx, u8 attribute_value)
{
	(void)vab_id;
	(void)prog;
	(void)tone;
	(void)vag_attr;
	(void)fn_idx;
	(void)attribute_value;
	if ( !is_first_dmy_SsSetNrpnVabAttr19 )
	{
		printf("_SsSetNrpnVabAttr19\n");
		is_first_dmy_SsSetNrpnVabAttr19 = 1;
	}
}
