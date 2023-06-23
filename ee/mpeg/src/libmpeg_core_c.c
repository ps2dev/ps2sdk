/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2006-2007 Eugene Plotnikov <e-plotnikov@operamail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
# Based on refernce software of MSSG
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include <ee_regs.h>

#include "libmpeg.h"
#include "libmpeg_internal.h"

static u8 s_DMAPack[128];
static u32 s_DataBuf[2];
static int ( * s_SetDMA_func) ( void* );
static void * s_SetDMA_arg;
static u32 s_IPUState[8];
static int* s_pEOF;
static int s_Sema;
static u32 s_CSCParam[3];
static int s_CSCID;
static u8 s_CSCFlag;

extern s32 _mpeg_dmac_handler( s32 channel, void *arg, void *addr );

void _MPEG_Initialize ( _MPEGContext* arg0, int ( * arg1) ( void* ), void* arg2, int* arg3)
{
	(void)arg0;

	*R_EE_IPU_CTRL = 0x40000000;
	while ((s32)*R_EE_IPU_CTRL < 0);
	*R_EE_IPU_CMD = 0;
	while ((s32)*R_EE_IPU_CTRL < 0);
	*R_EE_IPU_CTRL |= 0x800000;
	*R_EE_D3_QWC = 0;
	*R_EE_D4_QWC = 0;
	s_SetDMA_func = arg1;
	s_SetDMA_arg = arg2;
	s_pEOF = arg3;
	*s_pEOF = 0;
	// TODO: check if this is the correct options for the semaphore
	ee_sema_t sema;
	memset(&sema, 0, sizeof(sema));
	sema.init_count = 0;
	sema.max_count = 1;
	sema.option = 0;
	s_Sema = CreateSema(&sema);
	s_CSCID = AddDmacHandler2(3, _mpeg_dmac_handler, 0, &s_CSCParam);
	s_DataBuf[0] = 0;
	s_DataBuf[1] = 0;
}

void _MPEG_Destroy ( void )
{
	while (s_CSCFlag != 0);
	RemoveDmacHandler(3, s_CSCID);
	DeleteSema(s_Sema);
}

void _ipu_suspend ( void )
{
	int eie;
	do
	{
		DI();
		EE_SYNCP();
		asm volatile ("mfc0\t%0, $12" : "=r" (eie));
		eie &= 0x10000;
	}
	while (eie != 0);
	*R_EE_D4_CHCR &= ~0x100;
	*R_EE_D_ENABLEW = *R_EE_D_ENABLER & ~0x100;
	EI();
	s_IPUState[0] = *R_EE_D4_CHCR;
	s_IPUState[1] = *R_EE_D4_MADR;
	s_IPUState[2] = *R_EE_D4_QWC;
	while ((*R_EE_IPU_CTRL & 0xf0) != 0);
	do
	{
		DI();
		EE_SYNCP();
		asm volatile ("mfc0\t%0, $12" : "=r" (eie));
		eie &= 0x10000;
	}
	while (eie != 0);
	*R_EE_D3_CHCR &= ~0x100;
	*R_EE_D_ENABLEW = *R_EE_D_ENABLER & ~0x100;
	EI();
	s_IPUState[3] = *R_EE_D3_CHCR;
	s_IPUState[4] = *R_EE_D3_MADR;
	s_IPUState[5] = *R_EE_D3_QWC;
	s_IPUState[6] = *R_EE_IPU_CTRL;
	s_IPUState[7] = *R_EE_IPU_BP;
}

void _MPEG_Suspend ( void )
{
	while (s_CSCFlag != 0);
	return _ipu_suspend();
}

void _ipu_resume ( void )
{
	if (s_IPUState[5] != 0)
	{
		*R_EE_D3_MADR = s_IPUState[4];
		*R_EE_D3_QWC = s_IPUState[5];
		*R_EE_D3_CHCR = s_IPUState[3] | 0x100;
	}
	u32 var2 = (s_IPUState[7] >> 0x10 & 3) + (s_IPUState[7] >> 8 & 0xf);
	u32 var3 = (s_IPUState[2]) + var2;
	if (var3 != 0)
	{
		*R_EE_IPU_CMD = (s_IPUState[7]) & 0x7f;
		while (((*R_EE_IPU_CTRL) & 0x80000000) != 0);
		*R_EE_IPU_CTRL = s_IPUState[6];
		*R_EE_D4_MADR = (s_IPUState[1]) - var2 * 0x10;
		*R_EE_D4_QWC = var3;
		*R_EE_D4_CHCR = s_IPUState[0] | 0x100;
	}
}

void _MPEG_Resume ( void )
{
	return _ipu_resume();
}

s32 _mpeg_dmac_handler( s32 channel, void *arg, void *addr )
{
	(void)channel;
	(void)addr;

	u32 *carg = arg;
	u32 var1 = carg[2];
	if (var1 == 0)
	{
		iDisableDmac(3);
		iSignalSema(s_Sema);
		s_CSCFlag = 0;
		return ~0;
	}
	u32 var2 = var1;
	if (0x3fe < (int)var1)
	{
		var2 = 0x3ff;
	}
	*R_EE_D3_MADR = carg[1];
	*R_EE_D4_MADR = carg[0];
	carg[0] += var2 * 0x180;
	carg[1] += var2 * 0x400;
	carg[2] = var1 - var2;
	*R_EE_D3_QWC = var2 * 0x400 >> 4;
	*R_EE_D4_QWC = var2 * 0x180 >> 4;
	*R_EE_D4_CHCR = 0x101;
	*R_EE_IPU_CMD = var2 | 0x70000000;
	*R_EE_D3_CHCR = 0x100;
	return ~0;
}

int _MPEG_CSCImage ( void* arg0, void* arg1, int arg2 )
{
	_ipu_suspend();
	*R_EE_IPU_CMD = 0;
	*R_EE_D_STAT = 8;
	int var1 = arg2;
	if (0x3fe < var1)
	{
		var1 = 0x3ff;
	}
	s_CSCParam[2] = arg2 - var1;
	*R_EE_D3_MADR = (u32)arg1;
	*R_EE_D4_MADR = (u32)arg0;
	s_CSCParam[0] = (int)arg0 + var1 * 0x180;
	s_CSCParam[1] = (int)arg1 + var1 * 0x400;
	*R_EE_D4_QWC = var1 * 0x180;
	*R_EE_D3_QWC = var1 * 0x400;
	EnableDmac(3);
	var1 |= 0x70000000;
	*R_EE_D4_CHCR = 0x101;
	*R_EE_IPU_CMD = var1;
	*R_EE_D3_CHCR = 0x100;
	s_CSCFlag = 68; // TODO: validate this
	WaitSema(s_Sema);
	_ipu_resume();
	return var1;
}

void _ipu_sync( void )
{
	if ((s32)*R_EE_IPU_CTRL >= 0)
	{
		return;
	}
	while (1)
	{
		while (1)
		{
			if ((*R_EE_IPU_CTRL & 0x4000) != 0)
			{
				return;
			}
			u32 var0 = *R_EE_IPU_BP;
			if ((int)((((var0 & 0xff00) >> 1) + ((var0 & 0x30000) >> 9)) - (var0 & 0x7f)) < 0x20)
			{
				break;
			}
LAB_0001041c:
			if ((u32)(-1) < *R_EE_IPU_CTRL)
			{
				return;
			}
		}

		if ((int)*R_EE_D4_QWC < 1)
		{
			if (s_SetDMA_func(s_SetDMA_arg) == 0)
			{
				*s_pEOF = 0x20;
				s_DataBuf[0] = 0x20;
				s_DataBuf[1] = 0x1b7;
			}
			goto LAB_0001041c;
		}
	}
}

u32 _ipu_sync_data( void )
{
	if ((u64)(-1) < *R_EE_IPU_CMD)
	{
		return *R_EE_IPU_BP;
	}
	u32 var3 = *R_EE_IPU_BP;
	do
	{
		while (0x1f < (((var3 & 0xff00) >> 1) + ((var3 & 0x30000) >> 9)) - (var3 & 0x7f))
		{
LAB_000104b8:
			if ((u64)(-1) < *R_EE_IPU_CMD)
			{
				return var3;
			}
			var3 = *R_EE_IPU_BP;
		}
		if (*R_EE_D4_QWC < 1)
		{
			if (s_SetDMA_func(s_SetDMA_arg) == 0)
			{
				*s_pEOF = 0x20;
				return var3;
			}
			goto LAB_000104b8;
		}
		var3 = *R_EE_IPU_BP;
	}
	while (1);
}

unsigned int _ipu_get_bits( unsigned int arg0 )
{
	_ipu_sync();
	if (s_DataBuf[0] < arg0)
	{
		*R_EE_IPU_CMD = 0x40000000;
		s_DataBuf[1] = _ipu_sync_data();
		s_DataBuf[0] = 0x20;
	}
	*R_EE_IPU_CMD = arg0 | 0x40000000;
	u32 var3 = s_DataBuf[1] >> (-arg0 & 0x1f);
	s_DataBuf[0] = s_DataBuf[0] - arg0;
	s_DataBuf[1] = s_DataBuf[1] << (arg0 & 0x1f);
	return var3;
}

unsigned int _MPEG_GetBits ( unsigned int arg0 )
{
	return _ipu_get_bits(arg0);
}

unsigned int _ipu_show_bits ( unsigned int arg0 )
{
	if (s_DataBuf[0] < arg0)
	{
		_ipu_sync();
		*R_EE_IPU_CMD = 0x40000000;
		s_DataBuf[1] = _ipu_sync_data();
		s_DataBuf[0] = 0x20;
	}
	return s_DataBuf[1] >> (-arg0 & 0x1f);
}

unsigned int _MPEG_ShowBits ( unsigned int arg0 )
{
	return _ipu_show_bits(arg0);
}

void _ipu_align_bits( void )
{
	_ipu_sync();
	u32 var3 = -(*R_EE_IPU_BP & 7) & 7;
	if (var3 != 0)
	{
		_MPEG_GetBits(var3);
	}
}

void _MPEG_AlignBits ( void )
{
	return _ipu_align_bits();
}

unsigned int _MPEG_NextStartCode ( void )
{
	_MPEG_AlignBits();
	while (_MPEG_ShowBits(0x18) != 1)
	{
		_MPEG_GetBits(8);
	}
	return _MPEG_ShowBits(0x20);
}

void _MPEG_SetDefQM ( int arg0 )
{
	(void)arg0;

	_ipu_suspend();
	*R_EE_IPU_CMD = 0;
	while (((*R_EE_IPU_CTRL) & 0x80000000) != 0);
	R_EE_IPU_in_FIFO[0] = 0x13101008;
	R_EE_IPU_in_FIFO[1] = 0x16161310;
	R_EE_IPU_in_FIFO[2] = 0x16161616;
	R_EE_IPU_in_FIFO[3] = 0x1B1A181A;
	R_EE_IPU_in_FIFO[0] = 0x1A1A1B1B;
	R_EE_IPU_in_FIFO[1] = 0x1B1B1A1A;
	R_EE_IPU_in_FIFO[2] = 0x1D1D1D1B;
	R_EE_IPU_in_FIFO[3] = 0x1D222222;
	R_EE_IPU_in_FIFO[0] = 0x1B1B1D1D;
	R_EE_IPU_in_FIFO[1] = 0x20201D1D;
	R_EE_IPU_in_FIFO[2] = 0x26252222;
	R_EE_IPU_in_FIFO[3] = 0x22232325;
	R_EE_IPU_in_FIFO[0] = 0x28262623;
	R_EE_IPU_in_FIFO[1] = 0x30302828;
	R_EE_IPU_in_FIFO[2] = 0x38382E2E;
	R_EE_IPU_in_FIFO[3] = 0x5345453A;
	*R_EE_IPU_CMD = 0x50000000;
	while (((*R_EE_IPU_CTRL) & 0x80000000) != 0);
	R_EE_IPU_in_FIFO[0] = 0x10101010;
	R_EE_IPU_in_FIFO[1] = 0x10101010;
	R_EE_IPU_in_FIFO[2] = 0x10101010;
	R_EE_IPU_in_FIFO[3] = 0x10101010;
	R_EE_IPU_in_FIFO[0] = 0x10101010;
	R_EE_IPU_in_FIFO[1] = 0x10101010;
	R_EE_IPU_in_FIFO[2] = 0x10101010;
	R_EE_IPU_in_FIFO[3] = 0x10101010;
	R_EE_IPU_in_FIFO[0] = 0x10101010;
	R_EE_IPU_in_FIFO[1] = 0x10101010;
	R_EE_IPU_in_FIFO[2] = 0x10101010;
	R_EE_IPU_in_FIFO[3] = 0x10101010;
	R_EE_IPU_in_FIFO[0] = 0x10101010;
	R_EE_IPU_in_FIFO[1] = 0x10101010;
	R_EE_IPU_in_FIFO[2] = 0x10101010;
	R_EE_IPU_in_FIFO[3] = 0x10101010;
	*R_EE_IPU_CMD = 0x58000000;
	while (((*R_EE_IPU_CTRL) & 0x80000000) != 0);
	_MPEG_Resume();
}

void _MPEG_SetQM ( int arg0 )
{
	_ipu_sync();
	*R_EE_IPU_CMD = arg0 << 0x1b | 0x50000000;
	s_DataBuf[0] = 0;
}

int _MPEG_GetMBAI ( void )
{
	_ipu_sync();
	int var5 = 0;
	u32 var4 = 0;
	while (1)
	{
		*R_EE_IPU_CMD = 0x30000000;
		var4 = _ipu_sync_data();
		var4 &= 0xffff;
		if (var4 == 0)
		{
			return 0;
		}
		if (var4 < 0x22)
		{
			break;
		}
		if (var4 == 0x23)
		{
			var5 += 0x21;
		}
	}
	s_DataBuf[0] = 0x20;
	s_DataBuf[1] = *R_EE_IPU_TOP;
	return var5 + (int)var4;
}

