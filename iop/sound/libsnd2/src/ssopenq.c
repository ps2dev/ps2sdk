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

s16 SsSeqOpen(unsigned int *addr, s16 vab_id)
{
	s16 open_counter_2;
	unsigned int open_counter_1;
	int seq_init;

	open_counter_2 = 0;
	if ( _snd_openflag == (u32)-1 )
	{
		printf("Can't Open Sequence data any more\n\n");
		return -1;
	}
	for ( open_counter_1 = 0; open_counter_1 < 32; open_counter_1 += 1 )
	{
		if ( (_snd_openflag & ((u32)1 << open_counter_1)) == 0 )
		{
			open_counter_2 = open_counter_1;
			break;
		}
	}
	_snd_openflag |= 1 << open_counter_2;
	seq_init = _SsInitSoundSeq(open_counter_2, vab_id, (u8 *)addr);
	SsFCALL.noteon = _SsNoteOn;
	SsFCALL.programchange = _SsSetProgramChange;
	SsFCALL.metaevent = _SsGetMetaEvent;
	SsFCALL.pitchbend = _SsSetPitchBend;
	SsFCALL.control[CC_NUMBER] = _SsSetControlChange;
	SsFCALL.control[CC_BANKCHANGE] = _SsContBankChange;
	SsFCALL.control[CC_MAINVOL] = _SsContMainVol;
	SsFCALL.control[CC_PANPOT] = _SsContPanpot;
	SsFCALL.control[CC_EXPRESSION] = _SsContExpression;
	SsFCALL.control[CC_DAMPER] = _SsContDamper;
	SsFCALL.control[CC_NRPN1] = _SsContNrpn1;
	SsFCALL.control[CC_NRPN2] = _SsContNrpn2;
	SsFCALL.control[CC_RPN1] = _SsContRpn1;
	SsFCALL.control[CC_RPN2] = _SsContRpn2;
	SsFCALL.control[CC_EXTERNAL] = _SsContExternal;
	SsFCALL.control[CC_RESETALL] = _SsContResetAll;
	SsFCALL.control[CC_DATAENTRY] = _SsContDataEntry;
	SsFCALL.ccentry[DE_PRIORITY] = _SsSetNrpnVabAttr0;
	SsFCALL.ccentry[DE_MODE] = _SsSetNrpnVabAttr1;
	SsFCALL.ccentry[DE_LIMITL] = _SsSetNrpnVabAttr2;
	SsFCALL.ccentry[DE_LIMITH] = _SsSetNrpnVabAttr3;
	SsFCALL.ccentry[DE_ADSR_AR_L] = _SsSetNrpnVabAttr4;
	SsFCALL.ccentry[DE_ADSR_AR_E] = _SsSetNrpnVabAttr5;
	SsFCALL.ccentry[DE_ADSR_DR] = _SsSetNrpnVabAttr6;
	SsFCALL.ccentry[DE_ADSR_SL] = _SsSetNrpnVabAttr7;
	SsFCALL.ccentry[DE_ADSR_SR_L] = _SsSetNrpnVabAttr8;
	SsFCALL.ccentry[DE_ADSR_SR_E] = _SsSetNrpnVabAttr9;
	SsFCALL.ccentry[DE_ADSR_RR_L] = _SsSetNrpnVabAttr10;
	SsFCALL.ccentry[DE_ADSR_RR_E] = _SsSetNrpnVabAttr11;
	SsFCALL.ccentry[DE_ADSR_SR] = _SsSetNrpnVabAttr12;
	SsFCALL.ccentry[DE_VIB_TIME] = _SsSetNrpnVabAttr13;
	SsFCALL.ccentry[DE_PORTA_DEPTH] = _SsSetNrpnVabAttr14;
	SsFCALL.ccentry[DE_REV_TYPE] = _SsSetNrpnVabAttr15;
	SsFCALL.ccentry[DE_REV_DEPTH] = _SsSetNrpnVabAttr16;
	SsFCALL.ccentry[DE_ECHO_FB] = _SsSetNrpnVabAttr17;
	SsFCALL.ccentry[DE_ECHO_DELAY] = _SsSetNrpnVabAttr18;
	SsFCALL.ccentry[DE_DELAY] = _SsSetNrpnVabAttr19;
	if ( seq_init == -1 )
		return -1;
	return open_counter_2;
}
