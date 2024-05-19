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

vu16 *_spu_RXX = (vu16 *)0xBF900000;
u32 _spu_tsa[2] = {0u, 0u};
u32 _spu_transMode = 0u;
u32 _spu_inTransfer = 1u;
SpuTransferCallbackProc _spu_transferCallback = NULL;
SpuTransferCallbackProc _spu_AutoDMACallback = NULL;
SpuIRQCallbackProc _spu_IRQCallback = NULL;
static int g_DmaCoreIndex = 0;
static int _spu_dma_mode = 0;
static int _spu_transfer_startaddr = 0;
static int _spu_transfer_time = 0;

static int gMode;
static int gWhichBuff;
static u8 *gHostAddr;
static int gBufferSize48;

u16 _spu_RQ[16];

static void _spu_FsetDelayW(int flag);
static void _spu_FsetDelayR(int flag);

int _spu_init(int flag)
{
	if ( !flag )
	{
		vu16 *v2;

		*((vu32 *)0xBF8010F0) |= 0xB0000u;
		v2 = &_spu_RXX[20 * _spu_core];
		v2[944] = 0;
		v2[945] = 0;
		v2[1456] = 0;
		v2[1457] = 0;
#ifndef LIB_1300
		// Added in OSDSND 110U
		*((vu16 *)0xBF9007C0) = 0;
		_spu_Fw1ts();
		_spu_Fw1ts();
		*((vu16 *)0xBF9007C0) = 0x8000;
		_spu_Fw1ts();
#endif
		for ( _spu_core = 0; (unsigned int)_spu_core < 2; _spu_core += 1 )
		{
			vu16 *v7;
			vu16 *v8;
			unsigned int v9;
			vu16 *v11;
			vu16 *v12;
			int v13;

			_spu_transMode = 0;
			_spu_tsa[_spu_core] = 0;
#ifndef LIB_1300
			// Added in OSDSND 110U
			*(u16 *)((_spu_core << 10) + 0xbf9001b0) = 0;
#endif
			_spu_RXX[512 * _spu_core + 205] = 0;
			_spu_Fw1ts();
			_spu_Fw1ts();
			v7 = &_spu_RXX[512 * _spu_core];
			v7[205] = 0x8000;
			v8 = &_spu_RXX[20 * _spu_core];
			v8[944] = 0;
			v8[945] = 0;
			v9 = 1;
			while ( (v7[418] & 0x7FF) != 0 )
			{
				if ( v9 >= 0xF01 )
				{
					printf("SPU:T/O [%s]\n", "wait (reset)");
					break;
				}
				v9 += 1;
			}
			v11 = &_spu_RXX[20 * _spu_core];
			v12 = &_spu_RXX[512 * _spu_core];
			v11[946] = 0;
			v11[947] = 0;
			v12[210] = -1;
			v12[211] = -1;
			_spu_RXX[205] &= ~0x80u;
			for ( v13 = 0; v13 < 10; v13 += 1 )
			{
				_spu_RQ[v13] = 0;
			}
		}
		for ( _spu_core = 0; (unsigned int)_spu_core < 2; _spu_core += 1 )
		{
			vu16 *v17;
			vu16 *v18;

			v17 = &_spu_RXX[512 * _spu_core];
			v17[192] = 0;
			v17[193] = 0;
			v17[194] = 0;
			v17[195] = 0;
			v18 = &_spu_RXX[20 * _spu_core];
			v18[968] = 0;
			v18[969] = 0;
			v18[970] = 0;
			v18[971] = 0;
		}
	}
	_spu_core = 0;
	SpuStopFreeRun();
	_spu_inTransfer = 1;
	_spu_transferCallback = 0;
	_spu_IRQCallback = 0;
	return 0;
}

int spu_do_set_DmaCoreIndex(int dma_core_index)
{
	int (*v1)(void *);

	g_DmaCoreIndex = dma_core_index;
	if ( dma_core_index )
	{
		_SpuDataCallback(_spu_FiAutoDMA);
		v1 = _spu_FiDMA;
	}
	else
	{
		_SpuDataCallback(_spu_FiDMA);
		v1 = _spu_FiAutoDMA;
	}
	_SpuAutoDMACallback(v1);
	return g_DmaCoreIndex;
}

int spu_do_get_DmaCoreIndex(void)
{
	return g_DmaCoreIndex;
}