int _MPEG_GetMBType ( void )
{
	_ipu_sync();
	*R_EE_IPU_CMD = 0x34000000;
	u32 var4 = _ipu_sync_data();
	if (var4 != 0)
	{
		var4 &= 0xffff;
		s_DataBuf[0] = 0x20;
		s_DataBuf[1] = *R_EE_IPU_TOP;
	}
	return (int)var4;
}

int _MPEG_GetMotionCode ( void )
{
	_ipu_sync();
	*R_EE_IPU_CMD = 0x38000000;
	u32 var4 = _ipu_sync_data();
	if (var4 == 0)
	{
		var4 = 0x8000;
	}
	else
	{
		var4 &= 0xffff;
		s_DataBuf[0] = 0x20;
		s_DataBuf[1] = *R_EE_IPU_TOP;
	}
	return (int)var4;
}

int _MPEG_GetDMVector ( void )
{
	_ipu_sync();
	*R_EE_IPU_CMD = 0x3c000000;
	u32 var4 = _ipu_sync_data();
	var4 &= 0xffff;
	s_DataBuf[0] = 0x20;
	s_DataBuf[1] = *R_EE_IPU_TOP;
	return (int)var4;
}

void _MPEG_SetIDCP ( void )
{
	unsigned int var1 = _MPEG_GetBits(2);
	*R_EE_IPU_CTRL = (*R_EE_IPU_CTRL & ~0x30000) | var1 << 0x10;
}

void _MPEG_SetQSTIVFAS ( void )
{
	unsigned int var1 = _MPEG_GetBits(1);
	unsigned int var2 = _MPEG_GetBits(1);
	unsigned int var3 = _MPEG_GetBits(1);
	*R_EE_IPU_CTRL = (*R_EE_IPU_CTRL & ~0x700000) | var1 << 0x16 | var2 << 0x15 | var3 << 0x14;
}

void _MPEG_SetPCT ( unsigned int arg0 )
{
	u32 var3 = *R_EE_IPU_CTRL;
	if (-1 < (int)var3)
	{
		*R_EE_IPU_CTRL = (var3 & ~0x7000000) | arg0 << 0x18;
		return;
	}
	// TODO: validate. Bugged and in wrong place?
	_ipu_sync();
}

void _MPEG_BDEC ( int arg0, int arg1, int arg2, int arg3, void* arg4 )
{
	*R_EE_D3_MADR = ((uint32_t)arg4 & ~0xf0000000) | 0x80000000;
	*R_EE_D3_QWC = 0x30;
	*R_EE_D3_CHCR = 0x100;
	_ipu_sync();
	*R_EE_IPU_CMD = arg0 << 0x1b | 0x20000000 | arg1 << 0x1a | arg2 << 0x19 | arg3 << 0x10;
}

int _MPEG_WaitBDEC ( void )
{
	while (1)
	{
		_ipu_sync();
		if ((*s_pEOF != 0))
		{
			break;
		}
		u32 var1 = *R_EE_D3_QWC;
		if ((*R_EE_IPU_CTRL & 0x4000) != 0)
		{
			break;
		}
		if (var1 == 0)
		{
			s_DataBuf[0] = 0x20;
			s_DataBuf[1] = *R_EE_IPU_TOP;
			return 1;
		}
	}
	_ipu_suspend();
	// XXX: $t1 is not set in this function, so probably from another function?
	*R_EE_IPU_CTRL = 0x40000000;
	_ipu_resume();
	int eie;
	do
	{
		DI();
		EE_SYNCP();
		asm volatile ("mfc0\t%0, $12" : "=r" (eie));
		eie &= 0x10000;
	}
	while (eie != 0);
	*R_EE_D_ENABLEW = *R_EE_D_ENABLER | 0x10000;
	*R_EE_D3_CHCR = 0;
	*R_EE_D_ENABLEW = *R_EE_D_ENABLER & ~0x10000;
	EI();
	*R_EE_D3_QWC = 0;
	s_DataBuf[0] = 0;
	s_DataBuf[1] = 0;
	return 0;
}

void _MPEG_dma_ref_image ( _MPEGMacroBlock8* arg0, _MPEGMotion* arg1, s64 arg2, int arg3 )
{
	u8* var00 = (u8*)arg0;
	_MPEGMotion* var01 = (_MPEGMotion*)arg1;
	u32 var3 = 4;
	if (arg2 < 5)
	{
		var3 = arg2;
	}
	u64 var5 = (uint64_t)var3;
	if (arg2 >> 0x1f < 1)
	{
		// TODO: correct implementation of CONCAT44?
		var5 = ((u64)(arg2 >> 0x1f) << 32) | var3;
	}
	if (0 < var5)
	{
		while ((*R_EE_D9_CHCR & 0x100) != 0);
		*R_EE_D9_QWC = 0;
		*R_EE_D9_SADR = (u32)arg0 & ~0xf0000000;
		*R_EE_D9_SADR = (u32)&s_DMAPack;
		u32 *var2 = (u32 *)((u32)&s_DMAPack | 0x20000000);
		u32 *var6;
		do
		{
			var6 = var2;
			u8 * var1 = var01->m_pSrc;
			var5 -= 1;
			var6[0] = 0x30000030;
			var6[1] = (u32)var1;
			var6[4] = 0x30000030;
			var6[5] = (u32)var1 + arg3 * 0x180;
			var01->m_pSrc = var00;
			var00 += 4;
			var2 = var6 + 2; // + 8 bytewise
			var01 += 1;
		}
		while (var5 != 0);
		var6[4] = 0x30;
		var01 += 1;
		var01->MC_Luma = (void *)0x0;
		EE_SYNCL();
		*R_EE_D9_CHCR = 0x105;
	}
}

void _MPEG_put_block_fr(_MPEGMotions *a1)
{
	u8 *m_pMBDstY;
	u8 *m_pSrc;
	int count;
	u128 reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8, reg9;

	__asm__
	(
		"pnor    %[reg1], $zero, $zero\n"
		: [reg1] "=r"(reg1)
	);
	m_pMBDstY = a1->m_pMBDstY;
	m_pSrc = a1->m_pSrc;
	count = 6;
	__asm__
	(
		"psrlh   %[reg1],  %[reg1], 8\n"
		: [reg1] "+r"(reg1)
	);
	do
	{
		reg2 = ((u128 *)m_pSrc)[0];
		reg9 = ((u128 *)m_pSrc)[1];
		reg8 = ((u128 *)m_pSrc)[2];
		reg7 = ((u128 *)m_pSrc)[3];
		count -= 1;
		reg3 = ((u128 *)m_pSrc)[4];
		reg4 = ((u128 *)m_pSrc)[5];
		reg5 = ((u128 *)m_pSrc)[6];
		reg6 = ((u128 *)m_pSrc)[7];
		m_pSrc += 128;
		__asm__
		(
			"pmaxh   %[reg2], $zero, %[reg2]\n"
			"pmaxh   %[reg9], $zero, %[reg9]\n"
			"pmaxh   %[reg8], $zero, %[reg8]\n"
			"pmaxh   %[reg7], $zero, %[reg7]\n"
			"pmaxh   %[reg3], $zero, %[reg3]\n"
			"pmaxh   %[reg4], $zero, %[reg4]\n"
			"pmaxh   %[reg5], $zero, %[reg5]\n"
			"pmaxh   %[reg6], $zero, %[reg6]\n"
			"pminh   %[reg2], %[reg1], %[reg2]\n"
			"pminh   %[reg9], %[reg1], %[reg9]\n"
			"pminh   %[reg8], %[reg1], %[reg8]\n"
			"pminh   %[reg7], %[reg1], %[reg7]\n"
			"pminh   %[reg3], %[reg1], %[reg3]\n"
			"pminh   %[reg4], %[reg1], %[reg4]\n"
			"pminh   %[reg5], %[reg1], %[reg5]\n"
			"pminh   %[reg6], %[reg1], %[reg6]\n"
			"ppacb   %[reg2], %[reg9], %[reg2]\n"
			"ppacb   %[reg8], %[reg7], %[reg8]\n"
			"ppacb   %[reg3], %[reg4], %[reg3]\n"
			"ppacb   %[reg5], %[reg6], %[reg5]\n"
			: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			: [reg1] "r"(reg1)
		);
		((u128 *)m_pMBDstY)[0] = reg2;
		((u128 *)m_pMBDstY)[1] = reg8;
		((u128 *)m_pMBDstY)[2] = reg3;
		((u128 *)m_pMBDstY)[3] = reg5;
		m_pMBDstY += 64;
	}
	while ( count > 0 );
}

void _MPEG_put_block_fl(_MPEGMotions *a1)
{
	u8 *m_pMBDstY;
	u8 *m_pSrc;
	int count;
	u128 reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8, reg9;

	__asm__
	(
		"pnor    %[reg1], $zero, $zero\n"
		: [reg1] "=r"(reg1)
	);
	m_pMBDstY = a1->m_pMBDstY;
	m_pSrc = a1->m_pSrc;
	count = 4;
	__asm__
	(
		"psrlh   %[reg1], %[reg1], 8\n"
		: [reg1] "+r"(reg1)
	);
	do
	{
		reg2 = ((u128 *)m_pSrc)[0x00];
		reg9 = ((u128 *)m_pSrc)[0x01];
		reg8 = ((u128 *)m_pSrc)[0x02];
		reg7 = ((u128 *)m_pSrc)[0x03];
		count -= 1;
		reg3 = ((u128 *)m_pSrc)[0x10];
		reg4 = ((u128 *)m_pSrc)[0x11];
		reg5 = ((u128 *)m_pSrc)[0x12];
		reg6 = ((u128 *)m_pSrc)[0x13];
		m_pSrc += 64;
		__asm__
		(
			"pmaxh   %[reg2], $zero, %[reg2]\n"
			"pmaxh   %[reg9], $zero, %[reg9]\n"
			"pmaxh   %[reg8], $zero, %[reg8]\n"
			"pmaxh   %[reg7], $zero, %[reg7]\n"
			"pmaxh   %[reg3], $zero, %[reg3]\n"
			"pmaxh   %[reg4], $zero, %[reg4]\n"
			"pmaxh   %[reg5], $zero, %[reg5]\n"
			"pmaxh   %[reg6], $zero, %[reg6]\n"
			"pminh   %[reg2], %[reg1], %[reg2]\n"
			"pminh   %[reg9], %[reg1], %[reg9]\n"
			"pminh   %[reg8], %[reg1], %[reg8]\n"
			"pminh   %[reg7], %[reg1], %[reg7]\n"
			"pminh   %[reg3], %[reg1], %[reg3]\n"
			"pminh   %[reg4], %[reg1], %[reg4]\n"
			"pminh   %[reg5], %[reg1], %[reg5]\n"
			"pminh   %[reg6], %[reg1], %[reg6]\n"
			"ppacb   %[reg2], %[reg9], %[reg2]\n"
			"ppacb   %[reg8], %[reg7], %[reg8]\n"
			"ppacb   %[reg3], %[reg4], %[reg3]\n"
			"ppacb   %[reg5], %[reg6], %[reg5]\n"
			: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			: [reg1] "r"(reg1)
		);
		((u128 *)m_pMBDstY)[0] = reg2;
		((u128 *)m_pMBDstY)[1] = reg3;
		((u128 *)m_pMBDstY)[2] = reg8;
		((u128 *)m_pMBDstY)[3] = reg5;
		m_pMBDstY += 64;
	}
	while ( count > 0 );
	count += 2;
	do
	{
		reg2 = ((u128 *)m_pSrc)[0x10];
		reg9 = ((u128 *)m_pSrc)[0x11];
		reg8 = ((u128 *)m_pSrc)[0x12];
		reg7 = ((u128 *)m_pSrc)[0x13];
		count -= 1;
		reg3 = ((u128 *)m_pSrc)[0x14];
		reg4 = ((u128 *)m_pSrc)[0x15];
		reg5 = ((u128 *)m_pSrc)[0x16];
		reg6 = ((u128 *)m_pSrc)[0x17];
		m_pSrc += 128;
		__asm__
		(
			"pmaxh   %[reg2], $zero, %[reg2]\n"
			"pmaxh   %[reg9], $zero, %[reg9]\n"
			"pmaxh   %[reg8], $zero, %[reg8]\n"
			"pmaxh   %[reg7], $zero, %[reg7]\n"
			"pmaxh   %[reg3], $zero, %[reg3]\n"
			"pmaxh   %[reg4], $zero, %[reg4]\n"
			"pmaxh   %[reg5], $zero, %[reg5]\n"
			"pmaxh   %[reg6], $zero, %[reg6]\n"
			"pminh   %[reg2], %[reg1], %[reg2]\n"
			"pminh   %[reg9], %[reg1], %[reg9]\n"
			"pminh   %[reg8], %[reg1], %[reg8]\n"
			"pminh   %[reg7], %[reg1], %[reg7]\n"
			"pminh   %[reg3], %[reg1], %[reg3]\n"
			"pminh   %[reg4], %[reg1], %[reg4]\n"
			"pminh   %[reg5], %[reg1], %[reg5]\n"
			"pminh   %[reg6], %[reg1], %[reg6]\n"
			"ppacb   %[reg2], %[reg9], %[reg2]\n"
			"ppacb   %[reg8], %[reg7], %[reg8]\n"
			"ppacb   %[reg3], %[reg4], %[reg3]\n"
			"ppacb   %[reg5], %[reg6], %[reg5]\n"
			: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			: [reg1] "r"(reg1)
		);
		((u128 *)m_pMBDstY)[0] = reg2;
		((u128 *)m_pMBDstY)[1] = reg8;
		((u128 *)m_pMBDstY)[2] = reg3;
		((u128 *)m_pMBDstY)[3] = reg5;
		m_pMBDstY += 64;
	}
	while ( count > 0 );
}

