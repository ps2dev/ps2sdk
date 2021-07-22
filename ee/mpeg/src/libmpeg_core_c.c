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

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include <ee_regs.h>

#include "libmpeg.h"
#include "libmpeg_internal.h"

#define A_EE_IPU_in_FIFO_0 (A_EE_IPU_in_FIFO + 0x0)
#define A_EE_IPU_in_FIFO_1 (A_EE_IPU_in_FIFO + 0x4)
#define A_EE_IPU_in_FIFO_2 (A_EE_IPU_in_FIFO + 0x8)
#define A_EE_IPU_in_FIFO_3 (A_EE_IPU_in_FIFO + 0xC)

#define R_EE_IPU_in_FIFO_0 ((vu32 *) A_EE_IPU_in_FIFO_0)
#define R_EE_IPU_in_FIFO_1 ((vu32 *) A_EE_IPU_in_FIFO_1)
#define R_EE_IPU_in_FIFO_2 ((vu32 *) A_EE_IPU_in_FIFO_2)
#define R_EE_IPU_in_FIFO_3 ((vu32 *) A_EE_IPU_in_FIFO_3)

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
	*R_EE_IPU_CTRL = 0x40000000;
	do {} while (*R_EE_IPU_CTRL < 0);
	*R_EE_IPU_CMD = 0;
	u32 var2;
	do {
		var2 = *R_EE_IPU_CTRL;
	} while ((s32)var2 < 0);
	*R_EE_IPU_CTRL = var2 | 0x800000;
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
	do {} while (s_CSCFlag != 0);
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
	*R_EE_D_ENABLEW = *R_EE_D_ENABLER | 0x10000;
	u32 var2 = *R_EE_D4_CHCR & 0xfffffeff;
	*R_EE_D4_CHCR = var2;
	*R_EE_D_ENABLEW = *R_EE_D_ENABLER & 0xfffffeff;
	EI();
	s_IPUState[0] = var2;
	s_IPUState[1] = *R_EE_D4_MADR;
	s_IPUState[2] = *R_EE_D4_QWC;
	do {} while ((*R_EE_IPU_CTRL & 0xf0) != 0);
	do
	{
		DI();
		EE_SYNCP();
		asm volatile ("mfc0\t%0, $12" : "=r" (eie));
		eie &= 0x10000;
	}
	while (eie != 0);
	*R_EE_D_ENABLEW = *R_EE_D_ENABLER | 0x10000;
	u32 var1 = *R_EE_D3_CHCR & 0xfffffeff;
	*R_EE_D3_CHCR = var1;
	*R_EE_D_ENABLEW = *R_EE_D_ENABLER & 0xfffffeff;
	EI();
	s_IPUState[3] = var1;
	s_IPUState[4] = *R_EE_D3_MADR;
	s_IPUState[5] = *R_EE_D3_QWC;
	s_IPUState[6] = *R_EE_IPU_CTRL;
	s_IPUState[7] = *R_EE_IPU_BP;
}

void _MPEG_Suspend ( void )
{
	do {} while (s_CSCFlag != 0);
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
		do {} while (*R_EE_IPU_CTRL < 0);
		*R_EE_IPU_CTRL = s_IPUState[6];
		*R_EE_D4_MADR = (s_IPUState[1]) + var2 * -0x10;
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
	u32 var0 = *R_EE_IPU_BP;
	u32 var1 = *R_EE_IPU_CTRL;
	if ((s32)var1 >= 0)
	{
		return;
	}
	do
	{
		do
		{
			if ((var1 & 0x4000) != 0)
			{
				return;
			}
			if ((int)((((var0 & 0xff00) >> 1) + ((var0 & 0x30000) >> 9)) - (var0 & 0x7f)) < 0x20)
			{
				break;
			}
LAB_0001041c:
			var1 = *R_EE_IPU_CTRL;
			if (-1 < *R_EE_IPU_CTRL)
			{
				return;
			}
			var0 = *R_EE_IPU_BP;
		}
		while (1);

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
		var0 = *R_EE_IPU_BP;
		var1 = *R_EE_IPU_CTRL;
	}
	while (1);
}