static void _spu_FwriteByIO(void *addr, u32 size)
{
	_spu_RXX[725] = _spu_tsa[1];
	_spu_RXX[724] = (_spu_tsa[1] >> 16) & 0xFFFF;
	while ( size )
	{
		s32 v6;
		s32 i;
		unsigned int v9;

		v6 = 64;
		if ( size <= 64 )
			v6 = size;
		for ( i = 0; i < v6; i += 2 )
		{
			*((vu16 *)0xBF9001AC) = *(u16 *)((char *)addr + 2 * i);
		}
		*((vu16 *)0xBF90019A) = (*((vu16 *)0xBF90019A) & ~0x30) | 0x10;
		v9 = 1;
		while ( (*((vu16 *)0xBF900344) & 0x400) != 0 )
		{
			if ( v9 >= 0xF01 )
			{
				printf("SPU:T/O [%s]\n", "wait (SPU2_STATX_WRDY_M)");
				break;
			}
			v9 += 1;
		}
		size -= v6;
	}
	*((vu16 *)0xBF90019A) &= ~0x30;
}

int _spu_FiDMA(void *userdata)
{
	int v1;
	unsigned int v3;

	(void)userdata;

	v1 = 1;
	while ( (*((vu16 *)0xBF900744) & 0x80) != 0 )
	{
		if ( v1 > 0x1000000 )
		{
			printf("SPU:T/O [%s]\n", "wait (SPU2_STATX_DREQ)");
			break;
		}
		v1 += 1;
	}
	_spu_RXX[717] &= ~0x30;
	v3 = 1;
	while ( (_spu_RXX[717] & 0x30) != 0 )
	{
		if ( v3 >= 0xF01 )
			break;
		v3 += 1;
	}
	if ( _spu_transferCallback )
		_spu_transferCallback();
	else
		gDMADeliverEvent = 1;
	FlushDcache();
	return 1;
}

int _spu_FiAutoDMA(void *userdata)
{
	(void)userdata;

#ifdef LIB_1600
	if ( gMode != SPU_AUTODMA_ONESHOT )
	{
		gWhichBuff = 1 - gWhichBuff;
		if ( gWhichBuff )
			*((vu32 *)0xBF8010C0) = (u32)&gHostAddr[512 * (gBufferSize48 / 512)];
		else
			*((vu32 *)0xBF8010C0) = (u32)&gHostAddr[0];
		if ( gWhichBuff )
		{
			int v1;
			int v2;

			v1 = (gBufferSize48 / 512) << 9;
			v2 = (2 * gBufferSize48 - v1) >> 6;
			if ( 2 * gBufferSize48 - v1 < 0 )
				v2 = (2 * gBufferSize48 - v1 + 63) >> 6;
			*((vu16 *)0xBF8010C6) = v2 + ((2 * gBufferSize48 - ((gBufferSize48 / 512) << 9)) % 64 > 0);
		}
		else
		{
			*((vu16 *)0xBF8010C6) = 8 * (gBufferSize48 / 512);
		}
		*((vu32 *)0xBF8010C8) = 0x1000201;
	}
#else
	if ( (gMode & SPU_AUTODMA_LOOP) != 0 )
	{
		gWhichBuff = 1 - gWhichBuff;
		if ( gWhichBuff )
			*(u32 *)(1088 * g_DmaCoreIndex + 0xBF8010C0) = (u32)&gHostAddr[512 * (gBufferSize48 / 512)];
		else
			*(u32 *)(1088 * g_DmaCoreIndex + 0xBF8010C0) = (u32)gHostAddr;
		if ( gWhichBuff )
			*(u16 *)(1088 * g_DmaCoreIndex + 0xBF8010C6) = (2 * gBufferSize48 - ((gBufferSize48 / 512) << 9)) / 64
																									 + ((2 * gBufferSize48 - ((gBufferSize48 / 512) << 9)) % 64 > 0);
		else
			*(u16 *)(1088 * g_DmaCoreIndex + 0xBF8010C6) = 8 * (gBufferSize48 / 512);
		*(u32 *)(1088 * g_DmaCoreIndex + 0xBF8010C8) = 0x1000201;
	}
#endif
	else
	{
		*((vu16 *)0xBF90019A) &= ~0x30;
		*((vu16 *)0xBF900198) &= ~0xc0;
		*((vu16 *)0xBF9001B0) = 0;
	}
	if ( _spu_AutoDMACallback )
		_spu_AutoDMACallback();
	FlushDcache();
	return 1;
}

void _spu_Fr_(void *data, int addr, u32 size)
{
	_spu_RXX[725] = addr;
	_spu_RXX[724] = (addr >> 16) & 0xFFFF;
	_spu_RXX[717] |= 0x30u;
	_spu_FsetDelayR(1);
	*((vu32 *)0xBF8010C0) = (vu32)data;
	*((vu32 *)0xBF8010C4) = (size << 16) | 0x10;
	_spu_dma_mode = 1;
	*((vu32 *)0xBF8010C8) = 16777728;
}