void _MPEG_put_block_il(_MPEGMotions *a1)
{
	u8 *m_pMBDstY;
	u8 *m_pSrc;
	int count;
	u8 *m_pMBDstCbCr;
	u8 *v28;
	u128 reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8, reg9;

	__asm__
	(
		"pnor    %[reg1], $zero, $zero\n"
		: [reg1] "=r"(reg1)
	);
	m_pMBDstY = a1->m_pMBDstY;
	m_pSrc = a1->m_pSrc;
	count = 4;
	v28 = &m_pMBDstY[a1->m_Stride];
	__asm__
	(
		"psrlh   %[reg1], %[reg1], 8\n"
		: [reg1] "+r"(reg1)
	);
	do
	{
		reg2 = ((u128 *)m_pSrc)[0x00];
		reg9 = ((u128 *)m_pSrc)[0x01];
		reg8 = ((u128 *)m_pSrc)[0x02];
		reg7 = ((u128 *)m_pSrc)[0x03];
		count -= 1;
		reg3 = ((u128 *)m_pSrc)[0x10];
		reg4 = ((u128 *)m_pSrc)[0x11];
		reg5 = ((u128 *)m_pSrc)[0x12];
		reg6 = ((u128 *)m_pSrc)[0x13];
		m_pSrc += 64;
		__asm__
		(
			"pmaxh   %[reg2], $zero, %[reg2]\n"
			"pmaxh   %[reg9], $zero, %[reg9]\n"
			"pmaxh   %[reg8], $zero, %[reg8]\n"
			"pmaxh   %[reg7], $zero, %[reg7]\n"
			"pmaxh   %[reg3], $zero, %[reg3]\n"
			"pmaxh   %[reg4], $zero, %[reg4]\n"
			"pmaxh   %[reg5], $zero, %[reg5]\n"
			"pmaxh   %[reg6], $zero, %[reg6]\n"
			"pminh   %[reg2], %[reg1], %[reg2]\n"
			"pminh   %[reg9], %[reg1], %[reg9]\n"
			"pminh   %[reg8], %[reg1], %[reg8]\n"
			"pminh   %[reg7], %[reg1], %[reg7]\n"
			"pminh   %[reg3], %[reg1], %[reg3]\n"
			"pminh   %[reg4], %[reg1], %[reg4]\n"
			"pminh   %[reg5], %[reg1], %[reg5]\n"
			"pminh   %[reg6], %[reg1], %[reg6]\n"
			"ppacb   %[reg2], %[reg9], %[reg2]\n"
			"ppacb   %[reg8], %[reg7], %[reg8]\n"
			"ppacb   %[reg3], %[reg4], %[reg3]\n"
			"ppacb   %[reg5], %[reg6], %[reg5]\n"
			: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			: [reg1] "r"(reg1)
		);
		((u128 *)m_pMBDstY)[0x00] = reg2;
		((u128 *)m_pMBDstY)[0x02] = reg8;
		m_pMBDstY += 64;
		((u128 *)v28)[0x00] = reg3;
		((u128 *)v28)[0x02] = reg5;
		v28 += 64;
	}
	while ( count > 0 );
	m_pMBDstCbCr = a1->m_pMBDstCbCr;
	count = 2;
	v28 = &m_pMBDstCbCr[a1->m_Stride];
	do
	{
		reg2 = ((u128 *)m_pSrc)[0x10];
		reg9 = ((u128 *)m_pSrc)[0x11];
		reg8 = ((u128 *)m_pSrc)[0x12];
		reg7 = ((u128 *)m_pSrc)[0x13];
		count -= 1;
		reg3 = ((u128 *)m_pSrc)[0x14];
		reg4 = ((u128 *)m_pSrc)[0x15];
		reg5 = ((u128 *)m_pSrc)[0x16];
		reg6 = ((u128 *)m_pSrc)[0x17];
		m_pSrc += 128;
		__asm__
		(
			"pmaxh   %[reg2], $zero, %[reg2]\n"
			"pmaxh   %[reg9], $zero, %[reg9]\n"
			"pmaxh   %[reg8], $zero, %[reg8]\n"
			"pmaxh   %[reg7], $zero, %[reg7]\n"
			"pmaxh   %[reg3], $zero, %[reg3]\n"
			"pmaxh   %[reg4], $zero, %[reg4]\n"
			"pmaxh   %[reg5], $zero, %[reg5]\n"
			"pmaxh   %[reg6], $zero, %[reg6]\n"
			"pminh   %[reg2], %[reg1], %[reg2]\n"
			"pminh   %[reg9], %[reg1], %[reg9]\n"
			"pminh   %[reg8], %[reg1], %[reg8]\n"
			"pminh   %[reg7], %[reg1], %[reg7]\n"
			"pminh   %[reg3], %[reg1], %[reg3]\n"
			"pminh   %[reg4], %[reg1], %[reg4]\n"
			"pminh   %[reg5], %[reg1], %[reg5]\n"
			"pminh   %[reg6], %[reg1], %[reg6]\n"
			"ppacb   %[reg2], $zero, %[reg2]\n"
			"ppacb   %[reg9], $zero, %[reg9]\n"
			"ppacb   %[reg8], $zero, %[reg8]\n"
			"ppacb   %[reg7], $zero, %[reg7]\n"
			"ppacb   %[reg3], $zero, %[reg3]\n"
			"ppacb   %[reg4], $zero, %[reg4]\n"
			"ppacb   %[reg5], $zero, %[reg5]\n"
			"ppacb   %[reg6], $zero, %[reg6]\n"
			: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			: [reg1] "r"(reg1)
		);
		((u64 *)m_pMBDstCbCr)[0x00] = reg2;
		((u64 *)m_pMBDstCbCr)[0x02] = reg9;
		((u64 *)m_pMBDstCbCr)[0x04] = reg8;
		((u64 *)m_pMBDstCbCr)[0x06] = reg7;
		((u64 *)v28)[0x00] = reg3;
		((u64 *)v28)[0x02] = reg4;
		((u64 *)v28)[0x04] = reg5;
		((u64 *)v28)[0x06] = reg6;
		m_pMBDstCbCr += 64;
		v28 += 64;
	}
	while ( count > 0 );
}

void _MPEG_add_block_frfr(_MPEGMotions *a1)
{
	u8 *m_pMBDstY;
	u8 *m_pSPRBlk;
	u8 *m_pSPRRes;
	int count;
	u128 reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8, reg9;

	__asm__
	(
		"pnor    %[reg1], $zero, $zero\n"
		: [reg1] "=r"(reg1)
	);
	m_pMBDstY = a1->m_pMBDstY;
	m_pSPRBlk = a1->m_pSPRBlk;
	m_pSPRRes = a1->m_pSPRRes;
	count = 6;
	__asm__
	(
		"psrlh   %[reg1], %[reg1], 8\n"
		: [reg1] "+r"(reg1)
	);
	do
	{
		reg2 = ((u128 *)m_pSPRBlk)[0x00];
		reg9 = ((u128 *)m_pSPRBlk)[0x01];
		reg8 = ((u128 *)m_pSPRBlk)[0x02];
		reg7 = ((u128 *)m_pSPRBlk)[0x03];
		count -= 1;
		reg3 = ((u128 *)m_pSPRRes)[0x00];
		reg4 = ((u128 *)m_pSPRRes)[0x01];
		reg5 = ((u128 *)m_pSPRRes)[0x02];
		reg6 = ((u128 *)m_pSPRRes)[0x03];
		__asm__
		(
			"paddh   %[reg2], %[reg2], %[reg3]\n"
			"paddh   %[reg9], %[reg9], %[reg4]\n"
			"paddh   %[reg8], %[reg8], %[reg5]\n"
			"paddh   %[reg7], %[reg7], %[reg6]\n"
			"pmaxh   %[reg2], $zero, %[reg2]\n"
			"pmaxh   %[reg9], $zero, %[reg9]\n"
			"pmaxh   %[reg8], $zero, %[reg8]\n"
			"pmaxh   %[reg7], $zero, %[reg7]\n"
			"pminh   %[reg2], %[reg1], %[reg2]\n"
			"pminh   %[reg9], %[reg1], %[reg9]\n"
			"pminh   %[reg8], %[reg1], %[reg8]\n"
			"pminh   %[reg7], %[reg1], %[reg7]\n"
			"ppacb   %[reg2], %[reg9], %[reg2]\n"
			"ppacb   %[reg8], %[reg7], %[reg8]\n"
			: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			: [reg1] "r"(reg1)
		);
		((u128 *)m_pMBDstY)[0x00] = reg2;
		((u128 *)m_pMBDstY)[0x01] = reg8;
		reg3 = ((u128 *)m_pSPRBlk)[0x04];
		reg4 = ((u128 *)m_pSPRBlk)[0x05];
		reg5 = ((u128 *)m_pSPRBlk)[0x06];
		reg6 = ((u128 *)m_pSPRBlk)[0x07];
		m_pSPRBlk += 128;
		reg2 = ((u128 *)m_pSPRRes)[0x04];
		reg9 = ((u128 *)m_pSPRRes)[0x05];
		reg8 = ((u128 *)m_pSPRRes)[0x06];
		reg7 = ((u128 *)m_pSPRRes)[0x07];
		m_pSPRRes += 128;
		__asm__
		(
			"paddh   %[reg3], %[reg3], %[reg2]\n"
			"paddh   %[reg4], %[reg4], %[reg9]\n"
			"paddh   %[reg5], %[reg5], %[reg8]\n"
			"paddh   %[reg6], %[reg6], %[reg7]\n"
			"pmaxh   %[reg3], $zero, %[reg3]\n"
			"pmaxh   %[reg4], $zero, %[reg4]\n"
			"pmaxh   %[reg5], $zero, %[reg5]\n"
			"pmaxh   %[reg6], $zero, %[reg6]\n"
			"pminh   %[reg3], %[reg1], %[reg3]\n"
			"pminh   %[reg4], %[reg1], %[reg4]\n"
			"pminh   %[reg5], %[reg1], %[reg5]\n"
			"pminh   %[reg6], %[reg1], %[reg6]\n"
			"ppacb   %[reg3], %[reg4], %[reg3]\n"
			"ppacb   %[reg5], %[reg6], %[reg5]\n"
			: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			: [reg1] "r"(reg1)
		);
		((u128 *)m_pMBDstY)[0x02] = reg3;
		((u128 *)m_pMBDstY)[0x03] = reg5;
		m_pMBDstY += 64;
	}
	while ( count > 0 );
}