u32 _ipu_sync_data( void )
{
	if (-1 < *R_EE_IPU_CMD)
	{
		return *R_EE_IPU_BP;
	}
	u32 var3 = *R_EE_IPU_BP;
	do
	{
		while (0x1f < (((var3 & 0xff00) >> 1) + ((var3 & 0x30000) >> 9)) - (var3 & 0x7f))
		{
LAB_000104b8:
			if (-1 < *R_EE_IPU_CMD)
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
	if (s_DataBuf[0] < (int)arg0)
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
	if (s_DataBuf[0] < (int)arg0)
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
	_ipu_suspend();
	*R_EE_IPU_CMD = 0;
	do {} while (*R_EE_IPU_CTRL < 0);
	*R_EE_IPU_in_FIFO_0 = 0x13101008;
	*R_EE_IPU_in_FIFO_1 = 0x16161310;
	*R_EE_IPU_in_FIFO_2 = 0x16161616;
	*R_EE_IPU_in_FIFO_3 = 0x1B1A181A;
	*R_EE_IPU_in_FIFO_0 = 0x1A1A1B1B;
	*R_EE_IPU_in_FIFO_1 = 0x1B1B1A1A;
	*R_EE_IPU_in_FIFO_2 = 0x1D1D1D1B;
	*R_EE_IPU_in_FIFO_3 = 0x1D222222;
	*R_EE_IPU_in_FIFO_0 = 0x1B1B1D1D;
	*R_EE_IPU_in_FIFO_1 = 0x20201D1D;
	*R_EE_IPU_in_FIFO_2 = 0x26252222;
	*R_EE_IPU_in_FIFO_3 = 0x22232325;
	*R_EE_IPU_in_FIFO_0 = 0x28262623;
	*R_EE_IPU_in_FIFO_1 = 0x30302828;
	*R_EE_IPU_in_FIFO_2 = 0x38382E2E;
	*R_EE_IPU_in_FIFO_3 = 0x5345453A;
	*R_EE_IPU_CMD = 0x50000000;
	do {} while (*R_EE_IPU_CTRL < 0);
	*R_EE_IPU_in_FIFO_0 = 0x10101010;
	*R_EE_IPU_in_FIFO_1 = 0x10101010;
	*R_EE_IPU_in_FIFO_2 = 0x10101010;
	*R_EE_IPU_in_FIFO_3 = 0x10101010;
	*R_EE_IPU_in_FIFO_0 = 0x10101010;
	*R_EE_IPU_in_FIFO_1 = 0x10101010;
	*R_EE_IPU_in_FIFO_2 = 0x10101010;
	*R_EE_IPU_in_FIFO_3 = 0x10101010;
	*R_EE_IPU_in_FIFO_0 = 0x10101010;
	*R_EE_IPU_in_FIFO_1 = 0x10101010;
	*R_EE_IPU_in_FIFO_2 = 0x10101010;
	*R_EE_IPU_in_FIFO_3 = 0x10101010;
	*R_EE_IPU_in_FIFO_0 = 0x10101010;
	*R_EE_IPU_in_FIFO_1 = 0x10101010;
	*R_EE_IPU_in_FIFO_2 = 0x10101010;
	*R_EE_IPU_in_FIFO_3 = 0x10101010;
	*R_EE_IPU_CMD = 0x58000000;
	do {} while (*R_EE_IPU_CTRL < 0);
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
		if (var4 == 0)
		{
			return 0;
		}
		var4 &= 0xffff;
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
	return (int)var4; // TODO: check "unnecessarary bit shifts": ((long)(var4 << 0x30) >> 0x30)
}

int _MPEG_GetDMVector ( void )
{
	_ipu_sync();
	*R_EE_IPU_CMD = 0x3c000000;
	u32 var4 = _ipu_sync_data();
	s_DataBuf[0] = 0x20;
	s_DataBuf[1] = *R_EE_IPU_TOP;
	return (int)var4; // TODO: check "unnecessarary bit shifts": ((long)(var4 << 0x30) >> 0x30)
}

void _MPEG_SetIDCP ( void )
{
	unsigned int var1 = _MPEG_GetBits(2);
	*R_EE_IPU_CTRL = (*R_EE_IPU_CTRL & 0xfffcffff) | var1 << 0x10;
}

void _MPEG_SetQSTIVFAS ( void )
{
	unsigned int var1 = _MPEG_GetBits(1);
	unsigned int var2 = _MPEG_GetBits(1);
	unsigned int var3 = _MPEG_GetBits(1);
	*R_EE_IPU_CTRL = (*R_EE_IPU_CTRL & 0xff8fffff) | var1 << 0x16 | var2 << 0x15 | var3 << 0x14;
}

void _MPEG_SetPCT ( unsigned int arg0 )
{
	u32 var3 = *R_EE_IPU_CTRL;
	if (-1 < (int)var3)
	{
		*R_EE_IPU_CTRL = (var3 & 0xf8ffffff) | arg0 << 0x18;
		return;
	}
	// TODO: validate. Bugged and in wrong place?
	_ipu_sync();
}

void _MPEG_BDEC ( int arg0, int arg1, int arg2, int arg3, void* arg4 )
{
	*R_EE_D3_MADR = ((uint)arg4 & 0xfffffff) | 0x80000000;
	*R_EE_D3_QWC = 0x30;
	*R_EE_D3_CHCR = 0x100;
	_ipu_sync();
	*R_EE_IPU_CMD = arg0 << 0x1b | 0x20000000U | arg1 << 0x1a | arg2 << 0x19 | arg3 << 0x10;
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
			// XXX: check CONCAT44 order
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
	*R_EE_D_ENABLEW = *R_EE_D_ENABLER & 0xfffeffff;
	EI();
	*R_EE_D3_QWC = 0;
	s_DataBuf[0] = 0;
	s_DataBuf[1] = 0;
	return 0;
}

void _MPEG_dma_ref_image ( _MPEGMacroBlock8* arg0, _MPEGMotion* arg1, int arg2, int arg3 )
{
	u8* var00 = (u8*)arg0;
	_MPEGMotion* var01 = (_MPEGMotion*)arg1;
	u32 var3 = 4;
	if (arg2 < 5)
	{
		var3 = arg2;
	}
	u64 var5 = (ulong)var3;
	if (arg2 >> 0x1f < 1)
	{
		// TODO: correct implementation of CONCAT44?
		var5 = ((u64)(arg2 >> 0x1f) << 32) | var3;
	}
	if (0 < var5)
	{
		do {} while ((*R_EE_D9_CHCR & 0x100) != 0);
		*R_EE_D9_QWC = 0;
		*R_EE_D9_SADR = (u32)arg0 & 0xfffffff;
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

void _MPEG_put_block_fr ( _MPEGMotions* arg0 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"lw      %[tmp9], 0(%[arg0])\n"
		"lw      %[tmp10], 8(%[arg0])\n"
		"pnor    $v0, $zero, $zero\n"
		"addiu   $v1, $zero, 6\n"
		"psrlh   $v0, $v0, 8\n"
	"1:\n"
		"lq      %[tmp0],   0(%[tmp10])\n"
		"lq      %[tmp1],  16(%[tmp10])\n"
		"lq      %[tmp2],  32(%[tmp10])\n"
		"lq      %[tmp3],  48(%[tmp10])\n"
		"addiu   $v1, $v1, -1;\n"
		"lq      %[tmp4],  64(%[tmp10])\n"
		"lq      %[tmp5],  80(%[tmp10])\n"
		"lq      %[tmp6],  96(%[tmp10])\n"
		"lq      %[tmp7], 112(%[tmp10])\n"
		"addiu   %[tmp10], %[tmp10], 128\n"
		"pmaxh   %[tmp0], $zero, %[tmp0]\n"
		"pmaxh   %[tmp1], $zero, %[tmp1]\n"
		"pmaxh   %[tmp2], $zero, %[tmp2]\n"
		"pmaxh   %[tmp3], $zero, %[tmp3]\n"
		"pmaxh   %[tmp4], $zero, %[tmp4]\n"
		"pmaxh   %[tmp5], $zero, %[tmp5]\n"
		"pmaxh   %[tmp6], $zero, %[tmp6]\n"
		"pmaxh   %[tmp7], $zero, %[tmp7]\n"
		"pminh   %[tmp0], $v0, %[tmp0]\n"
		"pminh   %[tmp1], $v0, %[tmp1]\n"
		"pminh   %[tmp2], $v0, %[tmp2]\n"
		"pminh   %[tmp3], $v0, %[tmp3]\n"
		"pminh   %[tmp4], $v0, %[tmp4]\n"
		"pminh   %[tmp5], $v0, %[tmp5]\n"
		"pminh   %[tmp6], $v0, %[tmp6]\n"
		"pminh   %[tmp7], $v0, %[tmp7]\n"
		"ppacb   %[tmp0], %[tmp1], %[tmp0]\n"
		"ppacb   %[tmp2], %[tmp3], %[tmp2]\n"
		"ppacb   %[tmp4], %[tmp5], %[tmp4]\n"
		"ppacb   %[tmp6], %[tmp7], %[tmp6]\n"
		"sq      %[tmp0],  0(%[tmp9])\n"
		"sq      %[tmp2], 16(%[tmp9])\n"
		"sq      %[tmp4], 32(%[tmp9])\n"
		"sq      %[tmp6], 48(%[tmp9])\n"
		"bgtzl   $v1, 1b\n"
		"addiu   %[tmp9], %[tmp9], 64\n"
	: [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg0] "r"(arg0)
	: "a1", "v0", "v1", "memory"
	);
}

void _MPEG_put_block_fl ( _MPEGMotions* arg0 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"pnor    $v0, $zero, $zero\n"
		"lw      %[tmp9], 0(%[arg0])\n"
		"lw      %[tmp10], 8(%[arg0])\n"
		"addiu   $v1, $zero, 4\n"
		"psrlh   $v0, $v0, 8\n"
	"1:\n"
		"lq      %[tmp0],   0(%[tmp10])\n"
		"lq      %[tmp1],  16(%[tmp10])\n"
		"lq      %[tmp2],  32(%[tmp10])\n"
		"lq      %[tmp3],  48(%[tmp10])\n"
		"addiu   $v1, $v1, -1\n"
		"lq      %[tmp4], 256(%[tmp10])\n"
		"lq      %[tmp5], 272(%[tmp10])\n"
		"lq      %[tmp6], 288(%[tmp10])\n"
		"lq      %[tmp7], 304(%[tmp10])\n"
		"addiu   %[tmp10], %[tmp10], 64\n"
		"pmaxh   %[tmp0], $zero, %[tmp0]\n"
		"pmaxh   %[tmp1], $zero, %[tmp1]\n"
		"pmaxh   %[tmp2], $zero, %[tmp2]\n"
		"pmaxh   %[tmp3], $zero, %[tmp3]\n"
		"pmaxh   %[tmp4], $zero, %[tmp4]\n"
		"pmaxh   %[tmp5], $zero, %[tmp5]\n"
		"pmaxh   %[tmp6], $zero, %[tmp6]\n"
		"pmaxh   %[tmp7], $zero, %[tmp7]\n"
		"pminh   %[tmp0], $v0, %[tmp0]\n"
		"pminh   %[tmp1], $v0, %[tmp1]\n"
		"pminh   %[tmp2], $v0, %[tmp2]\n"
		"pminh   %[tmp3], $v0, %[tmp3]\n"
		"pminh   %[tmp4], $v0, %[tmp4]\n"
		"pminh   %[tmp5], $v0, %[tmp5]\n"
		"pminh   %[tmp6], $v0, %[tmp6]\n"
		"pminh   %[tmp7], $v0, %[tmp7]\n"
		"ppacb   %[tmp0], %[tmp1], %[tmp0]\n"
		"ppacb   %[tmp2], %[tmp3], %[tmp2]\n"
		"ppacb   %[tmp4], %[tmp5], %[tmp4]\n"
		"ppacb   %[tmp6], %[tmp7], %[tmp6]\n"
		"sq      %[tmp0],  0(%[tmp9])\n"
		"sq      %[tmp4], 16(%[tmp9])\n"
		"sq      %[tmp2], 32(%[tmp9])\n"
		"sq      %[tmp6], 48(%[tmp9])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[tmp9], %[tmp9], 64\n"
		"addiu   $v1, $v1, 2\n"
	"2:\n"
		"lq      %[tmp0], 256(%[tmp10])\n"
		"lq      %[tmp1], 272(%[tmp10])\n"
		"lq      %[tmp2], 288(%[tmp10])\n"
		"lq      %[tmp3], 304(%[tmp10])\n"
		"addiu   $v1, $v1, -1\n"
		"lq      %[tmp4], 320(%[tmp10])\n"
		"lq      %[tmp5], 336(%[tmp10])\n"
		"lq      %[tmp6], 352(%[tmp10])\n"
		"lq      %[tmp7], 368(%[tmp10])\n"
		"addiu   %[tmp10], %[tmp10], 128\n"
		"pmaxh   %[tmp0], $zero, %[tmp0]\n"
		"pmaxh   %[tmp1], $zero, %[tmp1]\n"
		"pmaxh   %[tmp2], $zero, %[tmp2]\n"
		"pmaxh   %[tmp3], $zero, %[tmp3]\n"
		"pmaxh   %[tmp4], $zero, %[tmp4]\n"
		"pmaxh   %[tmp5], $zero, %[tmp5]\n"
		"pmaxh   %[tmp6], $zero, %[tmp6]\n"
		"pmaxh   %[tmp7], $zero, %[tmp7]\n"
		"pminh   %[tmp0], $v0, %[tmp0]\n"
		"pminh   %[tmp1], $v0, %[tmp1]\n"
		"pminh   %[tmp2], $v0, %[tmp2]\n"
		"pminh   %[tmp3], $v0, %[tmp3]\n"
		"pminh   %[tmp4], $v0, %[tmp4]\n"
		"pminh   %[tmp5], $v0, %[tmp5]\n"
		"pminh   %[tmp6], $v0, %[tmp6]\n"
		"pminh   %[tmp7], $v0, %[tmp7]\n"
		"ppacb   %[tmp0], %[tmp1], %[tmp0]\n"
		"ppacb   %[tmp2], %[tmp3], %[tmp2]\n"
		"ppacb   %[tmp4], %[tmp5], %[tmp4]\n"
		"ppacb   %[tmp6], %[tmp7], %[tmp6]\n"
		"sq      %[tmp0],  0(%[tmp9])\n"
		"sq      %[tmp2], 16(%[tmp9])\n"
		"sq      %[tmp4], 32(%[tmp9])\n"
		"sq      %[tmp6], 48(%[tmp9])\n"
		"bgtzl   $v1, 2b\n"
		"addiu   %[tmp9], %[tmp9], 64\n"
	: [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg0] "r"(arg0)
	: "a1", "v0", "v1", "memory"
	);
}

void _MPEG_put_block_il ( _MPEGMotions* arg0 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"pnor    $v0, $zero, $zero\n"
		"lw      %[tmp9],  0(%[arg0])\n"
		"lw      %[tmp10],  8(%[arg0])\n"
		"lw      %[tmp8], 24(%[arg0])\n"
		"addiu   $v1, $zero, 4\n"
		"psrlh   $v0, $v0, 8\n"
		"addu    %[tmp8], %[tmp8], %[tmp9]\n"
	"1:\n"
		"lq      %[tmp0],   0(%[tmp10])\n"
		"lq      %[tmp1],  16(%[tmp10])\n"
		"lq      %[tmp2],  32(%[tmp10])\n"
		"lq      %[tmp3],  48(%[tmp10])\n"
		"addiu   $v1, $v1, -1\n"
		"lq      %[tmp4], 256(%[tmp10])\n"
		"lq      %[tmp5], 272(%[tmp10])\n"
		"lq      %[tmp6], 288(%[tmp10])\n"
		"lq      %[tmp7], 304(%[tmp10])\n"
		"addiu   %[tmp10], %[tmp10], 64\n"
		"pmaxh   %[tmp0], $zero, %[tmp0]\n"
		"pmaxh   %[tmp1], $zero, %[tmp1]\n"
		"pmaxh   %[tmp2], $zero, %[tmp2]\n"
		"pmaxh   %[tmp3], $zero, %[tmp3]\n"
		"pmaxh   %[tmp4], $zero, %[tmp4]\n"
		"pmaxh   %[tmp5], $zero, %[tmp5]\n"
		"pmaxh   %[tmp6], $zero, %[tmp6]\n"
		"pmaxh   %[tmp7], $zero, %[tmp7]\n"
		"pminh   %[tmp0], $v0, %[tmp0]\n"
		"pminh   %[tmp1], $v0, %[tmp1]\n"
		"pminh   %[tmp2], $v0, %[tmp2]\n"
		"pminh   %[tmp3], $v0, %[tmp3]\n"
		"pminh   %[tmp4], $v0, %[tmp4]\n"
		"pminh   %[tmp5], $v0, %[tmp5]\n"
		"pminh   %[tmp6], $v0, %[tmp6]\n"
		"pminh   %[tmp7], $v0, %[tmp7]\n"
		"ppacb   %[tmp0], %[tmp1], %[tmp0]\n"
		"ppacb   %[tmp2], %[tmp3], %[tmp2]\n"
		"ppacb   %[tmp4], %[tmp5], %[tmp4]\n"
		"ppacb   %[tmp6], %[tmp7], %[tmp6]\n"
		"sq      %[tmp0],  0(%[tmp9])\n"
		"sq      %[tmp2], 32(%[tmp9])\n"
		"addiu   %[tmp9], %[tmp9], 64\n"
		"sq      %[tmp4],  0(%[tmp8])\n"
		"sq      %[tmp6], 32(%[tmp8])\n"
		"bgtzl   $v1, 1b\n"
		"addiu   %[tmp8], %[tmp8], 64\n"
		"lw      %[tmp9],  4(%[arg0])\n"
		"lw      %[tmp8], 24(%[arg0])\n"
		"addiu   $v1, $zero, 2\n"
		"addu    %[tmp8], %[tmp8], %[tmp9]\n"
	"2:\n"
		"lq      %[tmp0], 256(%[tmp10])\n"
		"lq      %[tmp1], 272(%[tmp10])\n"
		"lq      %[tmp2], 288(%[tmp10])\n"
		"lq      %[tmp3], 304(%[tmp10])\n"
		"addiu   $v1, $v1, -1\n"
		"lq      %[tmp4], 320(%[tmp10])\n"
		"lq      %[tmp5], 336(%[tmp10])\n"
		"lq      %[tmp6], 352(%[tmp10])\n"
		"lq      %[tmp7], 368(%[tmp10])\n"
		"addiu   %[tmp10], %[tmp10], 128\n"
		"pmaxh   %[tmp0], $zero, %[tmp0]\n"
		"pmaxh   %[tmp1], $zero, %[tmp1]\n"
		"pmaxh   %[tmp2], $zero, %[tmp2]\n"
		"pmaxh   %[tmp3], $zero, %[tmp3]\n"
		"pmaxh   %[tmp4], $zero, %[tmp4]\n"
		"pmaxh   %[tmp5], $zero, %[tmp5]\n"
		"pmaxh   %[tmp6], $zero, %[tmp6]\n"
		"pmaxh   %[tmp7], $zero, %[tmp7]\n"
		"pminh   %[tmp0], $v0, %[tmp0]\n"
		"pminh   %[tmp1], $v0, %[tmp1]\n"
		"pminh   %[tmp2], $v0, %[tmp2]\n"
		"pminh   %[tmp3], $v0, %[tmp3]\n"
		"pminh   %[tmp4], $v0, %[tmp4]\n"
		"pminh   %[tmp5], $v0, %[tmp5]\n"
		"pminh   %[tmp6], $v0, %[tmp6]\n"
		"pminh   %[tmp7], $v0, %[tmp7]\n"
		"ppacb   %[tmp0], $zero, %[tmp0]\n"
		"ppacb   %[tmp1], $zero, %[tmp1]\n"
		"ppacb   %[tmp2], $zero, %[tmp2]\n"
		"ppacb   %[tmp3], $zero, %[tmp3]\n"
		"ppacb   %[tmp4], $zero, %[tmp4]\n"
		"ppacb   %[tmp5], $zero, %[tmp5]\n"
		"ppacb   %[tmp6], $zero, %[tmp6]\n"
		"ppacb   %[tmp7], $zero, %[tmp7]\n"
		"sd      %[tmp0],  0(%[tmp9])\n"
		"sd      %[tmp1], 16(%[tmp9])\n"
		"sd      %[tmp2], 32(%[tmp9])\n"
		"sd      %[tmp3], 48(%[tmp9])\n"
		"sd      %[tmp4],  0(%[tmp8])\n"
		"sd      %[tmp5], 16(%[tmp8])\n"
		"sd      %[tmp6], 32(%[tmp8])\n"
		"sd      %[tmp7], 48(%[tmp8])\n"
		"addiu   %[tmp9], %[tmp9], 64\n"
		"bgtzl   $v1, 2b\n"
		"addiu   %[tmp8], %[tmp8], 64\n"
	: [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg0] "r"(arg0)
	: "a1", "v0", "v1", "memory"
	);
}

void _MPEG_add_block_frfr ( _MPEGMotions* arg0 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"pnor    $v0, $zero, $zero\n"
		"lw      %[tmp9],  0(%[arg0])\n"
		"lw      %[tmp10], 12(%[arg0])\n"
		"lw      $a1, 16($a1)\n"
		"addiu   $v1, $zero, 6\n"
		"psrlh   $v0, $v0, 8\n"
	"1:\n"
		"lq      %[tmp0],   0(%[tmp10])\n"
		"lq      %[tmp1],  16(%[tmp10])\n"
		"lq      %[tmp2],  32(%[tmp10])\n"
		"lq      %[tmp3],  48(%[tmp10])\n"
		"addiu   $v1, $v1, -1\n"
		"lq      %[tmp4],   0($a1)\n"
		"lq      %[tmp5],  16($a1)\n"
		"lq      %[tmp6],  32($a1)\n"
		"lq      %[tmp7],  48($a1)\n"
		"paddh   %[tmp0], %[tmp0], %[tmp4]\n"
		"paddh   %[tmp1], %[tmp1], %[tmp5]\n"
		"paddh   %[tmp2], %[tmp2], %[tmp6]\n"
		"paddh   %[tmp3], %[tmp3], %[tmp7]\n"
		"pmaxh   %[tmp0], $zero, %[tmp0]\n"
		"pmaxh   %[tmp1], $zero, %[tmp1]\n"
		"pmaxh   %[tmp2], $zero, %[tmp2]\n"
		"pmaxh   %[tmp3], $zero, %[tmp3]\n"
		"pminh   %[tmp0], $v0, %[tmp0]\n"
		"pminh   %[tmp1], $v0, %[tmp1]\n"
		"pminh   %[tmp2], $v0, %[tmp2]\n"
		"pminh   %[tmp3], $v0, %[tmp3]\n"
		"ppacb   %[tmp0], %[tmp1], %[tmp0]\n"
		"ppacb   %[tmp2], %[tmp3], %[tmp2]\n"
		"sq      %[tmp0],  0(%[tmp9])\n"
		"sq      %[tmp2], 16(%[tmp9])\n"
		"lq      %[tmp4],  64(%[tmp10])\n"
		"lq      %[tmp5],  80(%[tmp10])\n"
		"lq      %[tmp6],  96(%[tmp10])\n"
		"lq      %[tmp7], 112(%[tmp10])\n"
		"addiu   %[tmp10], %[tmp10], 128\n"
		"lq      %[tmp0],  64($a1)\n"
		"lq      %[tmp1],  80($a1)\n"
		"lq      %[tmp2],  96($a1)\n"
		"lq      %[tmp3], 112($a1)\n"
		"addiu   $a1, $a1, 128\n"
		"paddh   %[tmp4], %[tmp4], %[tmp0]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp1]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp2]\n"
		"paddh   %[tmp7], %[tmp7], %[tmp3]\n"
		"pmaxh   %[tmp4], $zero, %[tmp4]\n"
		"pmaxh   %[tmp5], $zero, %[tmp5]\n"
		"pmaxh   %[tmp6], $zero, %[tmp6]\n"
		"pmaxh   %[tmp7], $zero, %[tmp7]\n"
		"pminh   %[tmp4], $v0, %[tmp4]\n"
		"pminh   %[tmp5], $v0, %[tmp5]\n"
		"pminh   %[tmp6], $v0, %[tmp6]\n"
		"pminh   %[tmp7], $v0, %[tmp7]\n"
		"ppacb   %[tmp4], %[tmp5], %[tmp4]\n"
		"ppacb   %[tmp6], %[tmp7], %[tmp6]\n"
		"sq      %[tmp4], 32(%[tmp9])\n"
		"sq      %[tmp6], 48(%[tmp9])\n"
		"bgtzl   $v1, 1b\n"
		"addiu   %[tmp9], %[tmp9], 64\n"
	: [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg0] "r"(arg0)
	: "a1", "v0", "v1", "memory"
	);
}

