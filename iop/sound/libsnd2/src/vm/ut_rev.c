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

s16 SsUtSetReverbType(s16 type)
{
	int flag_tmp;
	int type_tmp1;
	int type_mode_flag_tmp;
	s16 type_tmp2;

	flag_tmp = 0;
	type_tmp1 = type;
	if ( (type & 0x8000) != 0 )
	{
		flag_tmp = 1;
		type_tmp1 = -type;
	}
	if ( (u16)type_tmp1 >= SS_REV_TYPE_MAX )
		return -1;
	_svm_rattr.mask = SPU_REV_MODE;
	if ( flag_tmp )
		type_mode_flag_tmp = (type_tmp1 | SPU_REV_MODE_CLEAR_WA);
	else
		type_mode_flag_tmp = type_tmp1;
	_svm_rattr.mode = type_mode_flag_tmp;
	type_tmp2 = type_tmp1;
	if ( !(u16)type_tmp1 )
		SpuSetReverb(SPU_OFF);
	SpuSetReverbModeParam(&_svm_rattr);
	return type_tmp2;
}

s16 SsUtGetReverbType(void)
{
	return _svm_rattr.mode;
}