int _spu_t(int count, ...)
{
	u32 v6;
	int spu_tmp;
	unsigned int ck;
	va_list va;

	va_start(va, count);
	spu_tmp = va_arg(va, u32);
	ck = va_arg(va, u32);
	va_end(va);
	switch ( count )
	{
		case 0:
			_spu_dma_mode = 1;
			_spu_RXX[717] = _spu_RXX[717] | 0x30;
			break;
		case 1:
			_spu_dma_mode = 0;
			_spu_RXX[717] = (_spu_RXX[717] & ~0x30) | 0x20;
			break;
		case 2:
			_spu_tsa[1] = spu_tmp;
			_spu_RXX[725] = spu_tmp;
			_spu_RXX[724] = (_spu_tsa[1] >> 16) & 0xFFFF;
			break;
		case 3:
			if ( _spu_dma_mode == 1 )
				_spu_FsetDelayR(1);
			else
				_spu_FsetDelayW(1);
			_spu_transfer_startaddr = spu_tmp;
			_spu_transfer_time = (ck >> 6) + ((ck & 0x3F) != 0);
			((vu32 *)0xBF8010C0)[272] = spu_tmp;
			((vu32 *)0xBF8010C4)[272] = (_spu_transfer_time << 16) | 0x10;
			v6 = 0x1000201;
			if ( _spu_dma_mode == 1 )
				v6 = 16777728;
			((vu32 *)0xBF8010C8)[272] = v6;
			break;
		default:
			break;
	}
	return 0;
}

int _spu_Fw(void *addr, u32 size)
{
	if ( _spu_transMode )
	{
		_spu_FwriteByIO(addr, size);
	}
	else
	{
		_spu_t(2, _spu_tsa[1]);
		_spu_t(1);
		_spu_t(3, addr, size);
	}
	return size;
}

int _spu_StopAutoDMA(void)
{
#ifdef LIB_1300
	*((vu16 *)0xBF90019A) &= ~0x30;
#else
	int v0;

	v0 = 0;
#ifdef LIB_1600
	if ( *((vu16 *)0xBF9001B0) )
		v0 = *((vu32 *)0xBF8010C0);
	*((vu32 *)0xBF8010C8) &= ~0x1000000u;
#else
	int do_set_dmacoreindex;
	u16 *v2;

	do_set_dmacoreindex = 0;
	if ( (*((vu16 *)0xBF9007C0) & 4) != 0 )
	{
		do_set_dmacoreindex = 1;
		*((vu16 *)0xBF9007C0) &= ~0xc0;
	}
	v2 = (u16 *)((g_DmaCoreIndex << 10) + 0xBF9001B0);
	if ( *v2 )
		v0 = *(u32 *)(1088 * g_DmaCoreIndex + 0xBF8010C0);
	*(u32 *)(1088 * g_DmaCoreIndex + 0xBF8010C8) &= ~0x1000000u;
#endif
#endif
#ifdef LIB_1600
	*((vu16 *)0xBF900198) &= ~0xf0;
	*((vu16 *)0xBF9001B0) = 0;
	*((vu16 *)0xBF90076E) = 0;
	*((vu16 *)0xBF90076C) = 0;
#else
	*(u16 *)((g_DmaCoreIndex << 10) + 0xBF900198) &= ~0xf0;
	*v2 = 0;
	*(u16 *)((g_DmaCoreIndex << 10) + 0xBF90076E) = 0;
	*(u16 *)((g_DmaCoreIndex << 10) + 0xBF90076C) = *(u16 *)((g_DmaCoreIndex << 10) + 0xBF90076E);
	if ( (*((vu16 *)0xBF9007C0) & 4) != 0 )
		*((vu16 *)0xBF9007C0) &= ~0xc4;
	if ( do_set_dmacoreindex )
		spu_do_set_DmaCoreIndex(0);
#endif
#ifdef LIB_1300
	return 0;
#else
	return (gWhichBuff << 24) | (v0 & 0xFFFFFF);
#endif
}

int _spu_AutoDMAGetStatus(void)
{
	int v0;

	v0 = 0;
#ifdef LIB_1600
	if ( *((vu16 *)0xBF9001B0) )
		v0 = *((vu32 *)0xBF8010C0);
#else
	if ( *(u16 *)((g_DmaCoreIndex << 10) + 0xBF9001B0) )
		v0 = *(u32 *)(1088 * g_DmaCoreIndex + 0xBF8010C0);
#endif
	return (gWhichBuff << 24) | (v0 & 0xFFFFFF);
}