void _MPEG_add_block_ilfl ( _MPEGMotions* arg0 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"pnor    $v0, $zero, $zero\n"
		"lw      %[tmp9],  0(%[arg0])\n"
		"lw      %[tmp10], 12(%[arg0])\n"
		"lw      %[tmp8], 24(%[arg0])\n"
		"lw      $a1, 16(%[arg0])\n"
		"addiu   $v1, $zero, 4\n"
		"psrlh   $v0, $v0, 8\n"
		"addu    %[tmp8], %[tmp8], %[tmp9]\n"
	"1:\n"
		"lq      %[tmp0],   0(%[tmp10])\n"
		"lq      %[tmp1],  16(%[tmp10])\n"
		"lq      %[tmp2],  32(%[tmp10])\n"
		"lq      %[tmp3],  48(%[tmp10])\n"
		"addiu   $v1, $v1, -1\n"
		"lq      %[tmp4],   0($a1)\n"
		"lq      %[tmp5],  16($a1)\n"
		"lq      %[tmp6],  32($a1)\n"
		"lq      %[tmp7],  48($a1)\n"
		"paddh   %[tmp0], %[tmp0], %[tmp4]\n"
		"paddh   %[tmp1], %[tmp1], %[tmp5]\n"
		"paddh   %[tmp2], %[tmp2], %[tmp6]\n"
		"paddh   %[tmp3], %[tmp3], %[tmp7]\n"
		"pmaxh   %[tmp0], $zero, %[tmp0]\n"
		"pmaxh   %[tmp1], $zero, %[tmp1]\n"
		"pmaxh   %[tmp2], $zero, %[tmp2]\n"
		"pmaxh   %[tmp3], $zero, %[tmp3]\n"
		"pminh   %[tmp0], $v0, %[tmp0]\n"
		"pminh   %[tmp1], $v0, %[tmp1]\n"
		"pminh   %[tmp2], $v0, %[tmp2]\n"
		"pminh   %[tmp3], $v0, %[tmp3]\n"
		"ppacb   %[tmp0], %[tmp1], %[tmp0]\n"
		"ppacb   %[tmp2], %[tmp3], %[tmp2]\n"
		"sq      %[tmp0],   0(%[tmp9])\n"
		"sq      %[tmp2],  32(%[tmp9])\n"
		"lq      %[tmp4], 256(%[tmp10])\n"
		"lq      %[tmp5], 272(%[tmp10])\n"
		"lq      %[tmp6], 288(%[tmp10])\n"
		"lq      %[tmp7], 304(%[tmp10])\n"
		"addiu   %[tmp10], %[tmp10], 64\n"
		"lq      %[tmp0], 256($a1)\n"
		"lq      %[tmp1], 272($a1)\n"
		"lq      %[tmp2], 288($a1)\n"
		"lq      %[tmp3], 304($a1)\n"
		"paddh   %[tmp4], %[tmp4], %[tmp0]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp1]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp2]\n"
		"paddh   %[tmp7], %[tmp7], %[tmp3]\n"
		"pmaxh   %[tmp4], $zero, %[tmp4]\n"
		"pmaxh   %[tmp5], $zero, %[tmp5]\n"
		"pmaxh   %[tmp6], $zero, %[tmp6]\n"
		"pmaxh   %[tmp7], $zero, %[tmp7]\n"
		"pminh   %[tmp4], $v0, %[tmp4]\n"
		"pminh   %[tmp5], $v0, %[tmp5]\n"
		"pminh   %[tmp6], $v0, %[tmp6]\n"
		"pminh   %[tmp7], $v0, %[tmp7]\n"
		"ppacb   %[tmp4], %[tmp5], %[tmp4]\n"
		"ppacb   %[tmp6], %[tmp7], %[tmp6]\n"
		"sq      %[tmp4],  0(%[tmp8])\n"
		"sq      %[tmp6], 32(%[tmp8])\n"
		"addiu   %[tmp8], %[tmp8], 64\n"
		"addiu   $a1, $a1, 64\n"
		"bgtzl   $v1, 1b\n"
		"addiu   %[tmp9], %[tmp9], 64\n"
		"lw      %[tmp9],  4(%[arg0])\n"
		"lw      %[tmp8], 24(%[arg0])\n"
		"addiu   $v1, $zero, 2\n"
		"addu    %[tmp8], %[tmp8], %[tmp9]\n"
	"2:\n"
		"lq      %[tmp0], 256(%[tmp10])\n"
		"lq      %[tmp1], 272(%[tmp10])\n"
		"lq      %[tmp2], 288(%[tmp10])\n"
		"lq      %[tmp3], 304(%[tmp10])\n"
		"addiu   $v1, $v1, -1\n"
		"lq      %[tmp4], 256($a1)\n"
		"lq      %[tmp5], 272($a1)\n"
		"lq      %[tmp6], 288($a1)\n"
		"lq      %[tmp7], 304($a1)\n"
		"paddh   %[tmp0], %[tmp0], %[tmp4]\n"
		"paddh   %[tmp1], %[tmp1], %[tmp5]\n"
		"paddh   %[tmp2], %[tmp2], %[tmp6]\n"
		"paddh   %[tmp3], %[tmp3], %[tmp7]\n"
		"pmaxh   %[tmp0], $zero, %[tmp0]\n"
		"pmaxh   %[tmp1], $zero, %[tmp1]\n"
		"pmaxh   %[tmp2], $zero, %[tmp2]\n"
		"pmaxh   %[tmp3], $zero, %[tmp3]\n"
		"pminh   %[tmp0], $v0, %[tmp0]\n"
		"pminh   %[tmp1], $v0, %[tmp1]\n"
		"pminh   %[tmp2], $v0, %[tmp2]\n"
		"pminh   %[tmp3], $v0, %[tmp3]\n"
		"ppacb   %[tmp0], $zero, %[tmp0]\n"
		"ppacb   %[tmp1], $zero, %[tmp1]\n"
		"ppacb   %[tmp2], $zero, %[tmp2]\n"
		"ppacb   %[tmp3], $zero, %[tmp3]\n"
		"sd      %[tmp0],  0(%[tmp9])\n"
		"sd      %[tmp1], 16(%[tmp9])\n"
		"sd      %[tmp2], 32(%[tmp9])\n"
		"sd      %[tmp3], 48(%[tmp9])\n"
		"lq      %[tmp4], 320(%[tmp10])\n"
		"lq      %[tmp5], 336(%[tmp10])\n"
		"lq      %[tmp6], 352(%[tmp10])\n"
		"lq      %[tmp7], 368(%[tmp10])\n"
		"addiu   %[tmp10], %[tmp10], 128\n"
		"lq      %[tmp0], 320($a1)\n"
		"lq      %[tmp1], 336($a1)\n"
		"lq      %[tmp2], 352($a1)\n"
		"lq      %[tmp3], 368($a1)\n"
		"paddh   %[tmp4], %[tmp4], %[tmp0]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp1]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp2]\n"
		"paddh   %[tmp7], %[tmp7], %[tmp3]\n"
		"pmaxh   %[tmp4], $zero, %[tmp4]\n"
		"pmaxh   %[tmp5], $zero, %[tmp5]\n"
		"pmaxh   %[tmp6], $zero, %[tmp6]\n"
		"pmaxh   %[tmp7], $zero, %[tmp7]\n"
		"pminh   %[tmp4], $v0, %[tmp4]\n"
		"pminh   %[tmp5], $v0, %[tmp5]\n"
		"pminh   %[tmp6], $v0, %[tmp6]\n"
		"pminh   %[tmp7], $v0, %[tmp7]\n"
		"ppacb   %[tmp4], $zero, %[tmp4]\n"
		"ppacb   %[tmp5], $zero, %[tmp5]\n"
		"ppacb   %[tmp6], $zero, %[tmp6]\n"
		"ppacb   %[tmp7], $zero, %[tmp7]\n"
		"sd      %[tmp4],  0(%[tmp8])\n"
		"sd      %[tmp5], 16(%[tmp8])\n"
		"sd      %[tmp6], 32(%[tmp8])\n"
		"sd      %[tmp7], 48(%[tmp8])\n"
		"addiu   %[tmp9], %[tmp9], 64\n"
		"addiu   %[tmp8], %[tmp8], 64\n"
		"bgtzl   $v1, 2b\n"
		"addiu   $a1, $a1, 128\n"
	: [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg0] "r"(arg0)
	: "a1", "v0", "v1", "memory"
	);
}