void _MPEG_add_block_ilfl(_MPEGMotions *a1)
{
	u8 *m_pMBDstY;
	u8 *m_pSPRBlk;
	u8 *m_pSPRRes;
	int count;
	u8 *v6;
	u8 *m_pMBDstCbCr;
	u128 reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8, reg9;

	__asm__
	(
		"pnor    %[reg1], $zero, $zero\n"
		: [reg1] "=r"(reg1)
	);
	m_pMBDstY = a1->m_pMBDstY;
	m_pSPRBlk = a1->m_pSPRBlk;
	m_pSPRRes = a1->m_pSPRRes;
	count = 4;
	__asm__
	(
		"psrlh   %[reg1], %[reg1], 8\n"
		: [reg1] "+r"(reg1)
	);
	v6 = &m_pMBDstY[a1->m_Stride];
	do
	{
		reg2 = ((u128 *)m_pSPRBlk)[0x00];
		reg9 = ((u128 *)m_pSPRBlk)[0x01];
		reg8 = ((u128 *)m_pSPRBlk)[0x02];
		reg7 = ((u128 *)m_pSPRBlk)[0x03];
		count -= 1;
		reg3 = ((u128 *)m_pSPRRes)[0x00];
		reg4 = ((u128 *)m_pSPRRes)[0x01];
		reg5 = ((u128 *)m_pSPRRes)[0x02];
		reg6 = ((u128 *)m_pSPRRes)[0x03];
		__asm__
		(
			"paddh   %[reg2], %[reg2], %[reg3]\n"
			"paddh   %[reg9], %[reg9], %[reg4]\n"
			"paddh   %[reg8], %[reg8], %[reg5]\n"
			"paddh   %[reg7], %[reg7], %[reg6]\n"
			"pmaxh   %[reg2], $zero, %[reg2]\n"
			"pmaxh   %[reg9], $zero, %[reg9]\n"
			"pmaxh   %[reg8], $zero, %[reg8]\n"
			"pmaxh   %[reg7], $zero, %[reg7]\n"
			"pminh   %[reg2], %[reg1], %[reg2]\n"
			"pminh   %[reg9], %[reg1], %[reg9]\n"
			"pminh   %[reg8], %[reg1], %[reg8]\n"
			"pminh   %[reg7], %[reg1], %[reg7]\n"
			"ppacb   %[reg2], %[reg9], %[reg2]\n"
			"ppacb   %[reg8], %[reg7], %[reg8]\n"
			: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			: [reg1] "r"(reg1)
		);
		((u128 *)m_pMBDstY)[0] = reg2;
		((u128 *)m_pMBDstY)[2] = reg8;
		reg3 = ((u128 *)m_pSPRBlk)[0x10];
		reg4 = ((u128 *)m_pSPRBlk)[0x11];
		reg5 = ((u128 *)m_pSPRBlk)[0x12];
		reg6 = ((u128 *)m_pSPRBlk)[0x13];
		m_pSPRBlk += 64;
		reg2 = ((u128 *)m_pSPRRes)[0x10];
		reg9 = ((u128 *)m_pSPRRes)[0x11];
		reg8 = ((u128 *)m_pSPRRes)[0x12];
		reg7 = ((u128 *)m_pSPRRes)[0x13];
		__asm__
		(
			"paddh   %[reg3], %[reg3], %[reg2]\n"
			"paddh   %[reg4], %[reg4], %[reg9]\n"
			"paddh   %[reg5], %[reg5], %[reg8]\n"
			"paddh   %[reg6], %[reg6], %[reg7]\n"
			"pmaxh   %[reg3], $zero, %[reg3]\n"
			"pmaxh   %[reg4], $zero, %[reg4]\n"
			"pmaxh   %[reg5], $zero, %[reg5]\n"
			"pmaxh   %[reg6], $zero, %[reg6]\n"
			"pminh   %[reg3], %[reg1], %[reg3]\n"
			"pminh   %[reg4], %[reg1], %[reg4]\n"
			"pminh   %[reg5], %[reg1], %[reg5]\n"
			"pminh   %[reg6], %[reg1], %[reg6]\n"
			"ppacb   %[reg3], %[reg4], %[reg3]\n"
			"ppacb   %[reg5], %[reg6], %[reg5]\n"
			: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			: [reg1] "r"(reg1)
		);
		((u128 *)v6)[0x00] = reg3;
		((u128 *)v6)[0x02] = reg5;
		v6 += 64;
		m_pSPRRes += 64;
	}
	while ( count > 0 );
	m_pMBDstCbCr = a1->m_pMBDstCbCr;
	count = 2;
	v6 = &m_pMBDstCbCr[a1->m_Stride];
	do
	{
		reg2 = ((u128 *)m_pSPRBlk)[0x10];
		reg9 = ((u128 *)m_pSPRBlk)[0x11];
		reg8 = ((u128 *)m_pSPRBlk)[0x12];
		reg7 = ((u128 *)m_pSPRBlk)[0x13];
		count -= 1;
		reg3 = ((u128 *)m_pSPRRes)[0x10];
		reg4 = ((u128 *)m_pSPRRes)[0x11];
		reg5 = ((u128 *)m_pSPRRes)[0x12];
		reg6 = ((u128 *)m_pSPRRes)[0x13];
		__asm__
		(
			"paddh   %[reg2], %[reg2], %[reg3]\n"
			"paddh   %[reg9], %[reg9], %[reg4]\n"
			"paddh   %[reg8], %[reg8], %[reg5]\n"
			"paddh   %[reg7], %[reg7], %[reg6]\n"
			"pmaxh   %[reg2], $zero, %[reg2]\n"
			"pmaxh   %[reg9], $zero, %[reg9]\n"
			"pmaxh   %[reg8], $zero, %[reg8]\n"
			"pmaxh   %[reg7], $zero, %[reg7]\n"
			"pminh   %[reg2], %[reg1], %[reg2]\n"
			"pminh   %[reg9], %[reg1], %[reg9]\n"
			"pminh   %[reg8], %[reg1], %[reg8]\n"
			"pminh   %[reg7], %[reg1], %[reg7]\n"
			"ppacb   %[reg2], $zero, %[reg2]\n"
			"ppacb   %[reg9], $zero, %[reg9]\n"
			"ppacb   %[reg8], $zero, %[reg8]\n"
			"ppacb   %[reg7], $zero, %[reg7]\n"
			: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			: [reg1] "r"(reg1)
		);
		((u64 *)m_pMBDstCbCr)[0] = reg2;
		((u64 *)m_pMBDstCbCr)[2] = reg9;
		((u64 *)m_pMBDstCbCr)[4] = reg8;
		((u64 *)m_pMBDstCbCr)[6] = reg7;
		reg3 = ((u128 *)m_pSPRBlk)[0x14];
		reg4 = ((u128 *)m_pSPRBlk)[0x15];
		reg5 = ((u128 *)m_pSPRBlk)[0x16];
		reg6 = ((u128 *)m_pSPRBlk)[0x17];
		m_pSPRBlk += 128;
		reg2 = ((u128 *)m_pSPRRes)[0x14];
		reg9 = ((u128 *)m_pSPRRes)[0x15];
		reg8 = ((u128 *)m_pSPRRes)[0x16];
		reg7 = ((u128 *)m_pSPRRes)[0x17];
		__asm__
		(
			"paddh   %[reg3], %[reg3], %[reg2]\n"
			"paddh   %[reg4], %[reg4], %[reg9]\n"
			"paddh   %[reg5], %[reg5], %[reg8]\n"
			"paddh   %[reg6], %[reg6], %[reg7]\n"
			"pmaxh   %[reg3], $zero, %[reg3]\n"
			"pmaxh   %[reg4], $zero, %[reg4]\n"
			"pmaxh   %[reg5], $zero, %[reg5]\n"
			"pmaxh   %[reg6], $zero, %[reg6]\n"
			"pminh   %[reg3], %[reg1], %[reg3]\n"
			"pminh   %[reg4], %[reg1], %[reg4]\n"
			"pminh   %[reg5], %[reg1], %[reg5]\n"
			"pminh   %[reg6], %[reg1], %[reg6]\n"
			"ppacb   %[reg3], $zero, %[reg3]\n"
			"ppacb   %[reg4], $zero, %[reg4]\n"
			"ppacb   %[reg5], $zero, %[reg5]\n"
			"ppacb   %[reg6], $zero, %[reg6]\n"
			: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			: [reg1] "r"(reg1)
		);
		((u64 *)v6)[0x00] = reg3;
		((u64 *)v6)[0x02] = reg4;
		((u64 *)v6)[0x04] = reg5;
		((u64 *)v6)[0x06] = reg6;
		m_pMBDstCbCr += 64;
		v6 += 64;
	}
	while ( count > 0 );
}

void _MPEG_add_block_frfl(_MPEGMotions *a1)
{
	u8 *m_pSPRBlk;
	u8 *m_pSPRRes;
	int count;
	u8 *m_pMBDstCbCr;
	u8 *m_pMBDstY;
	u128 reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8, reg9;

	__asm__
	(
		"pnor    %[reg1], $zero, $zero\n"
		: [reg1] "=r"(reg1)
	);
	m_pSPRBlk = a1->m_pSPRBlk;
	m_pSPRRes = a1->m_pSPRRes;
	m_pMBDstY = a1->m_pMBDstY;
	count = 4;
	__asm__
	(
		"psrlh   %[reg1], %[reg1], 8\n"
		: [reg1] "+r"(reg1)
	);
	do
	{
		reg2 = ((u128 *)m_pSPRBlk)[0x00];
		reg9 = ((u128 *)m_pSPRBlk)[0x01];
		reg8 = ((u128 *)m_pSPRBlk)[0x02];
		reg7 = ((u128 *)m_pSPRBlk)[0x03];
		count -= 1;
		reg3 = ((u128 *)m_pSPRRes)[0x00];
		reg4 = ((u128 *)m_pSPRRes)[0x01];
		reg5 = ((u128 *)m_pSPRRes)[0x10];
		reg6 = ((u128 *)m_pSPRRes)[0x11];
		__asm__
		(
			"paddh   %[reg2], %[reg2], %[reg3]\n"
			"paddh   %[reg9], %[reg9], %[reg4]\n"
			"paddh   %[reg8], %[reg8], %[reg5]\n"
			"paddh   %[reg7], %[reg7], %[reg6]\n"
			"pmaxh   %[reg2], $zero, %[reg2]\n"
			"pmaxh   %[reg9], $zero, %[reg9]\n"
			"pmaxh   %[reg8], $zero, %[reg8]\n"
			"pmaxh   %[reg7], $zero, %[reg7]\n"
			"pminh   %[reg2], %[reg1], %[reg2]\n"
			"pminh   %[reg9], %[reg1], %[reg9]\n"
			"pminh   %[reg8], %[reg1], %[reg8]\n"
			"pminh   %[reg7], %[reg1], %[reg7]\n"
			"ppacb   %[reg2], %[reg9], %[reg2]\n"
			"ppacb   %[reg8], %[reg7], %[reg8]\n"
			: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			: [reg1] "r"(reg1)
		);
		((u128 *)m_pMBDstY)[0x00] = reg2;
		((u128 *)m_pMBDstY)[0x01] = reg8;
		reg3 = ((u128 *)m_pSPRBlk)[0x04];
		reg4 = ((u128 *)m_pSPRBlk)[0x05];
		reg5 = ((u128 *)m_pSPRBlk)[0x06];
		reg6 = ((u128 *)m_pSPRBlk)[0x07];
		m_pSPRBlk += 128;
		reg2 = ((u128 *)m_pSPRRes)[0x02];
		reg9 = ((u128 *)m_pSPRRes)[0x03];
		reg8 = ((u128 *)m_pSPRRes)[0x12];
		reg7 = ((u128 *)m_pSPRRes)[0x13];
		__asm__
		(
			"paddh   %[reg3], %[reg3], %[reg2]\n"
			"paddh   %[reg4], %[reg4], %[reg9]\n"
			"paddh   %[reg5], %[reg5], %[reg8]\n"
			"paddh   %[reg6], %[reg6], %[reg7]\n"
			"pmaxh   %[reg3], $zero, %[reg3]\n"
			"pmaxh   %[reg4], $zero, %[reg4]\n"
			"pmaxh   %[reg5], $zero, %[reg5]\n"
			"pmaxh   %[reg6], $zero, %[reg6]\n"
			"pminh   %[reg3], %[reg1], %[reg3]\n"
			"pminh   %[reg4], %[reg1], %[reg4]\n"
			"pminh   %[reg5], %[reg1], %[reg5]\n"
			"pminh   %[reg6], %[reg1], %[reg6]\n"
			"ppacb   %[reg3], %[reg4], %[reg3]\n"
			"ppacb   %[reg5], %[reg6], %[reg5]\n"
			: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			: [reg1] "r"(reg1)
		);
		((u128 *)m_pMBDstY)[0x02] = reg3;
		((u128 *)m_pMBDstY)[0x03] = reg5;
		m_pSPRRes += 64;
		m_pMBDstY += 64;
	}
	while ( count > 0 );
	m_pMBDstCbCr = a1->m_pMBDstCbCr;
	count = 2;
	do
	{
		reg2 = ((u128 *)m_pSPRBlk)[0x00];
		reg9 = ((u128 *)m_pSPRBlk)[0x01];
		reg8 = ((u128 *)m_pSPRBlk)[0x02];
		reg7 = ((u128 *)m_pSPRBlk)[0x03];
		count -= 1;
		reg3 = ((u128 *)m_pSPRRes)[0x10];
		reg4 = ((u128 *)m_pSPRRes)[0x14];
		reg5 = ((u128 *)m_pSPRRes)[0x11];
		reg6 = ((u128 *)m_pSPRRes)[0x15];
		__asm__
		(
			"paddh   %[reg2], %[reg2], %[reg3]\n"
			"paddh   %[reg9], %[reg9], %[reg4]\n"
			"paddh   %[reg8], %[reg8], %[reg5]\n"
			"paddh   %[reg7], %[reg7], %[reg6]\n"
			"pmaxh   %[reg2], $zero, %[reg2]\n"
			"pmaxh   %[reg9], $zero, %[reg9]\n"
			"pmaxh   %[reg8], $zero, %[reg8]\n"
			"pmaxh   %[reg7], $zero, %[reg7]\n"
			"pminh   %[reg2], %[reg1], %[reg2]\n"
			"pminh   %[reg9], %[reg1], %[reg9]\n"
			"pminh   %[reg8], %[reg1], %[reg8]\n"
			"pminh   %[reg7], %[reg1], %[reg7]\n"
			"ppacb   %[reg2], %[reg9], %[reg2]\n"
			"ppacb   %[reg8], %[reg7], %[reg8]\n"
			: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			: [reg1] "r"(reg1)
		);
		((u128 *)m_pMBDstCbCr)[0x00] = reg2;
		((u128 *)m_pMBDstCbCr)[0x01] = reg8;
		reg3 = ((u128 *)m_pSPRBlk)[0x04];
		reg4 = ((u128 *)m_pSPRBlk)[0x05];
		reg5 = ((u128 *)m_pSPRBlk)[0x06];
		reg6 = ((u128 *)m_pSPRBlk)[0x07];
		m_pSPRBlk += 128;
		reg2 = ((u128 *)m_pSPRRes)[0x12];
		reg9 = ((u128 *)m_pSPRRes)[0x16];
		reg8 = ((u128 *)m_pSPRRes)[0x13];
		reg7 = ((u128 *)m_pSPRRes)[0x17];
		__asm__
		(
			"paddh   %[reg3], %[reg3], %[reg2]\n"
			"paddh   %[reg4], %[reg4], %[reg9]\n"
			"paddh   %[reg5], %[reg5], %[reg8]\n"
			"paddh   %[reg6], %[reg6], %[reg7]\n"
			"pmaxh   %[reg3], $zero, %[reg3]\n"
			"pmaxh   %[reg4], $zero, %[reg4]\n"
			"pmaxh   %[reg5], $zero, %[reg5]\n"
			"pmaxh   %[reg6], $zero, %[reg6]\n"
			"pminh   %[reg3], %[reg1], %[reg3]\n"
			"pminh   %[reg4], %[reg1], %[reg4]\n"
			"pminh   %[reg5], %[reg1], %[reg5]\n"
			"pminh   %[reg6], %[reg1], %[reg6]\n"
			"ppacb   %[reg3], %[reg4], %[reg3]\n"
			"ppacb   %[reg5], %[reg6], %[reg5]\n"
			: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			: [reg1] "r"(reg1)
		);
		((u128 *)m_pMBDstCbCr)[0x02] = reg3;
		((u128 *)m_pMBDstCbCr)[0x03] = reg5;
		m_pMBDstCbCr += 64;
		m_pSPRRes += 128;
	}
	while ( count > 0 );
}

