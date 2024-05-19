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

unsigned int SpuWrite(u8 *addr, unsigned int size)
{
	unsigned int size_tmp;

	size_tmp = size;
	if ( size > 0x1FAFF0 )
		size_tmp = 0x1FAFF0;
	_spu_Fw(addr, size_tmp);
	if ( !_spu_transferCallback )
		_spu_inTransfer = 0;
	return size_tmp;
}

unsigned int SpuAutoDMAWrite(u8 *addr, unsigned int size, unsigned int mode, ...)
{
#ifdef LIB_1300
	if ( mode == SPU_AUTODMA_LOOP )
		size >>= 1;
	_spu_FwAutoDMA(addr, size, mode);
	return size;
#else
	u8 *v6;
	va_list va;
	unsigned int mode_masked_1;

	va_start(va, mode);
	v6 = va_arg(va, u8 *);
	va_end(va);
#ifdef LIB_1600
	mode_masked_1 = mode & 1;
#else
	mode_masked_1 = mode;
#endif
	if ( (mode & SPU_AUTODMA_LOOP) != 0 )
		size >>= 1;
	if ( (mode & SPU_AUTODMA_START_ADDR) != 0 )
		return _spu_FwAutoDMAfrom(addr, size, mode_masked_1, v6);
	return _spu_FwAutoDMA(addr, size, mode_masked_1);
#endif
}

int SpuAutoDMAStop(void)
{
	return _spu_StopAutoDMA();
}

int SpuAutoDMAGetStatus(void)
{
	return _spu_AutoDMAGetStatus();
}
