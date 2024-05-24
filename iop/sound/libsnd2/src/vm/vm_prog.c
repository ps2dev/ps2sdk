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

void _SsVmSetProgVol(s16 vab_id, s16 prog, u8 vol)
{
	if ( !_SsVmVSetUp(vab_id, prog) )
		_svm_pg[prog].mvol = vol;
}

int _SsVmGetProgVol(s16 vab_id, s16 prog)
{
	if ( _SsVmVSetUp(vab_id, prog) != 0 )
		return -1;
	return _svm_pg[prog].mvol;
}

int _SsVmSetProgPan(s16 vab_id, s16 prog, char mpan)
{
	if ( _SsVmVSetUp(vab_id, prog) != 0 )
	{
		return -1;
	}
	_svm_pg[prog].mpan = mpan;
	return _svm_pg[prog].mpan;
}

int _SsVmGetProgPan(s16 vab_id, s16 prog)
{
	if ( _SsVmVSetUp(vab_id, prog) != 0 )
		return -1;
	return _svm_pg[prog].mpan;
}