void _MPEG_add_block_frfl ( _MPEGMotions* arg0 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"pnor    $v0, $zero, $zero\n"
		"lw      %[tmp9],  0(%[arg0])\n"
		"lw      %[tmp10], 12(%[arg0])\n"
		"lw      $a1, 16(%[arg0])\n"
		"addiu   $v1, $zero, 4\n"
		"psrlh   $v0, $v0, 8\n"
	"1:\n"
		"lq      %[tmp0],   0(%[tmp10])\n"
		"lq      %[tmp1],  16(%[tmp10])\n"
		"lq      %[tmp2],  32(%[tmp10])\n"
		"lq      %[tmp3],  48(%[tmp10])\n"
		"addiu   $v1, $v1, -1\n"
		"lq      %[tmp4],   0($a1)\n"
		"lq      %[tmp5],  16($a1)\n"
		"lq      %[tmp6], 256($a1)\n"
		"lq      %[tmp7], 272($a1)\n"
		"paddh   %[tmp0], %[tmp0], %[tmp4]\n"
		"paddh   %[tmp1], %[tmp1], %[tmp5]\n"
		"paddh   %[tmp2], %[tmp2], %[tmp6]\n"
		"paddh   %[tmp3], %[tmp3], %[tmp7]\n"
		"pmaxh   %[tmp0], $zero, %[tmp0]\n"
		"pmaxh   %[tmp1], $zero, %[tmp1]\n"
		"pmaxh   %[tmp2], $zero, %[tmp2]\n"
		"pmaxh   %[tmp3], $zero, %[tmp3]\n"
		"pminh   %[tmp0], $v0, %[tmp0]\n"
		"pminh   %[tmp1], $v0, %[tmp1]\n"
		"pminh   %[tmp2], $v0, %[tmp2]\n"
		"pminh   %[tmp3], $v0, %[tmp3]\n"
		"ppacb   %[tmp0], %[tmp1], %[tmp0]\n"
		"ppacb   %[tmp2], %[tmp3], %[tmp2]\n"
		"sq      %[tmp0],   0(%[tmp9])\n"
		"sq      %[tmp2],  16(%[tmp9])\n"
		"lq      %[tmp4],  64(%[tmp10])\n"
		"lq      %[tmp5],  80(%[tmp10])\n"
		"lq      %[tmp6],  96(%[tmp10])\n"
		"lq      %[tmp7], 112(%[tmp10])\n"
		"addiu   %[tmp10], %[tmp10], 128\n"
		"lq      %[tmp0],  32($a1)\n"
		"lq      %[tmp1],  48($a1)\n"
		"lq      %[tmp2], 288($a1)\n"
		"lq      %[tmp3], 304($a1)\n"
		"paddh   %[tmp4], %[tmp4], %[tmp0]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp1]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp2]\n"
		"paddh   %[tmp7], %[tmp7], %[tmp3]\n"
		"pmaxh   %[tmp4], $zero, %[tmp4]\n"
		"pmaxh   %[tmp5], $zero, %[tmp5]\n"
		"pmaxh   %[tmp6], $zero, %[tmp6]\n"
		"pmaxh   %[tmp7], $zero, %[tmp7]\n"
		"pminh   %[tmp4], $v0, %[tmp4]\n"
		"pminh   %[tmp5], $v0, %[tmp5]\n"
		"pminh   %[tmp6], $v0, %[tmp6]\n"
		"pminh   %[tmp7], $v0, %[tmp7]\n"
		"ppacb   %[tmp4], %[tmp5], %[tmp4]\n"
		"ppacb   %[tmp6], %[tmp7], %[tmp6]\n"
		"sq      %[tmp4], 32(%[tmp9])\n"
		"sq      %[tmp6], 48(%[tmp9])\n"
		"addiu   $a1, $a1, 64\n"
		"bgtzl   $v1, 1b\n"
		"addiu   %[tmp9], %[tmp9], 64\n"
		"lw      %[tmp9], 4(%[arg0])\n"
		"addiu   $v1, $zero, 2\n"
	"2:\n"
		"lq      %[tmp0],   0(%[tmp10])\n"
		"lq      %[tmp1],  16(%[tmp10])\n"
		"lq      %[tmp2],  32(%[tmp10])\n"
		"lq      %[tmp3],  48(%[tmp10])\n"
		"addiu   $v1, $v1, -1\n"
		"lq      %[tmp4], 256($a1)\n"
		"lq      %[tmp5], 320($a1)\n"
		"lq      %[tmp6], 272($a1)\n"
		"lq      %[tmp7], 336($a1)\n"
		"paddh   %[tmp0], %[tmp0], %[tmp4]\n"
		"paddh   %[tmp1], %[tmp1], %[tmp5]\n"
		"paddh   %[tmp2], %[tmp2], %[tmp6]\n"
		"paddh   %[tmp3], %[tmp3], %[tmp7]\n"
		"pmaxh   %[tmp0], $zero, %[tmp0]\n"
		"pmaxh   %[tmp1], $zero, %[tmp1]\n"
		"pmaxh   %[tmp2], $zero, %[tmp2]\n"
		"pmaxh   %[tmp3], $zero, %[tmp3]\n"
		"pminh   %[tmp0], $v0, %[tmp0]\n"
		"pminh   %[tmp1], $v0, %[tmp1]\n"
		"pminh   %[tmp2], $v0, %[tmp2]\n"
		"pminh   %[tmp3], $v0, %[tmp3]\n"
		"ppacb   %[tmp0], %[tmp1], %[tmp0]\n"
		"ppacb   %[tmp2], %[tmp3], %[tmp2]\n"
		"sq      %[tmp0],  0(%[tmp9])\n"
		"sq      %[tmp2], 16(%[tmp9])\n"
		"lq      %[tmp4],  64(%[tmp10])\n"
		"lq      %[tmp5],  80(%[tmp10])\n"
		"lq      %[tmp6],  96(%[tmp10])\n"
		"lq      %[tmp7], 112(%[tmp10])\n"
		"addiu   %[tmp10], %[tmp10], 128\n"
		"lq      %[tmp0], 288($a1)\n"
		"lq      %[tmp1], 352($a1)\n"
		"lq      %[tmp2], 304($a1)\n"
		"lq      %[tmp3], 368($a1)\n"
		"paddh   %[tmp4], %[tmp4], %[tmp0]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp1]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp2]\n"
		"paddh   %[tmp7], %[tmp7], %[tmp3]\n"
		"pmaxh   %[tmp4], $zero, %[tmp4]\n"
		"pmaxh   %[tmp5], $zero, %[tmp5]\n"
		"pmaxh   %[tmp6], $zero, %[tmp6]\n"
		"pmaxh   %[tmp7], $zero, %[tmp7]\n"
		"pminh   %[tmp4], $v0, %[tmp4]\n"
		"pminh   %[tmp5], $v0, %[tmp5]\n"
		"pminh   %[tmp6], $v0, %[tmp6]\n"
		"pminh   %[tmp7], $v0, %[tmp7]\n"
		"ppacb   %[tmp4], %[tmp5], %[tmp4]\n"
		"ppacb   %[tmp6], %[tmp7], %[tmp6]\n"
		"sq      %[tmp4], 32(%[tmp9])\n"
		"sq      %[tmp6], 48(%[tmp9])\n"
		"addiu   %[tmp9], %[tmp9], 64\n"
		"bgtzl   $v1, 2b\n"
		"addiu   $a1, $a1, 128\n"
	: [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg0] "r"(arg0)
	: "a1", "v0", "v1", "memory"
	);
}

