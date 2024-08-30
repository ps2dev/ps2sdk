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

void _spu2_config_iop(void)
{
	*((vu32 *)0xBF801404) = 0xBF900000;
	*((vu32 *)0xBF80140C) = 0xBF900800;
	*((vu32 *)0xBF8010F0) |= 0x80000u;
	*((vu32 *)0xBF801570) |= 8u;
	*((vu32 *)0xBF801014) = 0x200B31E1;
	*((vu32 *)0xBF801414) = 0x200B31E1;
}

static void _spu2_config_SPDIF(int flag)
{
	*((vu16 *)0xBF9007C6) = 2304;
	*((vu16 *)0xBF9007C8) = 512;
	if ( flag )
		*((vu16 *)0xBF9007C8) = 0;
	*((vu16 *)0xBF9007CA) = 8;
}

void _spu2_config_initialize(void)
{
	_spu2_config_iop();
	_spu2_config_SPDIF(0);
}

void _spu2_config_initialize_typically(void)
{
	*((vu16 *)0xBF9007C0) = 0xC032;
	*((vu16 *)0xBF90019A) = 0xC000;
	*((vu16 *)0xBF90059A) = 0xC001;
	*((vu16 *)0xBF900188) = 0xFFFF;
	*((vu16 *)0xBF90018A) = 0xFF;
	*((vu16 *)0xBF900190) = 0xFFFF;
	*((vu16 *)0xBF900192) = 0xFF;
	*((vu16 *)0xBF90018C) = 0xFFFF;
	*((vu16 *)0xBF90018E) = 0xFF;
	*((vu16 *)0xBF900194) = 0xFFFF;
	*((vu16 *)0xBF900196) = 0xFF;
	*((vu16 *)0xBF900588) = 0xFFFF;
	*((vu16 *)0xBF90058A) = 0xFF;
	*((vu16 *)0xBF900590) = 0xFFFF;
	*((vu16 *)0xBF900592) = 0xFF;
	*((vu16 *)0xBF90058C) = 0xFFFF;
	*((vu16 *)0xBF90058E) = 0xFF;
	*((vu16 *)0xBF900594) = 0xFFFF;
	*((vu16 *)0xBF900596) = 0xFF;
	*((vu16 *)0xBF900198) = 0xFFF;
	*((vu16 *)0xBF900598) = 0xFFF;
	*((vu16 *)0xBF900760) = 0;
	*((vu16 *)0xBF900762) = 0;
	*((vu16 *)0xBF900788) = 0;
	*((vu16 *)0xBF90078A) = 0;
	*((vu16 *)0xBF900764) = 0;
	*((vu16 *)0xBF900766) = 0;
	*((vu16 *)0xBF90078C) = 0;
	*((vu16 *)0xBF90078E) = 0;
	*((vu16 *)0xBF900768) = 0;
	*((vu16 *)0xBF90076A) = 0;
	*((vu16 *)0xBF900790) = 0x7FFF;
	*((vu16 *)0xBF900792) = 0x7FFF;
	*((vu16 *)0xBF90076C) = 0;
	*((vu16 *)0xBF90076E) = 0;
	*((vu16 *)0xBF900794) = 0;
	*((vu16 *)0xBF900796) = 0;
	*((vu16 *)0xBF90033C) = 0xE;
	*((vu16 *)0xBF90073C) = 0xF;
}

void _spu2_config_initialize_hot(void)
{
	*((vu16 *)0xBF9007C0) = 0xC032;
	*((vu16 *)0xBF90019A) = 0xC080;
	*((vu16 *)0xBF90059A) = 0xC081;
	*((vu16 *)0xBF900188) = 0xFFFF;
	*((vu16 *)0xBF90018A) = 0xFF;
	*((vu16 *)0xBF900190) = 0xFFFF;
	*((vu16 *)0xBF900192) = 0xFF;
	*((vu16 *)0xBF90018C) = 0xFFFF;
	*((vu16 *)0xBF90018E) = 0xFF;
	*((vu16 *)0xBF900194) = 0xFFFF;
	*((vu16 *)0xBF900196) = 0xFF;
	*((vu16 *)0xBF900588) = 0xFFFF;
	*((vu16 *)0xBF90058A) = 0xFF;
	*((vu16 *)0xBF900590) = 0xFFFF;
	*((vu16 *)0xBF900592) = 0xFF;
	*((vu16 *)0xBF90058C) = 0xFFFF;
	*((vu16 *)0xBF90058E) = 0xFF;
	*((vu16 *)0xBF900594) = 0xFFFF;
	*((vu16 *)0xBF900596) = 0xFF;
	*((vu16 *)0xBF900198) = 0xFFF;
	*((vu16 *)0xBF900598) = 0xFFF;
	*((vu16 *)0xBF900768) = 0;
	*((vu16 *)0xBF90076A) = 0;
	*((vu16 *)0xBF900790) = 0x7FFF;
	*((vu16 *)0xBF900792) = 0x7FFF;
	*((vu16 *)0xBF90076C) = 0;
	*((vu16 *)0xBF90076E) = 0;
	*((vu16 *)0xBF900794) = 0;
	*((vu16 *)0xBF900796) = 0;
}

void _spu2_config_before_compatible(void)
{
	*((vu16 *)0xBF9007C0) = 0xC032;
	*((vu16 *)0xBF900188) = 0xFFFF;
	*((vu16 *)0xBF90018A) = 0xFF;
	*((vu16 *)0xBF900190) = 0xFFFF;
	*((vu16 *)0xBF900192) = 0xFF;
	*((vu16 *)0xBF90018C) = 0xFFFF;
	*((vu16 *)0xBF90018E) = 0xFF;
	*((vu16 *)0xBF900194) = 0xFFFF;
	*((vu16 *)0xBF900196) = 0xFF;
	*((vu16 *)0xBF900198) = 0xFFF;
	*((vu16 *)0xBF90033C) = 3;
	_spu2_config_SPDIF(1);
}