// TODO: verify delay slots
void _MPEG_do_mc ( _MPEGMotion* arg0 )
{
	int var0 = 16; // addiu   $v0, $zero, 16
	u8* arg1 = arg0->m_pSrc; // lw      $a1,  0($a0)
	// addiu   $sp, $sp, -16
	u16* arg2 = (u16 *)arg0->m_pDstY; // lw      $a2,  4($a0)
	int arg3 = arg0->m_X; // lw      $a3, 12($a0)
	int tmp0 = arg0->m_Y; // lw      $t0, 16($a0)
	int tmp1 = arg0->m_H; // lw      $t1, 20($a0)
	int tmp2 = arg0->m_fInt; // lw      $t2, 24($a0)
	int tmp4 = arg0->m_Field; // lw      $t4, 28($a0)
	// lw      $t5, 32($a0) <-- MC_Luma
	tmp0 -= tmp4; // subu    $t0, $t0, $t4
	tmp4 <<= 4; // sll     $t4, $t4, 4
	arg1 += tmp4; // addu    $a1, $a1, $t4
	int var1 = var0 - tmp0; // subu    $v1, $v0, $t0
	int tmp3 = var0 << tmp2; // sllv    $t3, $v0, $t2
	var1 >>= tmp2; // srlv    $v1, $v1, $t2
	int ta = tmp0 << 4; // sll     $at, $t0, 4
	// sw      $ra, 0($sp)
	arg1 += ta; // addu    $a1, $a1, $at
	ta = tmp1 - var1; // subu    $at, $t1, $v1
	arg0->MC_Luma(arg1, arg2, arg3, tmp3, var1, ta); // jalr    $t5
	arg1 = arg0->m_pSrc; // lw      $a1,  0($a0)
	arg2 = (u16 *)arg0->m_pDstCbCr; // lw      $a2,  8($a0)
	// lw      $t5, 36($a0) <-- MC_Chroma
	arg1 += 256; // addiu   $a1, $a1, 256
	tmp4 >>= 1; // srl     $t4, $t4, 1
	arg3 >>= 1; // srl     $a3, $a3, 1
	tmp0 >>= 1; // srl     $t0, $t0, 1
	tmp1 >>= 1; // srl     $t1, $t1, 1
	// lw      $ra, 0($sp)
	tmp0 >>= tmp2; // srlv    $t0, $t0, $t2
	arg1 += tmp4; // addu    $a1, $a1, $t4
	var0 = 8; // addiu   $v0, $zero, 8
	tmp0 <<= tmp2; // sllv    $t0, $t0, $t2
	var1 = var0 - tmp0; // subu    $v1, $v0, $t0
	tmp3 = var0 << tmp2; // sllv    $t3, $v0, $t2
	var1 >>= tmp2; // srlv    $v1, $v1, $t2
	ta = tmp0 << 3; // sll     $at, $t0, 3
	arg1 += ta; // addu    $a1, $a1, $at
	ta = tmp1 - var1; // subu    $at, $t1, $v1
	arg0->MC_Chroma(arg1, arg2, arg3, tmp3, var1, ta); // jr      $t5
	// addiu   $sp, $sp, 16
}


static inline void set_mtsab_to_value(int value)
{
	if (value == 1)
	{
		// XXX: mtsab difference?
#if 1
		__asm__ volatile
		(
			"mtsab   $zero, 1\n"
		);
#else
		int tmp;
		__asm__ volatile
		(
			"addiu   %[tmp], $zero, 1\n"
			"mtsab   %[tmp], 0\n"
			: [tmp] "=r"(tmp)
		);
#endif
	}
	else
	{
		int tmp = value;
		__asm__ volatile
		(
			"mtsab   %[tmp], 0\n"
			:
			: [tmp] "r"(tmp)
		);
	}
}

void _MPEG_put_luma(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstY;
	u128 reg1, reg2;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstY = a2;
	set_mtsab_to_value(a3);
	do
	{
		do
		{
			reg1 = ((u128 *)m_pSrc)[0x00];
			reg2 = ((u128 *)m_pSrc)[0x18];
			m_pSrc += a4;
			count -= 1;
			__asm__
			(
				"qfsrv   %[reg1], %[reg2], %[reg1]\n"
				"pextlb  %[reg2], $zero, %[reg1]\n"
				"pextub  %[reg1], $zero, %[reg1]\n"
				: [reg1] "+r"(reg1), [reg2] "+r"(reg2)
			);
			((u128 *)m_pDstY)[0x00] = reg2;
			((u128 *)m_pDstY)[0x01] = reg1;
		}
		while ( count > 0 );
		m_pDstY += 16;
		count = count2;
		m_pSrc += 512;
	}
	while ( count2 > 0 );
}

void _MPEG_put_chroma(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstCbCr;
	u128 reg1, reg2, reg3, reg4;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstCbCr = a2;
	set_mtsab_to_value(a3);
	do
	{
		do
		{
			reg1 = ((u64 *)m_pSrc)[0x00];
			reg2 = ((u64 *)m_pSrc)[0x01];
			reg3 = ((u64 *)m_pSrc)[0x06];
			reg4 = ((u64 *)m_pSrc)[0x07];
			m_pSrc += a4;
			count -= 1;
			__asm__
			(
				"pcpyld  %[reg1], %[reg3], %[reg1]\n"
				"pcpyld  %[reg2], %[reg4], %[reg2]\n"
				"qfsrv   %[reg1], %[reg1], %[reg1]\n"
				"qfsrv   %[reg2], %[reg2], %[reg2]\n"
				"pextlb  %[reg1], $zero, %[reg1]\n"
				"pextlb  %[reg2], $zero, %[reg2]\n"
				: [reg1] "+r"(reg1), [reg2] "+r"(reg2)
				: [reg3] "r"(reg3), [reg4] "r"(reg4)
			);
			((u128 *)m_pDstCbCr)[0x00] = reg1;
			((u128 *)m_pDstCbCr)[0x08] = reg2;
		}
		while ( count > 0 );
		m_pDstCbCr += 8;
		count = count2;
		m_pSrc += 704;
	}
	while ( count2 > 0 );
}

void _MPEG_put_luma_X(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstY;
	u128 reg1, reg2, reg3, reg4, reg5;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstY = a2;
	__asm__
	(
		"pnor    %[reg1], $zero, $zero\n"
		"psrlh   %[reg1], %[reg1], 0xF\n"
		: [reg1] "=r"(reg1)
	);
	do
	{
		do
		{
			reg2 = ((u128 *)m_pSrc)[0x00];
			reg3 = ((u128 *)m_pSrc)[0x18];
			set_mtsab_to_value(a3);
			__asm__
			(
				"qfsrv   %[reg4], %[reg3], %[reg2]\n"
				"qfsrv   %[reg5], %[reg2], %[reg3]\n"
				"pextlb  %[reg2], $zero, %[reg4]\n"
				"pextub  %[reg3], $zero, %[reg4]\n"
				: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "=r"(reg4), [reg5] "=r"(reg5)
			);
			m_pSrc += a4;
			set_mtsab_to_value(1);
			count -= 1;
			__asm__
			(
				"qfsrv   %[reg5], %[reg5], %[reg4]\n"
				"pextlb  %[reg4], $zero, %[reg5]\n"
				"pextub  %[reg5], $zero, %[reg5]\n"
				"paddh   %[reg2], %[reg2], %[reg4]\n"
				"paddh   %[reg3], %[reg3], %[reg5]\n"
				"paddh   %[reg2], %[reg2], %[reg1]\n"
				"paddh   %[reg3], %[reg3], %[reg1]\n"
				"psrlh   %[reg2], %[reg2], 1\n"
				"psrlh   %[reg3], %[reg3], 1\n"
				: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5)
				: [reg1] "r"(reg1)
			);
			((u128 *)m_pDstY)[0x00] = reg2;
			((u128 *)m_pDstY)[0x01] = reg3;
		}
		while ( count > 0 );
		m_pDstY += 16;
		count = count2;
		m_pSrc += 512;
	}
	while ( count2 > 0 );
}

void _MPEG_put_chroma_X(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstCbCr;
	u128 reg1, reg2, reg3, reg4, reg5, reg6, reg7;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstCbCr = a2;
	__asm__
	(
		"pnor    %[reg1], $zero, $zero\n"
		"psrlh   %[reg1], %[reg1], 0xF\n"
		: [reg1] "=r"(reg1)
	);
	do
	{
		do
		{
			reg2 = ((u64 *)m_pSrc)[0x00];
			reg3 = ((u64 *)m_pSrc)[0x01];
			reg4 = ((u64 *)m_pSrc)[0x06];
			reg5 = ((u64 *)m_pSrc)[0x07];
			__asm__
			(
				"pcpyld  %[reg2], %[reg4], %[reg2]\n"
				"pcpyld  %[reg3], %[reg5], %[reg3]\n"
				: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5)
			);
			set_mtsab_to_value(a3);
			__asm__
			(
				"qfsrv   %[reg2], %[reg2], %[reg2]\n"
				"qfsrv   %[reg3], %[reg3], %[reg3]\n"
				: [reg2] "+r"(reg2), [reg3] "+r"(reg3)
			);
			m_pSrc += a4;
			count -= 1;
			set_mtsab_to_value(1);
			__asm__
			(
				"qfsrv   %[reg7], %[reg2], %[reg2]\n"
				"qfsrv   %[reg6], %[reg3], %[reg3]\n"
				"pextlb  %[reg2], $zero, %[reg2]\n"
				"pextlb  %[reg3], $zero, %[reg3]\n"
				"pextlb  %[reg7], $zero, %[reg7]\n"
				"pextlb  %[reg6], $zero, %[reg6]\n"
				"paddh   %[reg2], %[reg2], %[reg7]\n"
				"paddh   %[reg3], %[reg3], %[reg6]\n"
				"paddh   %[reg2], %[reg2], %[reg1]\n"
				"paddh   %[reg3], %[reg3], %[reg1]\n"
				"psrlh   %[reg2], %[reg2], 1\n"
				"psrlh   %[reg3], %[reg3], 1\n"
				: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg6] "=r"(reg6), [reg7] "=r"(reg7)
				: [reg1] "r"(reg1)
			);
			((u128 *)m_pDstCbCr)[0x00] = reg2;
			((u128 *)m_pDstCbCr)[0x08] = reg3;
		}
		while ( count > 0 );
		m_pDstCbCr += 8;
		count = count2;
		m_pSrc += 704;
	}
	while ( count2 > 0 );
}

void _MPEG_put_luma_Y(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstY;
	u128 reg1, reg2, reg3, reg4, reg5, reg6;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstY = a2;
	set_mtsab_to_value(a3);
	reg5 = ((u128 *)m_pSrc)[0x00];
	reg6 = ((u128 *)m_pSrc)[0x18];
	m_pSrc = &m_pSrc[a4];
	count -= 1;
	__asm__
	(
		"qfsrv   %[reg5], %[reg6], %[reg5]\n"
		"pextub  %[reg6], $zero, %[reg5]\n"
		"pextlb  %[reg5], $zero, %[reg5]\n"
		: [reg5] "+r"(reg5), [reg6] "+r"(reg6)
	);
	if ( !count )
		goto LABEL_5;
	count2 += 1;
	do
	{
		do
		{
			reg3 = ((u128 *)m_pSrc)[0x00];
			reg4 = ((u128 *)m_pSrc)[0x18];
			m_pSrc += a4;
			count -= 1;
			__asm__
			(
				"qfsrv   %[reg3], %[reg4], %[reg3]\n"
				"pextub  %[reg4], $zero, %[reg3]\n"
				"pextlb  %[reg3], $zero, %[reg3]\n"
				"paddh   %[reg2], %[reg4], %[reg6]\n"
				"pnor    %[reg6], $zero, $zero\n"
				"paddh   %[reg1], %[reg3], %[reg5]\n"
				"psrlh   %[reg6], %[reg6], 0xF\n"
				"por     %[reg5], $zero, %[reg3]\n"
				"paddh   %[reg1], %[reg1], %[reg6]\n"
				"paddh   %[reg2], %[reg2], %[reg6]\n"
				"por     %[reg6], $zero, %[reg4]\n"
				"psrlh   %[reg1], %[reg1], 1\n"
				"psrlh   %[reg2], %[reg2], 1\n"
				: [reg1] "=r"(reg1), [reg2] "=r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6)
			);
			((u128 *)m_pDstY)[0x00] = reg1;
			((u128 *)m_pDstY)[0x01] = reg2;
		}
		while ( count > 0 );
		m_pDstY += 16;
LABEL_5:
		count = count2;
		m_pSrc += 512;
	}
	while ( count2 > 0 );
}