// TODO: verify delay slots
void _MPEG_do_mc ( _MPEGMotion* arg0 )
{
	unsigned char* arg1 = arg0->m_pSrc;
	short* arg2 = arg0->m_pDstY;
	int arg3 = arg0->m_X;
	int tmp0 = arg0->m_Y;
	int tmp1 = arg0->m_H;
	int tmp2 = arg0->m_fInt;
	int tmp4 = arg0->m_Field;
	tmp0 -= tmp4;
	tmp4 <<= 4;
	arg1 += tmp4;
	int var1 = 16 - tmp0;
	int tmp3 = 16 << tmp2;
	var1 <<= tmp2;
	int tmpa = tmp0 << 4;
	arg1 += tmpa;
	arg0->MC_Luma(arg1, arg2, arg3, tmp3);
	tmpa = tmp1 - var1;
	arg1 = arg0->m_pSrc;
	arg2 = arg0->m_pDstCbCr;
	arg1 += 256;
	tmp4 >>= 1;
	arg3 >>= 1;
	tmp0 >>= 1;
	tmp1 >>= 1;
	tmp0 >>= tmp2;
	arg1 += tmp4;
	tmp0 <<= tmp2;
	var1 = 8 - tmp0;
	tmp3 = 8 << tmp2;
	var1 >>= tmp2;
	tmpa = tmp0 << 3;
	arg1 += tmpa;
	tmpa = tmp1 - var1;
	arg0->MC_Chroma(arg1, arg2, arg3, tmp3);
}


void _MPEG_put_luma ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"mtsab   %[arg3], 0\n"
	"1:\n"
		"lq      %[tmp5],   0(%[arg1])\n"
		"lq      %[tmp6], 384(%[arg1])\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"qfsrv   %[tmp5], %[tmp6], %[tmp5]\n"
		"pextlb  %[tmp6], $zero, %[tmp5]\n"
		"pextub  %[tmp5], $zero, %[tmp5]\n"
		"sq      %[tmp6],  0(%[arg2])\n"
		"sq      %[tmp5], 16(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 32\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 512\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

void _MPEG_put_chroma ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"mtsab   %[arg3], 0\n"
	"1:\n"
		"ld      %[tmp5],   0(%[arg1])\n"
		"ld      %[tmp6],  64(%[arg1])\n"
		"ld      %[tmp7], 384(%[arg1])\n"
		"ld      %[tmp8], 448(%[arg1])\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"pcpyld  %[tmp5], %[tmp7], %[tmp5]\n"
		"pcpyld  %[tmp6], %[tmp8], %[tmp6]\n"
		"qfsrv   %[tmp5], %[tmp5], %[tmp5]\n"
		"qfsrv   %[tmp6], %[tmp6], %[tmp6]\n"
		"pextlb  %[tmp5], $zero, %[tmp5]\n"
		"pextlb  %[tmp6], $zero, %[tmp6]\n"
		"sq      %[tmp5],   0(%[arg2])\n"
		"sq      %[tmp6], 128(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 16\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 704\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

void _MPEG_put_luma_X ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"pnor    $v0, $zero, $zero\n"
		"psrlh   $v0, $v0, 15\n"
	"1:\n"
		"lq      %[tmp5],   0(%[arg1])\n"
		"lq      %[tmp6], 384(%[arg1])\n"
		"mtsab   %[arg3], 0\n"
		"qfsrv   %[tmp7], %[tmp6], %[tmp5]\n"
		"qfsrv   %[tmp8], %[tmp5], %[tmp6]\n"
		"pextlb  %[tmp5], $zero, %[tmp7]\n"
		"pextub  %[tmp6], $zero, %[tmp7]\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"mtsab   $zero, 1\n"
		"addiu   $v1, $v1, -1\n"
		"qfsrv   %[tmp8], %[tmp8], %[tmp7]\n"
		"pextlb  %[tmp7], $zero, %[tmp8]\n"
		"pextub  %[tmp8], $zero, %[tmp8]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp7]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp8]\n"
		"paddh   %[tmp5], %[tmp5], $v0\n"
		"paddh   %[tmp6], %[tmp6], $v0\n"
		"psrlh   %[tmp5], %[tmp5], 1\n"
		"psrlh   %[tmp6], %[tmp6], 1\n"
		"sq      %[tmp5],  0(%[arg2])\n"
		"sq      %[tmp6], 16(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 32\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 512\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

void _MPEG_put_chroma_X ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"pnor    $v0, $zero, $zero\n"
		"psrlh   $v0, $v0, 15\n"
	"1:\n"
		"ld      %[tmp5],   0(%[arg1])\n"
		"ld      %[tmp6],  64(%[arg1])\n"
		"ld      %[tmp7], 384(%[arg1])\n"
		"ld      %[tmp8], 448(%[arg1])\n"
		"pcpyld  %[tmp5], %[tmp7], %[tmp5]\n"
		"pcpyld  %[tmp6], %[tmp8], %[tmp6]\n"
		"mtsab   %[arg3], 0\n"
		"qfsrv   %[tmp5], %[tmp5], %[tmp5]\n"
		"qfsrv   %[tmp6], %[tmp6], %[tmp6]\n"
		"addiu   %[tmp9], $zero, 1\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"mtsab   %[tmp9], 0\n"
		"qfsrv   %[tmp1], %[tmp5], %[tmp5]\n"
		"qfsrv   %[tmp2], %[tmp6], %[tmp6]\n"
		"pextlb  %[tmp5], $zero, %[tmp5]\n"
		"pextlb  %[tmp6], $zero, %[tmp6]\n"
		"pextlb  %[tmp1], $zero, %[tmp1]\n"
		"pextlb  %[tmp2], $zero, %[tmp2]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp1]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp2]\n"
		"paddh   %[tmp5], %[tmp5], $v0\n"
		"paddh   %[tmp6], %[tmp6], $v0\n"
		"psrlh   %[tmp5], %[tmp5], 1\n"
		"psrlh   %[tmp6], %[tmp6], 1\n"
		"sq      %[tmp5],   0(%[arg2])\n"
		"sq      %[tmp6], 128(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 16\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 704\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

void _MPEG_put_luma_Y ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"mtsab   %[arg3], 0\n"
		"lq      %[tmp7],   0(%[arg1])\n"
		"lq      %[tmp8], 384(%[arg1])\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"qfsrv   %[tmp7], %[tmp8], %[tmp7]\n"
		"pextub  %[tmp8], $zero, %[tmp7]\n"
		"pextlb  %[tmp7], $zero, %[tmp7]\n"
		"beq     $v1, $zero, 2f\n"
		"addiu   %[tmp10], %[tmp10], 1\n"
	"1:\n"
		"lq      %[tmp5],   0(%[arg1])\n"
		"lq      %[tmp6], 384(%[arg1])\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"qfsrv   %[tmp5], %[tmp6], %[tmp5]\n"
		"pextub  %[tmp6], $zero, %[tmp5]\n"
		"pextlb  %[tmp5], $zero, %[tmp5]\n"
		"paddh   $v0, %[tmp6], %[tmp8]\n"
		"pnor    %[tmp8], $zero, $zero\n"
		"paddh   %[tmp9], %[tmp5], %[tmp7]\n"
		"psrlh   %[tmp8], %[tmp8], 15\n"
		"por     %[tmp7], $zero, %[tmp5]\n"
		"paddh   %[tmp9], %[tmp9], %[tmp8]\n"
		"paddh   $v0, $v0, %[tmp8]\n"
		"por     %[tmp8], $zero, %[tmp6]\n"
		"psrlh   %[tmp9], %[tmp9], 1\n"
		"psrlh   $v0, $v0, 1\n"
		"sq      %[tmp9],  0(%[arg2])\n"
		"sq      $v0, 16(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 32\n"
	"2:\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 512\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

void _MPEG_put_chroma_Y ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"mtsab   %[arg3], 0\n"
		"ld      %[tmp3],   0(%[arg1])\n"
		"ld      %[tmp4],  64(%[arg1])\n"
		"ld      %[tmp0], 384(%[arg1])\n"
		"ld      %[tmp1], 448(%[arg1])\n"
		"pnor    $v0, $zero, $zero\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"psrlh   $v0, $v0, 15\n"
		"pcpyld  %[tmp3], %[tmp0], %[tmp3]\n"
		"pcpyld  %[tmp4], %[tmp1], %[tmp4]\n"
		"qfsrv   %[tmp3], %[tmp3], %[tmp3]\n"
		"qfsrv   %[tmp4], %[tmp4], %[tmp4]\n"
		"pextlb  %[tmp3], $zero, %[tmp3]\n"
		"pextlb  %[tmp4], $zero, %[tmp4]\n"
		"beq     $v1, $zero, 2f\n"
		"addiu   %[tmp10], %[tmp10], 1\n"
	"1:\n"
		"ld      %[tmp5],   0(%[arg1])\n"
		"ld      %[tmp6],  64(%[arg1])\n"
		"ld      %[tmp7], 384(%[arg1])\n"
		"ld      %[tmp8], 448(%[arg1])\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"pcpyld  %[tmp5], %[tmp7], %[tmp5]\n"
		"pcpyld  %[tmp6], %[tmp8], %[tmp6]\n"
		"qfsrv   %[tmp5], %[tmp5], %[tmp5]\n"
		"qfsrv   %[tmp6], %[tmp6], %[tmp6]\n"
		"pextlb  %[tmp5], $zero, %[tmp5]\n"
		"pextlb  %[tmp6], $zero, %[tmp6]\n"
		"paddh   %[tmp1], %[tmp5], %[tmp3]\n"
		"paddh   %[tmp2], %[tmp6], %[tmp4]\n"
		"por     %[tmp3], $zero, %[tmp5]\n"
		"por     %[tmp4], $zero, %[tmp6]\n"
		"paddh   %[tmp1], %[tmp1], $v0\n"
		"paddh   %[tmp2], %[tmp2], $v0\n"
		"psrlh   %[tmp1], %[tmp1], 1\n"
		"psrlh   %[tmp2], %[tmp2], 1\n"
		"sq      %[tmp1],   0(%[arg2])\n"
		"sq      %[tmp2], 128(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 16\n"
	"2:\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 704\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

