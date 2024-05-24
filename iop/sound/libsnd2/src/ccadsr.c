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

void _SsUtResolveADSR(u16 adsr1, u16 adsr2, u16 *adsr_buf)
{
	adsr_buf[5] = adsr1 & 0x8000;
	adsr_buf[6] = adsr2 & 0x8000;
	adsr_buf[8] = adsr2 & 0x4000;
	adsr_buf[7] = adsr2 & 0x20;
	*adsr_buf = (adsr1 >> 8) & 0x7F;
	adsr_buf[1] = (u8)adsr1 >> 4;
	adsr_buf[2] = adsr1 & 0xF;
	adsr_buf[3] = (adsr2 >> 6) & 0x7F;
	adsr_buf[4] = adsr2 & 0x1F;
}

void _SsUtBuildADSR(const u16 *adsr_buf, u16 *adsr1, u16 *adsr2)
{
	unsigned int v5;
	s16 v6;
	unsigned int v7;
	u16 v8;

	v5 = adsr_buf[6] != 0 ? 0xFFFF8000 : 0;
	v6 = v5;
	v7 = adsr_buf[5] != 0 ? 0xFFFF8000 : 0;
	if ( adsr_buf[8] )
		v6 = v5 | 0x4000;
	if ( adsr_buf[7] )
		v6 |= 0x20u;
	v8 = v6 | ((adsr_buf[3] << 6) & 0x1FC0) | (adsr_buf[4] & 0x1F);
	*adsr1 = v7 | ((*adsr_buf << 8) & 0x7F00) | ((16 * adsr_buf[1]) & 0xF0) | (adsr_buf[2] & 0xF);
	*adsr2 = v8;
}
