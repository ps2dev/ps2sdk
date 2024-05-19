/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "libspu2_internal.h"

int _spu_eea[4] = {15, 15, 0, 0};

int _spu_core;

void SpuInit(void)
{
	_spu_core = 0;
	_spu2_config_initialize();
	_SpuInit(0);
	_spu2_config_initialize_typically();
}

unsigned int SpuSetCore(unsigned int which_core)
{
	unsigned int result;

	result = _spu_core;
	_spu_core = which_core & 1;
	return result;
}

unsigned int SpuGetCore(void)
{
	return _spu_core;
}

void SpuSetReverbEndAddr(unsigned int eea)
{
	_spu_eea[_spu_core] = (eea >> 17) & 0xF;
	_spu_RXX[512 * _spu_core + 207] = _spu_eea[_spu_core];
}

unsigned int SpuGetReverbEndAddr(void)
{
	return (_spu_RXX[512 * _spu_core + 414] << 17) | 0x1FFFF;
}