void _MPEG_put_luma_XY ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"mtsab   %[arg3], 0\n"
		"lq      $v0,   0(%[arg1])\n"
		"lq      %[tmp7], 384(%[arg1])\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"qfsrv   %[tmp8], %[tmp7], $v0\n"
		"qfsrv   %[tmp9], $v0, %[tmp7]\n"
		"addiu   $v1, $v1, -1\n"
		"pextlb  $v0, $zero, %[tmp8]\n"
		"pextub  %[tmp7], $zero, %[tmp8]\n"
		"mtsab   $zero, 1\n"
		"qfsrv   %[tmp9], %[tmp9], %[tmp8]\n"
		"pextlb  %[tmp8], $zero, %[tmp9]\n"
		"pextub  %[tmp9], $zero, %[tmp9]\n"
		"paddh   $v0, $v0, %[tmp8]\n"
		"paddh   %[tmp7], %[tmp7], %[tmp9]\n"
		"beq     $v1, $zero, 2f\n"
		"addiu   %[tmp10], %[tmp10], 1\n"
	"1:\n"
		"lq      %[tmp5],   0(%[arg1])\n"
		"lq      %[tmp6], 384(%[arg1])\n"
		"mtsab   %[arg3], 0\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"qfsrv   %[tmp8], %[tmp6], %[tmp5]\n"
		"qfsrv   %[tmp9], %[tmp5], %[tmp6]\n"
		"addiu   $v1, $v1, -1\n"
		"pextlb  %[tmp5], $zero, %[tmp8]\n"
		"pextub  %[tmp6], $zero, %[tmp8]\n"
		"mtsab   $zero, 1\n"
		"qfsrv   %[tmp9], %[tmp9], %[tmp8]\n"
		"pextlb  %[tmp8], $zero, %[tmp9]\n"
		"pextub  %[tmp9], $zero, %[tmp9]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp8]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp9]\n"
		"paddh   %[tmp8], $v0, %[tmp5]\n"
		"paddh   %[tmp9], %[tmp7], %[tmp6]\n"
		"por     $v0, $zero, %[tmp5]\n"
		"pnor    %[tmp5], $zero, $zero\n"
		"por     %[tmp7], $zero, %[tmp6]\n"
		"psrlh   %[tmp5], %[tmp5], 15\n"
		"psllh   %[tmp5], %[tmp5],  1\n"
		"paddh   %[tmp8], %[tmp8], %[tmp5]\n"
		"paddh   %[tmp9], %[tmp9], %[tmp5]\n"
		"psrlh   %[tmp8], %[tmp8], 2\n"
		"psrlh   %[tmp9], %[tmp9], 2\n"
		"sq      %[tmp8],  0(%[arg2])\n"
		"sq      %[tmp9], 16(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 32\n"
	"2:\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 512\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

void _MPEG_put_chroma_XY ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"mtsab   %[arg3], 0\n"
		"pnor    %[tmp9], $zero, $zero\n"
		"ld      %[tmp3],   0(%[arg1])\n"
		"ld      $v0,  64(%[arg1])\n"
		"mtsab   $zero, 1\n"
		"ld      %[tmp0], 384(%[arg1])\n"
		"ld      %[tmp1], 448(%[arg1])\n"
		"pcpyld  %[tmp3], %[tmp0], %[tmp3]\n"
		"pcpyld  $v0, %[tmp1], $v0\n"
		"qfsrv   %[tmp3], %[tmp3], %[tmp3]\n"
		"qfsrv   $v0, $v0, $v0\n"
		"psrlh   %[tmp9], %[tmp9], 15\n"
		"psllh   %[tmp9], %[tmp9], 1\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"qfsrv   %[tmp0], %[tmp3], %[tmp3]\n"
		"qfsrv   %[tmp1], $v0, $v0\n"
		"pextlb  %[tmp3], $zero, %[tmp3]\n"
		"pextlb  $v0, $zero, $v0\n"
		"pextlb  %[tmp0], $zero, %[tmp0]\n"
		"pextlb  %[tmp1], $zero, %[tmp1]\n"
		"paddh   %[tmp3], %[tmp3], %[tmp0]\n"
		"paddh   %[tmp0], $v0, %[tmp1]\n"
		"beq     $v1, $zero, 2f\n"
		"addiu   %[tmp10], %[tmp10], 1\n"
	"1:\n"
		"ld      %[tmp5],   0(%[arg1])\n"
		"ld      %[tmp7],  64(%[arg1])\n"
		"mtsab   %[arg3], 0\n"
		"ld      %[tmp6], 384(%[arg1])\n"
		"ld      %[tmp8], 448(%[arg1])\n"
		"pcpyld  %[tmp5], %[tmp6], %[tmp5]\n"
		"pcpyld  %[tmp7], %[tmp8], %[tmp7]\n"
		"qfsrv   %[tmp5], %[tmp5], %[tmp5]\n"
		"qfsrv   %[tmp7], %[tmp7], %[tmp7]\n"
		"addiu   $v0, $zero, 1\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"mtsab   $v0, 0\n"
		"qfsrv   %[tmp6], %[tmp5], %[tmp5]\n"
		"qfsrv   %[tmp8], %[tmp7], %[tmp7]\n"
		"pextlb  %[tmp5], $zero, %[tmp5]\n"
		"pextlb  %[tmp7], $zero, %[tmp7]\n"
		"pextlb  %[tmp6], $zero, %[tmp6]\n"
		"pextlb  %[tmp8], $zero, %[tmp8]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp6]\n"
		"paddh   %[tmp6], %[tmp7], %[tmp8]\n"
		"paddh   %[tmp7], %[tmp3], %[tmp5]\n"
		"paddh   %[tmp8], %[tmp0], %[tmp6]\n"
		"por     %[tmp3], $zero, %[tmp5]\n"
		"por     %[tmp0], $zero, %[tmp6]\n"
		"paddh   %[tmp7], %[tmp7], %[tmp9]\n"
		"paddh   %[tmp8], %[tmp8], %[tmp9]\n"
		"psrlh   %[tmp7], %[tmp7], 2\n"
		"psrlh   %[tmp8], %[tmp8], 2\n"
		"sq      %[tmp7],   0(%[arg2])\n"
		"sq      %[tmp8], 128(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 16\n"
	"2:\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 704\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

void _MPEG_avg_luma ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"mtsab   %[arg3], 0\n"
	"1:\n"
		"lq      %[tmp5],   0(%[arg1])\n"
		"lq      %[tmp6], 384(%[arg1])\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"qfsrv   %[tmp5], %[tmp6], %[tmp5]\n"
		"pextlb  %[tmp6], $zero, %[tmp5]\n"
		"pextub  %[tmp5], $zero, %[tmp5]\n"
		"lq      %[tmp8],  0(%[arg2])\n"
		"lq      %[tmp9], 16(%[arg2])\n"
		"paddh   %[tmp6], %[tmp6], %[tmp8]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp9]\n"
		"pcgth   %[tmp8], %[tmp6], $zero\n"
		"pcgth   %[tmp9], %[tmp5], $zero\n"
		"pceqh   $v0, %[tmp6], $zero\n"
		"pceqh   %[tmp7], %[tmp5], $zero\n"
		"psrlh   %[tmp8], %[tmp8], 15\n"
		"psrlh   %[tmp9], %[tmp9], 15\n"
		"psrlh   $v0, $v0, 15\n"
		"psrlh   %[tmp7], %[tmp7], 15\n"
		"por     %[tmp8], %[tmp8], $v0\n"
		"por     %[tmp9], %[tmp9], %[tmp7]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp8]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp9]\n"
		"psrlh   %[tmp6], %[tmp6], 1\n"
		"psrlh   %[tmp5], %[tmp5], 1\n"
		"sq      %[tmp6],  0(%[arg2])\n"
		"sq      %[tmp5], 16(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 32\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 512\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

void _MPEG_avg_chroma ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"mtsab   %[arg3], 0\n"
	"1:\n"
		"ld      %[tmp5],   0(%[arg1])\n"
		"ld      %[tmp6],  64(%[arg1])\n"
		"addiu   $v1, $v1, -1\n"
		"ld      %[tmp7], 384(%[arg1])\n"
		"ld      %[tmp8], 448(%[arg1])\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"pcpyld  %[tmp5], %[tmp7], %[tmp5]\n"
		"pcpyld  %[tmp6], %[tmp8], %[tmp6]\n"
		"qfsrv   %[tmp5], %[tmp5], %[tmp5]\n"
		"qfsrv   %[tmp6], %[tmp6], %[tmp6]\n"
		"pextlb  %[tmp5], $zero, %[tmp5]\n"
		"pextlb  %[tmp6], $zero, %[tmp6]\n"
		"lq      %[tmp0],   0(%[arg2])\n"
		"lq      %[tmp1], 128(%[arg2])\n"
		"paddh   %[tmp5], %[tmp5], %[tmp0]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp1]\n"
		"pcgth   %[tmp0], %[tmp5], $zero\n"
		"pcgth   %[tmp1], %[tmp6], $zero\n"
		"pceqh   $v0, %[tmp5], $zero\n"
		"pceqh   %[tmp9], %[tmp6], $zero\n"
		"psrlh   %[tmp0], %[tmp0], 15\n"
		"psrlh   %[tmp1], %[tmp1], 15\n"
		"psrlh   $v0, $v0, 15\n"
		"psrlh   %[tmp9], %[tmp9], 15\n"
		"por     %[tmp0], %[tmp0], $v0\n"
		"por     %[tmp1], %[tmp1], %[tmp9]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp0]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp1]\n"
		"psrlh   %[tmp5], %[tmp5], 1\n"
		"psrlh   %[tmp6], %[tmp6], 1\n"
		"sq      %[tmp5],   0(%[arg2])\n"
		"sq      %[tmp6], 128(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 16\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 704\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