unsigned int _spu_FwAutoDMA(u8 *addr, unsigned int size, int mode)
{
	gHostAddr = addr;
	gWhichBuff = 0;
	gBufferSize48 = size;
	gMode = mode;
#ifdef LIB_1600
	*((vu16 *)0xBF90019A) &= ~0x30;
	*((vu16 *)0xBF9001A8) = 0;
	*((vu16 *)0xBF9001AA) = 0;
	*((vu16 *)0xBF9001B0) = 1;
	*((vu32 *)0xBF8010C0) = (u32)addr;
	*((vu32 *)0xBF8010C4) = 16;
	*((vu16 *)0xBF8010C6) = 8 * ((int)size / 512);
	*((vu32 *)0xBF8010C8) = 0x1000201;
	*((vu16 *)0xBF900198) |= 0xC0u;
	*((vu16 *)0xBF90076E) = 0x7FFF;
	*((vu16 *)0xBF90076C) = 0x7FFF;
#else
	if ( (mode & (SPU_AUTODMA_BIT4 | SPU_AUTODMA_LOOP)) != 0 )
	{
		spu_do_set_DmaCoreIndex(1);
		*((vu16 *)0xBF9007C0) |= 4u;
	}
	*(u16 *)((g_DmaCoreIndex << 10) + 0xBF90019A) &= ~0x30;
	*(u16 *)((g_DmaCoreIndex << 10) + 0xBF9001A8) = 0;
	*(u16 *)((g_DmaCoreIndex << 10) + 0xBF9001AA) = 0;
	if ( g_DmaCoreIndex != 0 )
		*(u16 *)((g_DmaCoreIndex << 10) + 0xBF9001B0) = 2;
	else
		*(u16 *)((g_DmaCoreIndex << 10) + 0xBF9001B0) = 1;
	_spu_FsetDelayW(g_DmaCoreIndex);
	*(u32 *)((1088 * g_DmaCoreIndex) + 0xBF8010C0) = (u32)addr;
	*(u16 *)((1088 * g_DmaCoreIndex) + 0xBF8010C4) = 16;
	*(u16 *)((1088 * g_DmaCoreIndex) + 0xBF8010C6) = 8 * ((int)size / 512);
	if ( (mode & (SPU_AUTODMA_BIT4 | SPU_AUTODMA_LOOP)) != 0 )
	{
		*((vu16 *)0xBF9007C0) |= (mode & SPU_AUTODMA_BIT6) | (mode & SPU_AUTODMA_BIT7);
	}
	*(u32 *)((1088 * g_DmaCoreIndex) + 0xBF8010C8) = 0x1000201;
	if ( (mode & SPU_AUTODMA_BIT4) == 0 )
	{
		*(u16 *)((g_DmaCoreIndex << 10) + 0xBF900198) |= 0xC0u;
		*(u16 *)((g_DmaCoreIndex << 10) + 0xBF90076E) = 0x7FFF;
		*(u16 *)((g_DmaCoreIndex << 10) + 0xBF90076C) = 0x7FFF;
	}
#endif
	return size;
}