void _MPEG_put_chroma_Y(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstCbCr;
	u128 reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8, reg9, regA;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstCbCr = a2;
	set_mtsab_to_value(a3);
	reg2 = ((u64 *)m_pSrc)[0x00];
	reg3 = ((u64 *)m_pSrc)[0x01];
	regA = ((u64 *)m_pSrc)[0x06];
	reg9 = ((u64 *)m_pSrc)[0x07];
	__asm__
	(
		"pnor    %[reg1], $zero, $zero\n"
		: [reg1] "=r"(reg1)
	);
	m_pSrc = &m_pSrc[a4];
	count -= 1;
	__asm__
	(
		"psrlh   %[reg1], %[reg1], 0xF\n"
		"pcpyld  %[reg2], %[regA], %[reg2]\n"
		"pcpyld  %[reg3], %[reg9], %[reg3]\n"
		"qfsrv   %[reg2], %[reg2], %[reg2]\n"
		"qfsrv   %[reg3], %[reg3], %[reg3]\n"
		"pextlb  %[reg2], $zero, %[reg2]\n"
		"pextlb  %[reg3], $zero, %[reg3]\n"
		: [reg1] "+r"(reg1), [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg9] "+r"(reg9), [regA] "+r"(regA)
	);
	if ( !count )
		goto LABEL_5;
	count2 += 1;
	do
	{
		do
		{
			reg4 = ((u64 *)m_pSrc)[0x00];
			reg5 = ((u64 *)m_pSrc)[0x01];
			reg6 = ((u64 *)m_pSrc)[0x06];
			reg7 = ((u64 *)m_pSrc)[0x07];
			m_pSrc += a4;
			count -= 1;
			__asm__
			(
				"pcpyld  %[reg4], %[reg6], %[reg4]\n"
				"pcpyld  %[reg5], %[reg7], %[reg5]\n"
				"qfsrv   %[reg4], %[reg4], %[reg4]\n"
				"qfsrv   %[reg5], %[reg5], %[reg5]\n"
				"pextlb  %[reg4], $zero, %[reg4]\n"
				"pextlb  %[reg5], $zero, %[reg5]\n"
				"paddh   %[reg9], %[reg4], %[reg2]\n"
				"paddh   %[reg8], %[reg5], %[reg3]\n"
				"por     %[reg2], $zero, %[reg4]\n"
				"por     %[reg3], $zero, %[reg5]\n"
				"paddh   %[reg9], %[reg9], %[reg1]\n"
				"paddh   %[reg8], %[reg8], %[reg1]\n"
				"psrlh   %[reg9], %[reg9], 1\n"
				"psrlh   %[reg8], %[reg8], 1\n"
				: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg8] "=r"(reg8), [reg9] "=r"(reg9)
				: [reg1] "r"(reg1), [reg6] "r"(reg6), [reg7] "r"(reg7)
			);
			((u128 *)m_pDstCbCr)[0x00] = reg9;
			((u128 *)m_pDstCbCr)[0x08] = reg8;
		}
		while ( count > 0 );
		m_pDstCbCr += 8;
LABEL_5:
		count = count2;
		m_pSrc += 704;
	}
	while ( count2 > 0 );
}

void _MPEG_put_luma_XY(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstY;
	u128 reg1, reg2, reg3, reg4, reg5, reg6;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstY = a2;
	set_mtsab_to_value(a3);
	reg2 = ((u128 *)m_pSrc)[0x00];
	reg5 = ((u128 *)m_pSrc)[0x18];
	m_pSrc = &m_pSrc[a4];
	__asm__
	(
		"qfsrv   %[reg6], %[reg5], %[reg2]\n"
		"qfsrv   %[reg1], %[reg2], %[reg5]\n"
		: [reg1] "=r"(reg1), [reg6] "=r"(reg6)
		: [reg2] "r"(reg2), [reg5] "r"(reg5)
	);
	count -= 1;
	__asm__
	(
		"pextlb  %[reg2], $zero, %[reg6]\n"
		"pextub  %[reg5], $zero, %[reg6]\n"
		: [reg2] "+r"(reg2), [reg5] "+r"(reg5)
		: [reg6] "r"(reg6)
	);
	set_mtsab_to_value(1);
	__asm__
	(
		"qfsrv   %[reg1], %[reg1], %[reg6]\n"
		"pextlb  %[reg6], $zero, %[reg1]\n"
		"pextub  %[reg1], $zero, %[reg1]\n"
		"paddh   %[reg2], %[reg2], %[reg6]\n"
		"paddh   %[reg5], %[reg5], %[reg1]\n"
		: [reg1] "+r"(reg1), [reg2] "+r"(reg2), [reg5] "+r"(reg5), [reg6] "+r"(reg6)
	);
	if ( !count )
		goto LABEL_5;
	count2 += 1;
	do
	{
		do
		{
			reg3 = ((u128 *)m_pSrc)[0x00];
			reg4 = ((u128 *)m_pSrc)[0x18];
			set_mtsab_to_value(a3);
			m_pSrc += a4;
			__asm__
			(
				"qfsrv   %[reg6], %[reg4], %[reg3]\n"
				"qfsrv   %[reg1], %[reg3], %[reg4]\n"
				: [reg1] "+r"(reg1), [reg6] "+r"(reg6)
				: [reg3] "r"(reg3), [reg4] "r"(reg4)
			);
			count -= 1;
			__asm__
			(
				"pextlb  %[reg3], $zero, %[reg6]\n"
				"pextub  %[reg4], $zero, %[reg6]\n"
				: [reg3] "=r"(reg3), [reg4] "=r"(reg4)
				: [reg6] "r"(reg6)
			);
			set_mtsab_to_value(1);
			__asm__
			(
				"qfsrv   %[reg1], %[reg1], %[reg6]\n"
				"pextlb  %[reg6], $zero, %[reg1]\n"
				"pextub  %[reg1], $zero, %[reg1]\n"
				"paddh   %[reg3], %[reg3], %[reg6]\n"
				"paddh   %[reg4], %[reg4], %[reg1]\n"
				"paddh   %[reg6], %[reg2], %[reg3]\n"
				"paddh   %[reg1], %[reg5], %[reg4]\n"
				"por     %[reg2], $zero, %[reg3]\n"
				"pnor    %[reg3], $zero, $zero\n"
				"por     %[reg5], $zero, %[reg4]\n"
				"psrlh   %[reg3], %[reg3], 0xF\n"
				"psllh   %[reg3], %[reg3], 1\n"
				"paddh   %[reg6], %[reg6], %[reg3]\n"
				"paddh   %[reg1], %[reg1], %[reg3]\n"
				"psrlh   %[reg6], %[reg6], 2\n"
				"psrlh   %[reg1], %[reg1], 2\n"
				: [reg1] "+r"(reg1), [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6)
			);
			((u128 *)m_pDstY)[0x00] = reg6;
			((u128 *)m_pDstY)[0x01] = reg1;
		}
		while ( count > 0 );
		m_pDstY += 16;
LABEL_5:
		count = count2;
		m_pSrc += 512;
	}
	while ( count2 > 0 );
}

void _MPEG_put_chroma_XY(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstCbCr;
	u128 reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8, reg9;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstCbCr = a2;
	set_mtsab_to_value(a3);
	__asm__
	(
		"pnor    %[reg1], $zero, $zero\n"
		: [reg1] "=r"(reg1)
	);
	reg3 = ((u64 *)m_pSrc)[0x00];
	reg2 = ((u64 *)m_pSrc)[0x01];
	set_mtsab_to_value(1);
	reg9 = ((u64 *)m_pSrc)[0x06];
	reg8 = ((u64 *)m_pSrc)[0x07];
	__asm__
	(
		"pcpyld  %[reg3], %[reg9], %[reg3]\n"
		"pcpyld  %[reg2], %[reg8], %[reg2]\n"
		"qfsrv   %[reg3], %[reg3], %[reg3]\n"
		"qfsrv   %[reg2], %[reg2], %[reg2]\n"
		"psrlh   %[reg1], %[reg1], 0xF\n"
		"psllh   %[reg1], %[reg1], 1\n"
		: [reg1] "+r"(reg1), [reg2] "+r"(reg2), [reg3] "+r"(reg3)
		: [reg8] "r"(reg8), [reg9] "r"(reg9)
	);
	m_pSrc = &m_pSrc[a4];
	count -= 1;
	__asm__
	(
		"qfsrv   %[reg9], %[reg3], %[reg3]\n"
		"qfsrv   %[reg8], %[reg2], %[reg2]\n"
		"pextlb  %[reg3], $zero, %[reg3]\n"
		"pextlb  %[reg2], $zero, %[reg2]\n"
		"pextlb  %[reg9], $zero, %[reg9]\n"
		"pextlb  %[reg8], $zero, %[reg8]\n"
		"paddh   %[reg3], %[reg3], %[reg9]\n"
		"paddh   %[reg9], %[reg2], %[reg8]\n"
		: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
	);
	if ( !count )
		goto LABEL_5;
	count2 += 1;
	do
	{
		do
		{
			reg4 = ((u64 *)m_pSrc)[0x00];
			reg6 = ((u64 *)m_pSrc)[0x01];
			set_mtsab_to_value(a3);
			reg5 = ((u64 *)m_pSrc)[0x06];
			reg7 = ((u64 *)m_pSrc)[0x07];
			__asm__
			(
				"pcpyld  %[reg4], %[reg5], %[reg4]\n"
				"pcpyld  %[reg6], %[reg7], %[reg6]\n"
				"qfsrv   %[reg4], %[reg4], %[reg4]\n"
				"qfsrv   %[reg6], %[reg6], %[reg6]\n"
				: [reg4] "+r"(reg4), [reg6] "+r"(reg6)
				: [reg5] "r"(reg5), [reg7] "r"(reg7)
			);
			m_pSrc += a4;
			count -= 1;
			set_mtsab_to_value(1);
			__asm__
			(
				"qfsrv   %[reg5], %[reg4], %[reg4]\n"
				"qfsrv   %[reg7], %[reg6], %[reg6]\n"
				"pextlb  %[reg4], $zero, %[reg4]\n"
				"pextlb  %[reg6], $zero, %[reg6]\n"
				"pextlb  %[reg5], $zero, %[reg5]\n"
				"pextlb  %[reg7], $zero, %[reg7]\n"
				"paddh   %[reg4], %[reg4], %[reg5]\n"
				"paddh   %[reg5], %[reg6], %[reg7]\n"
				"paddh   %[reg6], %[reg3], %[reg4]\n"
				"paddh   %[reg7], %[reg9], %[reg5]\n"
				"por     %[reg3], $zero, %[reg4]\n"
				"por     %[reg9], $zero, %[reg5]\n"
				"paddh   %[reg6], %[reg6], %[reg1]\n"
				"paddh   %[reg7], %[reg7], %[reg1]\n"
				"psrlh   %[reg6], %[reg6], 2\n"
				"psrlh   %[reg7], %[reg7], 2\n"
				: [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg9] "+r"(reg9)
				: [reg1] "r"(reg1)
			);
			((u128 *)m_pDstCbCr)[0x00] = reg6;
			((u128 *)m_pDstCbCr)[0x08] = reg7;
		}
		while ( count > 0 );
		m_pDstCbCr += 8;
LABEL_5:
		count = count2;
		m_pSrc += 704;
	}
	while ( count2 > 0 );
}

void _MPEG_avg_luma(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstY;
	u128 reg1, reg2, reg3, reg4, reg5, reg6;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstY = a2;
	set_mtsab_to_value(a3);
	do
	{
		do
		{
			reg3 = ((u128 *)m_pSrc)[0x00];
			reg4 = ((u128 *)m_pSrc)[0x18];
			m_pSrc += a4;
			count -= 1;
			__asm__
			(
				"qfsrv   %[reg3], %[reg4], %[reg3]\n"
				"pextlb  %[reg4], $zero, %[reg3]\n"
				"pextub  %[reg3], $zero, %[reg3]\n"
				: [reg3] "+r"(reg3), [reg4] "+r"(reg4)
			);
			reg6 = ((u128 *)m_pDstY)[0x00];
			reg1 = ((u128 *)m_pDstY)[0x01];
			__asm__
			(
				"paddh   %[reg4], %[reg4], %[reg6]\n"
				"paddh   %[reg3], %[reg3], %[reg1]\n"
				"pcgth   %[reg6], %[reg4], $zero\n"
				"pcgth   %[reg1], %[reg3], $zero\n"
				"pceqh   %[reg2], %[reg4], $zero\n"
				"pceqh   %[reg5], %[reg3], $zero\n"
				"psrlh   %[reg6], %[reg6], 0xF\n"
				"psrlh   %[reg1], %[reg1], 0xF\n"
				"psrlh   %[reg2], %[reg2], 0xF\n"
				"psrlh   %[reg5], %[reg5], 0xF\n"
				"por     %[reg6], %[reg6], %[reg2]\n"
				"por     %[reg1], %[reg1], %[reg5]\n"
				"paddh   %[reg4], %[reg4], %[reg6]\n"
				"paddh   %[reg3], %[reg3], %[reg1]\n"
				"psrlh   %[reg4], %[reg4], 1\n"
				"psrlh   %[reg3], %[reg3], 1\n"
				: [reg1] "+r"(reg1), [reg2] "=r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "=r"(reg5), [reg6] "+r"(reg6)
			);
			((u128 *)m_pDstY)[0x00] = reg4;
			((u128 *)m_pDstY)[0x01] = reg3;
		}
		while ( count > 0 );
		m_pDstY += 16;
		count = count2;
		m_pSrc += 512;
	}
	while ( count2 > 0 );
}