void _MPEG_avg_luma_X ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"pnor    $v0, $zero, $zero\n"
		"psrlh   $v0, $v0, 15\n"
	"1:\n"
		"lq      %[tmp5],   0(%[arg1])\n"
		"lq      %[tmp6], 384(%[arg1])\n"
		"mtsab   %[arg3], 0\n"
		"qfsrv   %[tmp7], %[tmp6], %[tmp5]\n"
		"qfsrv   %[tmp8], %[tmp5], %[tmp6]\n"
		"pextlb  %[tmp5], $zero, %[tmp7]\n"
		"pextub  %[tmp6], $zero, %[tmp7]\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"mtsab   $zero, 1\n"
		"addiu   $v1, $v1, -1\n"
		"qfsrv   %[tmp8], %[tmp8], %[tmp7]\n"
		"pextlb  %[tmp7], $zero, %[tmp8]\n"
		"pextub  %[tmp8], $zero, %[tmp8]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp7]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp8]\n"
		"paddh   %[tmp5], %[tmp5], $v0\n"
		"paddh   %[tmp6], %[tmp6], $v0\n"
		"psrlh   %[tmp5], %[tmp5], 1\n"
		"psrlh   %[tmp6], %[tmp6], 1\n"
		"lq      %[tmp8],  0(%[arg2])\n"
		"lq      %[tmp9], 16(%[arg2])\n"
		"paddh   %[tmp5], %[tmp5], %[tmp8]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp9]\n"
		"pcgth   %[tmp8], %[tmp5], $zero\n"
		"pceqh   %[tmp9], %[tmp5], $zero\n"
		"psrlh   %[tmp8], %[tmp8], 15\n"
		"psrlh   %[tmp9], %[tmp9], 15\n"
		"por     %[tmp8], %[tmp8], %[tmp9]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp8]\n"
		"pcgth   %[tmp8], %[tmp6], $zero\n"
		"pceqh   %[tmp9], %[tmp6], $zero\n"
		"psrlh   %[tmp8], %[tmp8], 15\n"
		"psrlh   %[tmp9], %[tmp9], 15\n"
		"por     %[tmp8], %[tmp8], %[tmp9]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp8]\n"
		"psrlh   %[tmp5], %[tmp5], 1\n"
		"psrlh   %[tmp6], %[tmp6], 1\n"
		"sq      %[tmp5],  0(%[arg2])\n"
		"sq      %[tmp6], 16(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 32\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 512\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

void _MPEG_avg_chroma_X ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"pnor    $v0, $zero, $zero\n"
		"psrlh   $v0, $v0, 15\n"
	"1:\n"
		"ld      %[tmp5],   0(%[arg1])\n"
		"ld      %[tmp6],  64(%[arg1])\n"
		"mtsab   %[arg3], 0\n"
		"ld      %[tmp7], 384(%[arg1])\n"
		"ld      %[tmp8], 448(%[arg1])\n"
		"pcpyld  %[tmp5], %[tmp7], %[tmp5]\n"
		"pcpyld  %[tmp6], %[tmp8], %[tmp6]\n"
		"qfsrv   %[tmp5], %[tmp5], %[tmp5]\n"
		"qfsrv   %[tmp6], %[tmp6], %[tmp6]\n"
		"addiu   %[tmp9], $zero, 1\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"mtsab   %[tmp9], 0\n"
		"qfsrv   %[tmp1], %[tmp5], %[tmp5]\n"
		"qfsrv   %[tmp2], %[tmp6], %[tmp6]\n"
		"pextlb  %[tmp5], $zero, %[tmp5]\n"
		"pextlb  %[tmp6], $zero, %[tmp6]\n"
		"pextlb  %[tmp1], $zero, %[tmp1]\n"
		"pextlb  %[tmp2], $zero, %[tmp2]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp1]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp2]\n"
		"paddh   %[tmp5], %[tmp5], $v0\n"
		"paddh   %[tmp6], %[tmp6], $v0\n"
		"psrlh   %[tmp5], %[tmp5], 1\n"
		"psrlh   %[tmp6], %[tmp6], 1\n"
		"lq      %[tmp1],   0(%[arg2])\n"
		"lq      %[tmp2], 128(%[arg2])\n"
		"paddh   %[tmp5], %[tmp5], %[tmp1]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp2]\n"
		"pcgth   %[tmp1], %[tmp5], $zero\n"
		"pcgth   %[tmp2], %[tmp6], $zero\n"
		"pceqh   %[tmp9], %[tmp5], $zero\n"
		"pceqh   %[tmp3], %[tmp6], $zero\n"
		"psrlh   %[tmp1], %[tmp1], 15\n"
		"psrlh   %[tmp2], %[tmp2], 15\n"
		"psrlh   %[tmp9], %[tmp9], 15\n"
		"psrlh   %[tmp3], %[tmp3], 15\n"
		"por     %[tmp1], %[tmp1], %[tmp9]\n"
		"por     %[tmp2], %[tmp2], %[tmp3]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp1]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp2]\n"
		"psrlh   %[tmp5], %[tmp5], 1\n"
		"psrlh   %[tmp6], %[tmp6], 1\n"
		"sq      %[tmp5],   0(%[arg2])\n"
		"sq      %[tmp6], 128(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 16\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 704\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

void _MPEG_avg_luma_Y ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"mtsab   %[arg3], 0\n"
		"lq      %[tmp7],   0(%[arg1])\n"
		"lq      %[tmp8], 384(%[arg1])\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"qfsrv   %[tmp7], %[tmp8], %[tmp7]\n"
		"pextub  %[tmp8], $zero, %[tmp7]\n"
		"pextlb  %[tmp7], $zero, %[tmp7]\n"
		"beq     $v1, $zero, 2f\n"
		"addiu   %[tmp10], %[tmp10], 1\n"
	"1:\n"
		"lq      %[tmp5],   0(%[arg1])\n"
		"lq      %[tmp6], 384(%[arg1])\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"qfsrv   %[tmp5], %[tmp6], %[tmp5]\n"
		"pextub  %[tmp6], $zero, %[tmp5]\n"
		"pextlb  %[tmp5], $zero, %[tmp5]\n"
		"paddh   $v0, %[tmp6], %[tmp8]\n"
		"pnor    %[tmp8], $zero, $zero\n"
		"paddh   %[tmp9], %[tmp5], %[tmp7]\n"
		"psrlh   %[tmp8], %[tmp8], 15\n"
		"por     %[tmp7], $zero, %[tmp5]\n"
		"paddh   %[tmp9], %[tmp9], %[tmp8]\n"
		"paddh   $v0, $v0, %[tmp8]\n"
		"por     %[tmp8], $zero, %[tmp6]\n"
		"psrlh   %[tmp9], %[tmp9], 1\n"
		"psrlh   $v0, $v0, 1\n"
		"lq      %[tmp5],  0(%[arg2])\n"
		"lq      %[tmp6], 16(%[arg2])\n"
		"paddh   %[tmp9], %[tmp9], %[tmp5]\n"
		"paddh   $v0, $v0, %[tmp6]\n"
		"pcgth   %[tmp5], %[tmp9], $zero\n"
		"pceqh   %[tmp6], %[tmp9], $zero\n"
		"psrlh   %[tmp5], %[tmp5], 15\n"
		"psrlh   %[tmp6], %[tmp6], 15\n"
		"por     %[tmp5], %[tmp5], %[tmp6]\n"
		"paddh   %[tmp9], %[tmp9], %[tmp5]\n"
		"pcgth   %[tmp5], $v0, $zero\n"
		"pceqh   %[tmp6], $v0, $zero\n"
		"psrlh   %[tmp5], %[tmp5], 15\n"
		"psrlh   %[tmp6], %[tmp6], 15\n"
		"por     %[tmp5], %[tmp5], %[tmp6]\n"
		"paddh   $v0, $v0, %[tmp5]\n"
		"psrlh   %[tmp9], %[tmp9], 1\n"
		"psrlh   $v0, $v0, 1\n"
		"sq      %[tmp9],  0(%[arg2])\n"
		"sq      $v0, 16(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 32\n"
	"2:\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 512\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

void _MPEG_avg_chroma_Y ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"mtsab   %[arg3], 0\n"
		"ld      %[tmp3],   0(%[arg1])\n"
		"ld      %[tmp4],  64(%[arg1])\n"
		"ld      %[tmp0], 384(%[arg1])\n"
		"ld      %[tmp1], 448(%[arg1])\n"
		"pnor    $v0, $zero, $zero\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"psrlh   $v0, $v0, 15\n"
		"pcpyld  %[tmp3], %[tmp0], %[tmp3]\n"
		"pcpyld  %[tmp4], %[tmp1], %[tmp4]\n"
		"qfsrv   %[tmp3], %[tmp3], %[tmp3]\n"
		"qfsrv   %[tmp4], %[tmp4], %[tmp4]\n"
		"pextlb  %[tmp3], $zero, %[tmp3]\n"
		"pextlb  %[tmp4], $zero, %[tmp4]\n"
		"beq     $v1, $zero, 2f\n"
		"addiu   %[tmp10], %[tmp10], 1\n"
	"1:\n"
		"ld      %[tmp5],   0(%[arg1])\n"
		"ld      %[tmp6],  64(%[arg1])\n"
		"addiu   $v1, $v1, -1\n"
		"ld      %[tmp7], 384(%[arg1])\n"
		"ld      %[tmp8], 448(%[arg1])\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"pcpyld  %[tmp5], %[tmp7], %[tmp5]\n"
		"pcpyld  %[tmp6], %[tmp8], %[tmp6]\n"
		"qfsrv   %[tmp5], %[tmp5], %[tmp5]\n"
		"qfsrv   %[tmp6], %[tmp6], %[tmp6]\n"
		"pextlb  %[tmp5], $zero, %[tmp5]\n"
		"pextlb  %[tmp6], $zero, %[tmp6]\n"
		"paddh   %[tmp1], %[tmp5], %[tmp3]\n"
		"paddh   %[tmp2], %[tmp6], %[tmp4]\n"
		"por     %[tmp3], $zero, %[tmp5]\n"
		"por     %[tmp4], $zero, %[tmp6]\n"
		"paddh   %[tmp1], %[tmp1], $v0\n"
		"paddh   %[tmp2], %[tmp2], $v0\n"
		"psrlh   %[tmp1], %[tmp1], 1\n"
		"psrlh   %[tmp2], %[tmp2], 1\n"
		"lq      %[tmp5],   0(%[arg2])\n"
		"lq      %[tmp6], 128(%[arg2])\n"
		"paddh   %[tmp1], %[tmp1], %[tmp5]\n"
		"paddh   %[tmp2], %[tmp2], %[tmp6]\n"
		"pcgth   %[tmp5], %[tmp1], $zero\n"
		"pceqh   %[tmp6], %[tmp1], $zero\n"
		"psrlh   %[tmp5], %[tmp5], 15\n"
		"psrlh   %[tmp6], %[tmp6], 15\n"
		"por     %[tmp5], %[tmp5], %[tmp6]\n"
		"paddh   %[tmp1], %[tmp1], %[tmp5]\n"
		"pcgth   %[tmp5], %[tmp2], $zero\n"
		"pceqh   %[tmp6], %[tmp2], $zero\n"
		"psrlh   %[tmp5], %[tmp5], 15\n"
		"psrlh   %[tmp6], %[tmp6], 15\n"
		"por     %[tmp5], %[tmp5], %[tmp6]\n"
		"paddh   %[tmp2], %[tmp2], %[tmp5]\n"
		"psrlh   %[tmp1], %[tmp1], 1\n"
		"psrlh   %[tmp2], %[tmp2], 1\n"
		"sq      %[tmp1],   0(%[arg2])\n"
		"sq      %[tmp2], 128(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 16\n"
	"2:\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 704\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