unsigned int _spu_FwAutoDMAfrom(u8 *addr, unsigned int size, int mode, u8 *unk_a4)
{
	u8 *v4;
	int v5;
#ifdef LIB_1600
	int v7;
#endif

	v4 = unk_a4;
	if ( !unk_a4 )
		v4 = addr;
	gHostAddr = addr;
	gWhichBuff = 0;
	gBufferSize48 = size;
	gMode = mode;
	v5 = size - (v4 - addr);
	if ( (unsigned int)(v4 - addr) >= size )
	{
#ifdef LIB_1600
		if ( mode != SPU_AUTODMA_LOOP )
#else
		if ( (mode & SPU_AUTODMA_LOOP) == 0 )
#endif
			return 0;
		gWhichBuff += 1;
		v5 = size - (v4 - addr - size);
	}
	if ( v5 % 1024 > 0 )
	{
		v5 = (v5 / 1024 + 1) << 10;
		v4 = &addr[gWhichBuff * size + size - v5];
	}
#ifdef LIB_1600
	*((vu16 *)0xBF90019A) &= ~0x30;
	*((vu16 *)0xBF9001A8) = 0;
	*((vu16 *)0xBF9001AA) = 0;
	*((vu16 *)0xBF9001B0) = 1;
	*((vu32 *)0xBF8010C0) = (u32)v4;
	*((vu32 *)0xBF8010C4) = 16;
	v7 = v5 >> 6;
	if ( v5 < 0 )
		v7 = (v5 + 63) >> 6;
	*((vu16 *)0xBF8010C6) = v7 + (v5 - (v7 << 6) > 0);
	*((vu32 *)0xBF8010C8) = 0x1000201;
	*((vu16 *)0xBF900198) |= 0xC0u;
	*((vu16 *)0xBF90076E) = 0x7FFF;
	*((vu16 *)0xBF90076C) = 0x7FFF;
#else
	if ( (mode & (SPU_AUTODMA_BIT4 | SPU_AUTODMA_LOOP)) != 0 )
	{
		spu_do_set_DmaCoreIndex(1);
		*((vu16 *)0xBF9007C0) |= 4u;
	}
	*(u16 *)((g_DmaCoreIndex << 10) + 0xBF90019A) &= ~0x30;
	*(u16 *)((g_DmaCoreIndex << 10) + 0xBF9001A8) = 0;
	*(u16 *)((g_DmaCoreIndex << 10) + 0xBF9001AA) = 0;
	if ( g_DmaCoreIndex != 0 )
		*(u16 *)((g_DmaCoreIndex << 10) + 0xBF9001B0) = 2;
	else
		*(u16 *)((g_DmaCoreIndex << 10) + 0xBF9001B0) = 1;
	*(u32 *)((1088 * g_DmaCoreIndex) + 0xBF8010C0) = (u32)v4;
	*(u16 *)((1088 * g_DmaCoreIndex) + 0xBF8010C4) = 16;
	*(u16 *)((1088 * g_DmaCoreIndex) + 0xBF8010C6) = v5 / 64 + (v5 % 64 > 0);
	if ( (mode & (SPU_AUTODMA_BIT4 | SPU_AUTODMA_LOOP)) != 0 )
	{
		*((vu16 *)0xBF9007C0) |= (mode & SPU_AUTODMA_BIT6) | (mode & SPU_AUTODMA_BIT7);
	}
	*(u32 *)(1088 * g_DmaCoreIndex + 0xBF8010C8) = 0x1000201;
	if ( (mode & SPU_AUTODMA_BIT4) == 0 )
	{
		*(u16 *)((g_DmaCoreIndex << 10) + 0xBF900198) |= 0xC0u;
		*(u16 *)((g_DmaCoreIndex << 10) + 0xBF90076E) = 0x7FFF;
		*(u16 *)((g_DmaCoreIndex << 10) + 0xBF90076C) = 0x7FFF;
	}
#endif
	return size;
}

void _spu_Fr(void *addr, u32 size)
{
	_spu_t(2, _spu_tsa[1]);
	_spu_t(0);
	_spu_t(3, addr, size);
}

void _spu_MGFsetRXX2(int offset, int value)
{
	int v2;
	vu16 *v3;

	v2 = 4 * value;
	v3 = &_spu_RXX[512 * _spu_core + offset];
	*v3 = (v2 >> 16) & 0xFFFF;
	v3[1] = v2;
}

void _spu_FsetRXX(int l, u32 addr, int flag)
{
	vu16 *v3;

	v3 = &_spu_RXX[512 * _spu_core + l];
	if ( flag )
	{
		*v3 = addr >> 17;
		v3[1] = addr >> 1;
	}
	else
	{
		*v3 = addr >> 14;
		v3[1] = 4 * addr;
	}
}

int _spu_FsetRXXa(int l, u32 flag)
{
	if ( l == -2 )
		return flag;
	if ( l == -1 )
		return flag >> 1;
	_spu_RXX[512 * _spu_core + l] = flag >> 1;
	return flag;
}

int _spu_MGFgetRXX2(int offset)
{
	return 2 * (_spu_RXX[512 * _spu_core + 1 + offset] | (_spu_RXX[512 * _spu_core + offset] << 16));
}

void _spu_FsetPCR(int flag)
{
	(void)flag;
}

static void _spu_FsetDelayW(int flag)
{
	((vu32 *)0xBF801014)[256 * flag] = (((vu32 *)0xBF801014)[256 * flag] & ~0x2f000000) | 0x20000000;
}

static void _spu_FsetDelayR(int flag)
{
	((vu32 *)0xBF801014)[256 * flag] = (((vu32 *)0xBF801014)[256 * flag] & ~0x2f000000) | 0x22000000;
}

void __attribute__((optimize("no-unroll-loops"))) _spu_Fw1ts(void)
{
	int i;
	int v1;

	v1 = 13;
	for ( i = 0; i < 60; i += 1 )
	{
		v1 *= 13;
		__asm__ __volatile__("" : "+g"(v1) : :);
	}
}