void _MPEG_avg_chroma(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstCbCr;
	u128 reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstCbCr = a2;
	set_mtsab_to_value(a3);
	do
	{
		do
		{
			reg3 = ((u64 *)m_pSrc)[0x00];
			reg4 = ((u64 *)m_pSrc)[0x01];
			count -= 1;
			reg5 = ((u64 *)m_pSrc)[0x06];
			reg6 = ((u64 *)m_pSrc)[0x07];
			m_pSrc += a4;
			__asm__
			(
				"pcpyld  %[reg3], %[reg5], %[reg3]\n"
				"pcpyld  %[reg4], %[reg6], %[reg4]\n"
				"qfsrv   %[reg3], %[reg3], %[reg3]\n"
				"qfsrv   %[reg4], %[reg4], %[reg4]\n"
				"pextlb  %[reg3], $zero, %[reg3]\n"
				"pextlb  %[reg4], $zero, %[reg4]\n"
				: [reg3] "+r"(reg3), [reg4] "+r"(reg4)
				: [reg5] "r"(reg5), [reg6] "r"(reg6)
			);
			reg8 = ((u128 *)m_pDstCbCr)[0x00];
			reg7 = ((u128 *)m_pDstCbCr)[0x08];
			__asm__
			(
				"paddh   %[reg3], %[reg3], %[reg8]\n"
				"paddh   %[reg4], %[reg4], %[reg7]\n"
				"pcgth   %[reg8], %[reg3], $zero\n"
				"pcgth   %[reg7], %[reg4], $zero\n"
				"pceqh   %[reg2], %[reg3], $zero\n"
				"pceqh   %[reg1], %[reg4], $zero\n"
				"psrlh   %[reg8], %[reg8], 0xF\n"
				"psrlh   %[reg7], %[reg7], 0xF\n"
				"psrlh   %[reg2], %[reg2], 0xF\n"
				"psrlh   %[reg1], %[reg1], 0xF\n"
				"por     %[reg8], %[reg8], %[reg2]\n"
				"por     %[reg7], %[reg7], %[reg1]\n"
				"paddh   %[reg3], %[reg3], %[reg8]\n"
				"paddh   %[reg4], %[reg4], %[reg7]\n"
				"psrlh   %[reg3], %[reg3], 1\n"
				"psrlh   %[reg4], %[reg4], 1\n"
				: [reg1] "=r"(reg1), [reg2] "=r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg7] "+r"(reg7), [reg8] "+r"(reg8)
			);
			((u128 *)m_pDstCbCr)[0x00] = reg4;
			((u128 *)m_pDstCbCr)[0x08] = reg3;
		}
		while ( count > 0 );
		m_pDstCbCr += 8;
		count = count2;
		m_pSrc += 704;
	}
	while ( count2 > 0 );
}

void _MPEG_avg_luma_X(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstY;
	u128 reg1, reg2, reg3, reg4, reg5, reg6;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstY = a2;
	__asm__
	(
		"pnor    %[reg2], $zero, $zero\n"
		"psrlh   %[reg2], %[reg2], 0xF\n"
		: [reg2] "=r"(reg2)
	);
	do
	{
		do
		{
			reg3 = ((u128 *)m_pSrc)[0x00];
			reg4 = ((u128 *)m_pSrc)[0x18];
			set_mtsab_to_value(a3);
			__asm__
			(
				"qfsrv   %[reg5], %[reg4], %[reg3]\n"
				"qfsrv   %[reg6], %[reg3], %[reg4]\n"
				"pextlb  %[reg3], $zero, %[reg5]\n"
				"pextub  %[reg4], $zero, %[reg5]\n"
				: [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "=r"(reg5), [reg6] "=r"(reg6)
			);
			m_pSrc += a4;
			set_mtsab_to_value(1);
			count -= 1;
			__asm__
			(
				"qfsrv   %[reg6], %[reg6], %[reg5]\n"
				"pextlb  %[reg5], $zero, %[reg6]\n"
				"pextub  %[reg6], $zero, %[reg6]\n"
				"paddh   %[reg3], %[reg3], %[reg5]\n"
				"paddh   %[reg4], %[reg4], %[reg6]\n"
				"paddh   %[reg3], %[reg3], %[reg2]\n"
				"paddh   %[reg4], %[reg4], %[reg2]\n"
				"psrlh   %[reg3], %[reg3], 1\n"
				"psrlh   %[reg4], %[reg4], 1\n"
				: [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6)
				: [reg2] "r"(reg2)
			);
			reg6 = ((u128 *)m_pDstY)[0x00];
			reg1 = ((u128 *)m_pDstY)[0x01];
			__asm__
			(
				"paddh   %[reg3], %[reg3], %[reg6]\n"
				"paddh   %[reg4], %[reg4], %[reg1]\n"
				"pcgth   %[reg6], %[reg3], $zero\n"
				"pceqh   %[reg1], %[reg3], $zero\n"
				"psrlh   %[reg6], %[reg6], 0xF\n"
				"psrlh   %[reg1], %[reg1], 0xF\n"
				"por     %[reg6], %[reg6], %[reg1]\n"
				"paddh   %[reg3], %[reg3], %[reg6]\n"
				"pcgth   %[reg6], %[reg4], $zero\n"
				"pceqh   %[reg1], %[reg4], $zero\n"
				"psrlh   %[reg6], %[reg6], 0xF\n"
				"psrlh   %[reg1], %[reg1], 0xF\n"
				"por     %[reg6], %[reg6], %[reg1]\n"
				"paddh   %[reg4], %[reg4], %[reg6]\n"
				"psrlh   %[reg3], %[reg3], 1\n"
				"psrlh   %[reg4], %[reg4], 1\n"
				: [reg1] "+r"(reg1), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg6] "+r"(reg6)
			);
			((u128 *)m_pDstY)[0x00] = reg3;
			((u128 *)m_pDstY)[0x01] = reg4;
		}
		while ( count > 0 );
		m_pDstY += 16;
		count = count2;
		m_pSrc += 512;
	}
	while ( count2 > 0 );
}

void _MPEG_avg_chroma_X(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstCbCr;
	u128 reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8, reg9;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstCbCr = a2;
	__asm__
	(
		"pnor    %[reg2], $zero, $zero\n"
		"psrlh   %[reg2], %[reg2], 0xF\n"
		: [reg2] "=r"(reg2)
	);
	do
	{
		do
		{
			reg4 = ((u64 *)m_pSrc)[0x00];
			reg5 = ((u64 *)m_pSrc)[0x01];
			set_mtsab_to_value(a3);
			reg6 = ((u64 *)m_pSrc)[0x06];
			reg7 = ((u64 *)m_pSrc)[0x07];
			__asm__
			(
				"pcpyld  %[reg4], %[reg6], %[reg4]\n"
				"pcpyld  %[reg5], %[reg7], %[reg5]\n"
				"qfsrv   %[reg4], %[reg4], %[reg4]\n"
				"qfsrv   %[reg5], %[reg5], %[reg5]\n"
				: [reg4] "+r"(reg4), [reg5] "+r"(reg5)
				: [reg6] "r"(reg6), [reg7] "r"(reg7)
			);
			m_pSrc += a4;
			count -= 1;
			set_mtsab_to_value(1);
			__asm__
			(
				"qfsrv   %[reg9], %[reg4], %[reg4]\n"
				"qfsrv   %[reg8], %[reg5], %[reg5]\n"
				"pextlb  %[reg4], $zero, %[reg4]\n"
				"pextlb  %[reg5], $zero, %[reg5]\n"
				"pextlb  %[reg9], $zero, %[reg9]\n"
				"pextlb  %[reg8], $zero, %[reg8]\n"
				"paddh   %[reg4], %[reg4], %[reg9]\n"
				"paddh   %[reg5], %[reg5], %[reg8]\n"
				"paddh   %[reg4], %[reg4], %[reg2]\n"
				"paddh   %[reg5], %[reg5], %[reg2]\n"
				"psrlh   %[reg4], %[reg4], 1\n"
				"psrlh   %[reg5], %[reg5], 1\n"
				: [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg8] "=r"(reg8), [reg9] "=r"(reg9)
				: [reg2] "r"(reg2)
			);
			reg9 = ((u128 *)m_pDstCbCr)[0x00];
			reg8 = ((u128 *)m_pDstCbCr)[0x08];
			__asm__
			(
				"paddh   %[reg4], %[reg4], %[reg9]\n"
				"paddh   %[reg5], %[reg5], %[reg8]\n"
				"pcgth   %[reg9], %[reg4], $zero\n"
				"pcgth   %[reg8], %[reg5], $zero\n"
				"pceqh   %[reg1], %[reg4], $zero\n"
				"pceqh   %[reg3], %[reg5], $zero\n"
				"psrlh   %[reg9], %[reg9], 0xF\n"
				"psrlh   %[reg8], %[reg8], 0xF\n"
				"psrlh   %[reg1], %[reg1], 0xF\n"
				"psrlh   %[reg3], %[reg3], 0xF\n"
				"por     %[reg9], %[reg9], %[reg1]\n"
				"por     %[reg8], %[reg8], %[reg3]\n"
				"paddh   %[reg4], %[reg4], %[reg9]\n"
				"paddh   %[reg5], %[reg5], %[reg8]\n"
				"psrlh   %[reg4], %[reg4], 1\n"
				"psrlh   %[reg5], %[reg5], 1\n"
				: [reg1] "=r"(reg1), [reg3] "=r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			);
			((u128 *)m_pDstCbCr)[0x00] = reg4;
			((u128 *)m_pDstCbCr)[0x08] = reg5;
		}
		while ( count > 0 );
		m_pDstCbCr += 8;
		count = count2;
		m_pSrc += 704;
	}
	while ( count2 > 0 );
}

void _MPEG_avg_luma_Y(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstY;
	u128 reg1, reg2, reg3, reg4, reg5, reg6;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstY = a2;
	set_mtsab_to_value(a3);
	reg5 = ((u128 *)m_pSrc)[0x00];
	reg6 = ((u128 *)m_pSrc)[0x18];
	m_pSrc = &m_pSrc[a4];
	count -= 1;
	__asm__
	(
		"qfsrv   %[reg5], %[reg6], %[reg5]\n"
		"pextub  %[reg6], $zero, %[reg5]\n"
		"pextlb  %[reg5], $zero, %[reg5]\n"
		: [reg5] "+r"(reg5), [reg6] "+r"(reg6)
	);
	if ( !count )
		goto LABEL_5;
	count2 += 1;
	do
	{
		do
		{
			reg3 = ((u128 *)m_pSrc)[0x00];
			reg4 = ((u128 *)m_pSrc)[0x18];
			m_pSrc += a4;
			count -= 1;
			__asm__
			(
				"qfsrv   %[reg3], %[reg4], %[reg3]\n"
				"pextub  %[reg4], $zero, %[reg3]\n"
				"pextlb  %[reg3], $zero, %[reg3]\n"
				"paddh   %[reg2], %[reg4], %[reg6]\n"
				"pnor    %[reg6], $zero, $zero\n"
				"paddh   %[reg1], %[reg3], %[reg5]\n"
				"psrlh   %[reg6], %[reg6], 0xF\n"
				"por     %[reg5], $zero, %[reg3]\n"
				"paddh   %[reg1], %[reg1], %[reg6]\n"
				"paddh   %[reg2], %[reg2], %[reg6]\n"
				"por     %[reg6], $zero, %[reg4]\n"
				"psrlh   %[reg1], %[reg1], 1\n"
				"psrlh   %[reg2], %[reg2], 1\n"
				: [reg1] "=r"(reg1), [reg2] "=r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6)
			);
			reg3 = ((u128 *)m_pDstY)[0x00];
			reg4 = ((u128 *)m_pDstY)[0x01];
			__asm__
			(
				"paddh   %[reg1], %[reg1], %[reg3]\n"
				"paddh   %[reg2], %[reg2], %[reg4]\n"
				"pcgth   %[reg3], %[reg1], $zero\n"
				"pceqh   %[reg4], %[reg1], $zero\n"
				"psrlh   %[reg3], %[reg3], 0xF\n"
				"psrlh   %[reg4], %[reg4], 0xF\n"
				"por     %[reg3], %[reg3], %[reg4]\n"
				"paddh   %[reg1], %[reg1], %[reg3]\n"
				"pcgth   %[reg3], %[reg2], $zero\n"
				"pceqh   %[reg4], %[reg2], $zero\n"
				"psrlh   %[reg3], %[reg3], 0xF\n"
				"psrlh   %[reg4], %[reg4], 0xF\n"
				"por     %[reg3], %[reg3], %[reg4]\n"
				"paddh   %[reg2], %[reg2], %[reg3]\n"
				"psrlh   %[reg1], %[reg1], 1\n"
				"psrlh   %[reg2], %[reg2], 1\n"
				: [reg1] "+r"(reg1), [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4)
			);
			((u128 *)m_pDstY)[0x00] = reg1;
			((u128 *)m_pDstY)[0x01] = reg2;
		}
		while ( count > 0 );
		m_pDstY += 16;
LABEL_5:
		count = count2;
		m_pSrc += 512;
	}
	while ( count2 > 0 );
}

void _MPEG_avg_chroma_Y(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstCbCr;
	u128 reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8, reg9, regA;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstCbCr = a2;
	set_mtsab_to_value(a3);
	reg2 = ((u64 *)m_pSrc)[0x00];
	reg3 = ((u64 *)m_pSrc)[0x01];
	regA = ((u64 *)m_pSrc)[0x06];
	reg9 = ((u64 *)m_pSrc)[0x07];
	__asm__
	(
		"pnor    %[reg1], $zero, $zero\n"
		: [reg1] "=r"(reg1)
	);
	m_pSrc = &m_pSrc[a4];
	count -= 1;
	__asm__
	(
		"psrlh   %[reg1], %[reg1], 0xF\n"
		"pcpyld  %[reg2], %[regA], %[reg2]\n"
		"pcpyld  %[reg3], %[reg9], %[reg3]\n"
		"qfsrv   %[reg2], %[reg2], %[reg2]\n"
		"qfsrv   %[reg3], %[reg3], %[reg3]\n"
		"pextlb  %[reg2], $zero, %[reg2]\n"
		"pextlb  %[reg3], $zero, %[reg3]\n"
		: [reg1] "+r"(reg1), [reg2] "+r"(reg2), [reg3] "+r"(reg3)
		: [reg9] "r"(reg9), [regA] "r"(regA)
	);
	if ( !count )
		goto LABEL_5;
	count2 += 1;
	do
	{
		do
		{
			reg4 = ((u64 *)m_pSrc)[0x00];
			reg5 = ((u64 *)m_pSrc)[0x01];
			count -= 1;
			reg6 = ((u64 *)m_pSrc)[0x06];
			reg7 = ((u64 *)m_pSrc)[0x07];
			m_pSrc += a4;
			__asm__
			(
				"pcpyld  %[reg4], %[reg6], %[reg4]\n"
				"pcpyld  %[reg5], %[reg7], %[reg5]\n"
				"qfsrv   %[reg4], %[reg4], %[reg4]\n"
				"qfsrv   %[reg5], %[reg5], %[reg5]\n"
				"pextlb  %[reg4], $zero, %[reg4]\n"
				"pextlb  %[reg5], $zero, %[reg5]\n"
				"paddh   %[reg9], %[reg4], %[reg2]\n"
				"paddh   %[reg8], %[reg5], %[reg3]\n"
				"por     %[reg2], $zero, %[reg4]\n"
				"por     %[reg3], $zero, %[reg5]\n"
				"paddh   %[reg9], %[reg9], %[reg1]\n"
				"paddh   %[reg8], %[reg8], %[reg1]\n"
				"psrlh   %[reg9], %[reg9], 1\n"
				"psrlh   %[reg8], %[reg8], 1\n"
				: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg8] "=r"(reg8), [reg9] "=r"(reg9)
				: [reg1] "r"(reg1), [reg6] "r"(reg6), [reg7] "r"(reg7)
			);
			reg4 = ((u128 *)m_pDstCbCr)[0x00];
			reg5 = ((u128 *)m_pDstCbCr)[0x80];
			__asm__
			(
				"paddh   %[reg9], %[reg9], %[reg4]\n"
				"paddh   %[reg8], %[reg8], %[reg5]\n"
				"pcgth   %[reg4], %[reg9], $zero\n"
				"pceqh   %[reg5], %[reg9], $zero\n"
				"psrlh   %[reg4], %[reg4], 0xF\n"
				"psrlh   %[reg5], %[reg5], 0xF\n"
				"por     %[reg4], %[reg4], %[reg5]\n"
				"paddh   %[reg9], %[reg9], %[reg4]\n"
				"pcgth   %[reg4], %[reg8], $zero\n"
				"pceqh   %[reg5], %[reg8], $zero\n"
				"psrlh   %[reg4], %[reg4], 0xF\n"
				"psrlh   %[reg5], %[reg5], 0xF\n"
				"por     %[reg4], %[reg4], %[reg5]\n"
				"paddh   %[reg8], %[reg8], %[reg4]\n"
				"psrlh   %[reg9], %[reg9], 1\n"
				"psrlh   %[reg8], %[reg8], 1\n"
				: [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg8] "+r"(reg8), [reg9] "+r"(reg9)
			);
			((u128 *)m_pDstCbCr)[0x00] = reg9;
			((u128 *)m_pDstCbCr)[0x08] = reg8;
		}
		while ( count > 0 );
		m_pDstCbCr += 8;
LABEL_5:
		count = count2;
		m_pSrc += 704;
	}
	while ( count2 > 0 );
}