void _MPEG_avg_luma_XY ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"mtsab   %[arg3], 0\n"
		"lq      $v0,   0(%[arg1])\n"
		"lq      %[tmp7], 384(%[arg1])\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"qfsrv   %[tmp8], %[tmp7], $v0\n"
		"qfsrv   %[tmp9], $v0, %[tmp7]\n"
		"addiu   $v1, $v1, -1\n"
		"pextlb  $v0, $zero, %[tmp8]\n"
		"pextub  %[tmp7], $zero, %[tmp8]\n"
		"mtsab   $zero, 1\n"
		"qfsrv   %[tmp9], %[tmp9], %[tmp8]\n"
		"pextlb  %[tmp8], $zero, %[tmp9]\n"
		"pextub  %[tmp9], $zero, %[tmp9]\n"
		"paddh   $v0, $v0, %[tmp8]\n"
		"paddh   %[tmp7], %[tmp7], %[tmp9]\n"
		"beq     $v1, $zero, 2f\n"
		"addiu   %[tmp10], %[tmp10], 1\n"
	"1:\n"
		"lq      %[tmp5],   0(%[arg1])\n"
		"lq      %[tmp6], 384(%[arg1])\n"
		"mtsab   %[arg3], 0\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"qfsrv   %[tmp8], %[tmp6], %[tmp5]\n"
		"qfsrv   %[tmp9], %[tmp5], %[tmp6]\n"
		"addiu   $v1, $v1, -1\n"
		"pextlb  %[tmp5], $zero, %[tmp8]\n"
		"pextub  %[tmp6], $zero, %[tmp8]\n"
		"mtsab   $zero, 1\n"
		"qfsrv   %[tmp9], %[tmp9], %[tmp8]\n"
		"pextlb  %[tmp8], $zero, %[tmp9]\n"
		"pextub  %[tmp9], $zero, %[tmp9]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp8]\n"
		"paddh   %[tmp6], %[tmp6], %[tmp9]\n"
		"paddh   %[tmp8], $v0, %[tmp5]\n"
		"paddh   %[tmp9], %[tmp7], %[tmp6]\n"
		"por     $v0, $zero, %[tmp5]\n"
		"pnor    %[tmp5], $zero, $zero\n"
		"por     %[tmp7], $zero, %[tmp6]\n"
		"psrlh   %[tmp5], %[tmp5], 15\n"
		"psllh   %[tmp5], %[tmp5],  1\n"
		"paddh   %[tmp8], %[tmp8], %[tmp5]\n"
		"paddh   %[tmp9], %[tmp9], %[tmp5]\n"
		"psrlh   %[tmp8], %[tmp8], 2\n"
		"psrlh   %[tmp9], %[tmp9], 2\n"
		"lq      %[tmp5],  0(%[arg2])\n"
		"lq      %[tmp6], 16(%[arg2])\n"
		"paddh   %[tmp8], %[tmp8], %[tmp5]\n"
		"paddh   %[tmp9], %[tmp9], %[tmp6]\n"
		"pcgth   %[tmp5], %[tmp8], $zero\n"
		"pceqh   %[tmp6], %[tmp8], $zero\n"
		"psrlh   %[tmp5], %[tmp5], 15\n"
		"psrlh   %[tmp6], %[tmp6], 15\n"
		"por     %[tmp5], %[tmp5], %[tmp6]\n"
		"paddh   %[tmp8], %[tmp8], %[tmp5]\n"
		"pcgth   %[tmp5], %[tmp9], $zero\n"
		"pceqh   %[tmp6], %[tmp9], $zero\n"
		"psrlh   %[tmp5], %[tmp5], 15\n"
		"psrlh   %[tmp6], %[tmp6], 15\n"
		"por     %[tmp5], %[tmp5], %[tmp6]\n"
		"paddh   %[tmp9], %[tmp9], %[tmp5]\n"
		"psrlh   %[tmp8], %[tmp8], 1\n"
		"psrlh   %[tmp9], %[tmp9], 1\n"
		"sq      %[tmp8],  0(%[arg2])\n"
		"sq      %[tmp9], 16(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 32\n"
	"2:\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 512\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

void _MPEG_avg_chroma_XY ( unsigned char* arg1, short* arg2, int arg3, int arg4 )
{
	u128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
	asm __volatile__(
		"mtsab   %[arg3], 0\n"
		"pnor    %[tmp9], $zero, $zero\n"
		"ld      %[tmp3],   0(%[arg1])\n"
		"ld      $v0,  64(%[arg1])\n"
		"mtsab   $zero, 1\n"
		"ld      %[tmp0], 384(%[arg1])\n"
		"ld      %[tmp1], 448(%[arg1])\n"
		"pcpyld  %[tmp3], %[tmp0], %[tmp3]\n"
		"pcpyld  $v0, %[tmp1], $v0\n"
		"qfsrv   %[tmp3], %[tmp3], %[tmp3]\n"
		"qfsrv   $v0, $v0, $v0\n"
		"psrlh   %[tmp9], %[tmp9], 15\n"
		"psllh   %[tmp9], %[tmp9],  1\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"qfsrv   %[tmp0], %[tmp3], %[tmp3]\n"
		"qfsrv   %[tmp1], $v0, $v0\n"
		"pextlb  %[tmp3], $zero, %[tmp3]\n"
		"pextlb  $v0, $zero, $v0\n"
		"pextlb  %[tmp0], $zero, %[tmp0]\n"
		"pextlb  %[tmp1], $zero, %[tmp1]\n"
		"paddh   %[tmp3], %[tmp3], %[tmp0]\n"
		"paddh   %[tmp0], $v0, %[tmp1]\n"
		"beq     $v1, $zero, 2f\n"
		"addiu   %[tmp10], %[tmp10], 1\n"
	"1:\n"
		"ld      %[tmp5],   0(%[arg1])\n"
		"ld      %[tmp7],  64(%[arg1])\n"
		"mtsab   %[arg3], 0\n"
		"ld      %[tmp6], 384(%[arg1])\n"
		"ld      %[tmp8], 448(%[arg1])\n"
		"pcpyld  %[tmp5], %[tmp6], %[tmp5]\n"
		"pcpyld  %[tmp7], %[tmp8], %[tmp7]\n"
		"qfsrv   %[tmp5], %[tmp5], %[tmp5]\n"
		"qfsrv   %[tmp7], %[tmp7], %[tmp7]\n"
		"addiu   $v0, $zero, 1\n"
		"addu    %[arg1], %[arg1], %[arg4]\n"
		"addiu   $v1, $v1, -1\n"
		"mtsab   $v0, 0\n"
		"qfsrv   %[tmp6], %[tmp5], %[tmp5]\n"
		"qfsrv   %[tmp8], %[tmp7], %[tmp7]\n"
		"pextlb  %[tmp5], $zero, %[tmp5]\n"
		"pextlb  %[tmp7], $zero, %[tmp7]\n"
		"pextlb  %[tmp6], $zero, %[tmp6]\n"
		"pextlb  %[tmp8], $zero, %[tmp8]\n"
		"paddh   %[tmp5], %[tmp5], %[tmp6]\n"
		"paddh   %[tmp6], %[tmp7], %[tmp8]\n"
		"paddh   %[tmp7], %[tmp3], %[tmp5]\n"
		"paddh   %[tmp8], %[tmp0], %[tmp6]\n"
		"por     %[tmp3], $zero, %[tmp5]\n"
		"por     %[tmp0], $zero, %[tmp6]\n"
		"paddh   %[tmp7], %[tmp7], %[tmp9]\n"
		"paddh   %[tmp8], %[tmp8], %[tmp9]\n"
		"psrlh   %[tmp7], %[tmp7], 2\n"
		"psrlh   %[tmp8], %[tmp8], 2\n"
		"lq      %[tmp5],   0(%[arg2])\n"
		"lq      %[tmp6], 128(%[arg2])\n"
		"paddh   %[tmp7], %[tmp7], %[tmp5]\n"
		"paddh   %[tmp8], %[tmp8], %[tmp6]\n"
		"pcgth   %[tmp5], %[tmp7], $zero\n"
		"pceqh   %[tmp6], %[tmp7], $zero\n"
		"psrlh   %[tmp5], %[tmp5], 15\n"
		"psrlh   %[tmp6], %[tmp6], 15\n"
		"por     %[tmp5], %[tmp5], %[tmp6]\n"
		"paddh   %[tmp7], %[tmp7], %[tmp5]\n"
		"pcgth   %[tmp5], %[tmp8], $zero\n"
		"pceqh   %[tmp6], %[tmp8], $zero\n"
		"psrlh   %[tmp5], %[tmp5], 15\n"
		"psrlh   %[tmp6], %[tmp6], 15\n"
		"por     %[tmp5], %[tmp5], %[tmp6]\n"
		"paddh   %[tmp8], %[tmp8], %[tmp5]\n"
		"psrlh   %[tmp7], %[tmp7], 1\n"
		"psrlh   %[tmp8], %[tmp8], 1\n"
		"sq      %[tmp7],   0(%[arg2])\n"
		"sq      %[tmp8], 128(%[arg2])\n"
		"bgtz    $v1, 1b\n"
		"addiu   %[arg2], %[arg2], 16\n"
	"2:\n"
		"addu    $v1, $zero, %[tmp10]\n"
		"addiu   %[arg1], %[arg1], 704\n"
		"bgtzl   $v1, 1b\n"
		"addu    %[tmp10], $zero, $zero\n"
	: [arg1] "+r"(arg1), [arg2] "+r"(arg2), [tmp0] "=r"(tmp0), [tmp1] "=r"(tmp1), [tmp2] "=r"(tmp2), [tmp3] "=r"(tmp3), [tmp4] "=r"(tmp4), [tmp5] "=r"(tmp5), [tmp6] "=r"(tmp6), [tmp7] "=r"(tmp7), [tmp8] "=r"(tmp8), [tmp9] "=r"(tmp9), [tmp10] "=r"(tmp10)
	: [arg3] "r"(arg3), [arg4] "r"(arg4)
	: "v0", "v1", "memory"
	);
}