void _MPEG_avg_luma_XY(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstY;
	u128 reg1, reg2, reg3, reg4, reg5, reg6;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstY = a2;
	set_mtsab_to_value(a3);
	reg2 = ((u128 *)m_pSrc)[0x00];
	reg5 = ((u128 *)m_pSrc)[0x18];
	m_pSrc = &m_pSrc[a4];
	__asm__
	(
		"qfsrv   %[reg6], %[reg5], %[reg2]\n"
		"qfsrv   %[reg1], %[reg2], %[reg5]\n"
		: [reg1] "=r"(reg1), [reg6] "=r"(reg6)
		: [reg2] "r"(reg2), [reg5] "r"(reg5)
	);
	count -= 1;
	__asm__
	(
		"pextlb  %[reg2], $zero, %[reg6]\n"
		"pextub  %[reg5], $zero, %[reg6]\n"
		: [reg2] "+r"(reg2), [reg5] "+r"(reg5)
		: [reg6] "r"(reg6)
	);
	set_mtsab_to_value(1);
	__asm__
	(
		"qfsrv   %[reg1], %[reg1], %[reg6]\n"
		"pextlb  %[reg6], $zero, %[reg1]\n"
		"pextub  %[reg1], $zero, %[reg1]\n"
		"paddh   %[reg2], %[reg2], %[reg6]\n"
		"paddh   %[reg5], %[reg5], %[reg1]\n"
		: [reg1] "+r"(reg1), [reg2] "+r"(reg2), [reg5] "+r"(reg5), [reg6] "+r"(reg6)
	);
	if ( !count )
		goto LABEL_5;
	count2 += 1;
	do
	{
		do
		{
			reg3 = ((u128 *)m_pSrc)[0x00];
			reg4 = ((u128 *)m_pSrc)[0x18];
			set_mtsab_to_value(a3);
			m_pSrc += a4;
			__asm__
			(
				"qfsrv   %[reg6], %[reg4], %[reg3]\n"
				"qfsrv   %[reg1], %[reg3], %[reg4]\n"
				: [reg1] "=r"(reg1), [reg6] "=r"(reg6)
				: [reg3] "r"(reg3), [reg4] "r"(reg4)
			);
			count -= 1;
			__asm__
			(
				"pextlb  %[reg3], $zero, %[reg6]\n"
				"pextub  %[reg4], $zero, %[reg6]\n"
				: [reg3] "=r"(reg3), [reg4] "=r"(reg4)
				: [reg6] "r"(reg6)
			);
			set_mtsab_to_value(1);
			__asm__
			(
				"qfsrv   %[reg1], %[reg1], %[reg6]\n"
				"pextlb  %[reg6], $zero, %[reg1]\n"
				"pextub  %[reg1], $zero, %[reg1]\n"
				"paddh   %[reg3], %[reg3], %[reg6]\n"
				"paddh   %[reg4], %[reg4], %[reg1]\n"
				"paddh   %[reg6], %[reg2], %[reg3]\n"
				"paddh   %[reg1], %[reg5], %[reg4]\n"
				"por     %[reg2], $zero, %[reg3]\n"
				"pnor    %[reg3], $zero, $zero\n"
				"por     %[reg5], $zero, %[reg4]\n"
				"psrlh   %[reg3], %[reg3], 0xF\n"
				"psllh   %[reg3], %[reg3], 1\n"
				"paddh   %[reg6], %[reg6], %[reg3]\n"
				"paddh   %[reg1], %[reg1], %[reg3]\n"
				"psrlh   %[reg6], %[reg6], 2\n"
				"psrlh   %[reg1], %[reg1], 2\n"
				: [reg1] "+r"(reg1), [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6)
			);
			reg3 = ((u128 *)m_pDstY)[0x00];
			reg4 = ((u128 *)m_pDstY)[0x01];
			__asm__
			(
				"paddh   %[reg6], %[reg6], %[reg3]\n"
				"paddh   %[reg1], %[reg1], %[reg4]\n"
				"pcgth   %[reg3], %[reg6], $zero\n"
				"pceqh   %[reg4], %[reg6], $zero\n"
				"psrlh   %[reg3], %[reg3], 0xF\n"
				"psrlh   %[reg4], %[reg4], 0xF\n"
				"por     %[reg3], %[reg3], %[reg4]\n"
				"paddh   %[reg6], %[reg6], %[reg3]\n"
				"pcgth   %[reg3], %[reg1], $zero\n"
				"pceqh   %[reg4], %[reg1], $zero\n"
				"psrlh   %[reg3], %[reg3], 0xF\n"
				"psrlh   %[reg4], %[reg4], 0xF\n"
				"por     %[reg3], %[reg3], %[reg4]\n"
				"paddh   %[reg1], %[reg1], %[reg3]\n"
				"psrlh   %[reg6], %[reg6], 1\n"
				"psrlh   %[reg1], %[reg1], 1\n"
				: [reg1] "+r"(reg1), [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg6] "+r"(reg6)
			);
			((u128 *)m_pDstY)[0x00] = reg6;
			((u128 *)m_pDstY)[0x01] = reg1;
		}
		while ( count > 0 );
		m_pDstY += 16;
LABEL_5:
		count = count2;
		m_pSrc += 512;
	}
	while ( count2 > 0 );
}

void _MPEG_avg_chroma_XY(u8 *a1, u16 *a2, int a3, int a4, int var1, int ta)
{
	int count;
	int count2;
	u8 *m_pSrc;
	u16 *m_pDstCbCr;
	u128 reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8, reg9;

	count = var1;
	count2 = ta;
	m_pSrc = a1;
	m_pDstCbCr = a2;
	set_mtsab_to_value(a3);
	__asm__
	(
		"pnor    %[reg1], $zero, $zero\n"
		: [reg1] "=r"(reg1)
	);
	reg3 = ((u64 *)m_pSrc)[0x00];
	reg2 = ((u64 *)m_pSrc)[0x01];
	set_mtsab_to_value(1);
	reg9 = ((u64 *)m_pSrc)[0x06];
	reg8 = ((u64 *)m_pSrc)[0x07];
	__asm__
	(
		"pcpyld  %[reg3], %[reg9], %[reg3]\n"
		"pcpyld  %[reg2], %[reg8], %[reg2]\n"
		"qfsrv   %[reg3], %[reg3], %[reg3]\n"
		"qfsrv   %[reg2], %[reg2], %[reg2]\n"
		"psrlh   %[reg1], %[reg1], 0xF\n"
		"psllh   %[reg1], %[reg1], 1\n"
		: [reg1] "+r"(reg1), [reg2] "+r"(reg2), [reg3] "+r"(reg3)
		: [reg8] "r"(reg8), [reg9] "r"(reg9)
	);
	m_pSrc = &m_pSrc[a4];
	count -= 1;
	__asm__
	(
		"qfsrv   %[reg9], %[reg3], %[reg3]\n"
		"qfsrv   %[reg8], %[reg2], %[reg2]\n"
		"pextlb  %[reg3], $zero, %[reg3]\n"
		"pextlb  %[reg2], $zero, %[reg2]\n"
		"pextlb  %[reg9], $zero, %[reg9]\n"
		"pextlb  %[reg8], $zero, %[reg8]\n"
		"paddh   %[reg3], %[reg3], %[reg9]\n"
		"paddh   %[reg9], %[reg2], %[reg8]\n"
		: [reg2] "+r"(reg2), [reg3] "+r"(reg3), [reg8] "=r"(reg8), [reg9] "=r"(reg9)
	);
	if ( !count )
		goto LABEL_5;
	count2 += 1;
	do
	{
		do
		{
			reg4 = ((u64 *)m_pSrc)[0x00];
			reg6 = ((u64 *)m_pSrc)[0x01];
			set_mtsab_to_value(a3);
			reg5 = ((u64 *)m_pSrc)[0x06];
			reg7 = ((u64 *)m_pSrc)[0x07];
			__asm__
			(
				"pcpyld  %[reg4], %[reg5], %[reg4]\n"
				"pcpyld  %[reg6], %[reg7], %[reg6]\n"
				"qfsrv   %[reg4], %[reg4], %[reg4]\n"
				"qfsrv   %[reg6], %[reg6], %[reg6]\n"
				: [reg4] "+r"(reg4), [reg6] "+r"(reg6)
				: [reg5] "r"(reg5), [reg7] "r"(reg7)
			);
			m_pSrc += a4;
			count -= 1;
			set_mtsab_to_value(1);
			__asm__
			(
				"qfsrv   %[reg5], %[reg4], %[reg4]\n"
				"qfsrv   %[reg7], %[reg6], %[reg6]\n"
				"pextlb  %[reg4], $zero, %[reg4]\n"
				"pextlb  %[reg6], $zero, %[reg6]\n"
				"pextlb  %[reg5], $zero, %[reg5]\n"
				"pextlb  %[reg7], $zero, %[reg7]\n"
				"paddh   %[reg4], %[reg4], %[reg5]\n"
				"paddh   %[reg5], %[reg6], %[reg7]\n"
				"paddh   %[reg6], %[reg3], %[reg4]\n"
				"paddh   %[reg7], %[reg9], %[reg5]\n"
				"por     %[reg3], $zero, %[reg4]\n"
				"por     %[reg9], $zero, %[reg5]\n"
				"paddh   %[reg6], %[reg6], %[reg1]\n"
				"paddh   %[reg7], %[reg7], %[reg1]\n"
				"psrlh   %[reg6], %[reg6], 2\n"
				"psrlh   %[reg7], %[reg7], 2\n"
				: [reg3] "+r"(reg3), [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7), [reg9] "+r"(reg9)
				: [reg1] "r"(reg1)
			);
			reg4 = ((u128 *)m_pDstCbCr)[0x00];
			reg5 = ((u128 *)m_pDstCbCr)[0x08];
			__asm__
			(
				"paddh   %[reg6], %[reg6], %[reg4]\n"
				"paddh   %[reg7], %[reg7], %[reg5]\n"
				"pcgth   %[reg4], %[reg6], $zero\n"
				"pceqh   %[reg5], %[reg6], $zero\n"
				"psrlh   %[reg4], %[reg4], 0xF\n"
				"psrlh   %[reg5], %[reg5], 0xF\n"
				"por     %[reg4], %[reg4], %[reg5]\n"
				"paddh   %[reg6], %[reg6], %[reg4]\n"
				"pcgth   %[reg4], %[reg7], $zero\n"
				"pceqh   %[reg5], %[reg7], $zero\n"
				"psrlh   %[reg4], %[reg4], 0xF\n"
				"psrlh   %[reg5], %[reg5], 0xF\n"
				"por     %[reg4], %[reg4], %[reg5]\n"
				"paddh   %[reg7], %[reg7], %[reg4]\n"
				"psrlh   %[reg6], %[reg6], 1\n"
				"psrlh   %[reg7], %[reg7], 1\n"
				: [reg4] "+r"(reg4), [reg5] "+r"(reg5), [reg6] "+r"(reg6), [reg7] "+r"(reg7)
			);
			((u128 *)m_pDstCbCr)[0x00] = reg6;
			((u128 *)m_pDstCbCr)[0x08] = reg7;
		}
		while ( count > 0 );
		m_pDstCbCr += 8;
LABEL_5:
		count = count2;
		m_pSrc += 704;
	}
	while ( count2 > 0 );
}

