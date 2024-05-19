/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"

#define MODNAME "clearspu"
IRX_ID(MODNAME, 1, 1);

static char buf_zero[0xC00];

static void _spu2_config_initialize(void);
static void _spu2_config_initialize_typically(void);
static void start_inner2(void);
static int do_spu_intr(void *userdata);
static void SpuStart(void);
static void spu_init_(void);
static int do_dma_finish(void *userdata);
static void do_dma_stop(int which_core);
static int do_dma_get_status(int which_core);
static void do_dma_start(void *addr, int size, int which_core);
static void do_spu_wait(void);

int _start(int argc, char *argv[])
{
	int i;
	int dummyarg[2];

	(void)argc;
	(void)argv;

	ReleaseIntrHandler(IOP_IRQ_DMA_SPU2);
	ReleaseIntrHandler(IOP_IRQ_DMA_SPU);
	ReleaseIntrHandler(IOP_IRQ_SPU);
	DisableIntr(IOP_IRQ_DMA_SPU2, dummyarg);
	DisableIntr(IOP_IRQ_DMA_SPU, dummyarg);
	DisableIntr(IOP_IRQ_SPU, dummyarg);
	_spu2_config_initialize();
	start_inner2();
	_spu2_config_initialize_typically();
	CpuEnableIntr();
	EnableIntr(IOP_IRQ_DMA_SPU);
	EnableIntr(IOP_IRQ_DMA_SPU2);
	EnableIntr(IOP_IRQ_SPU);
	memset(buf_zero, 0, sizeof(buf_zero));
	for ( i = 0; i < 2; i += 1 )
	{
		// Unofficial: Apply fixed argument, remove unused branches
		do_dma_start(buf_zero, sizeof(buf_zero), i);
	}
	while ( do_dma_get_status(0) && do_dma_get_status(1) )
		;
	ReleaseIntrHandler(IOP_IRQ_DMA_SPU2);
	ReleaseIntrHandler(IOP_IRQ_DMA_SPU);
	ReleaseIntrHandler(IOP_IRQ_SPU);
	printf("clearspu: completed\n");
	// Unofficial: Return non-resident
	return MODULE_NO_RESIDENT_END;
}

static void spu2_config_iop_(void)
{
	*((vu32 *)0xBF801404) = 0xBF900000;
	*((vu32 *)0xBF80140C) = 0xBF900800;
	*((vu32 *)0xBF8010F0) |= 0x80000u;
	*((vu32 *)0xBF801570) |= 8u;
	*((vu32 *)0xBF801014) = 0x200B31E1;
	*((vu32 *)0xBF801414) = 0x200B31E1;
}

static void spu2_config_SPDIF_(void)
{
	// Unofficial: Remove unused branch and contents
	*((vu16 *)0xBF9007C6) = 2304;
	*((vu16 *)0xBF9007C8) = 512;
	*((vu16 *)0xBF9007CA) = 8;
}

static void _spu2_config_initialize(void)
{
	spu2_config_iop_();
	spu2_config_SPDIF_();
}

static void _spu2_config_initialize_typically(void)
{
	// Unofficial: Remove unused branch and contents
	*((vu16 *)0xBF9007C0) = -16334;
	*((vu16 *)0xBF90019A) = -16384;
	*((vu16 *)0xBF90059A) = -16383;
	*((vu16 *)0xBF900188) = -1;
	*((vu16 *)0xBF90018A) = 255;
	*((vu16 *)0xBF900190) = -1;
	*((vu16 *)0xBF900192) = 255;
	*((vu16 *)0xBF90018C) = -1;
	*((vu16 *)0xBF90018E) = 255;
	*((vu16 *)0xBF900194) = -1;
	*((vu16 *)0xBF900196) = 255;
	*((vu16 *)0xBF900588) = -1;
	*((vu16 *)0xBF90058A) = 255;
	*((vu16 *)0xBF900590) = -1;
	*((vu16 *)0xBF900592) = 255;
	*((vu16 *)0xBF90058C) = -1;
	*((vu16 *)0xBF90058E) = 255;
	*((vu16 *)0xBF900594) = -1;
	*((vu16 *)0xBF900596) = 255;
	*((vu16 *)0xBF900198) = 4080;
	*((vu16 *)0xBF900598) = 4092;
	*((vu16 *)0xBF900760) = 0;
	*((vu16 *)0xBF900762) = 0;
	*((vu16 *)0xBF900788) = 0;
	*((vu16 *)0xBF90078A) = 0;
	*((vu16 *)0xBF900764) = 0;
	*((vu16 *)0xBF900766) = 0;
	*((vu16 *)0xBF90078C) = 0;
	*((vu16 *)0xBF90078E) = 0;
	*((vu16 *)0xBF90033C) = 14;
	*((vu16 *)0xBF90073C) = 15;
	*((vu16 *)0xBF900768) = 0;
	*((vu16 *)0xBF90076A) = 0;
	*((vu16 *)0xBF900790) = 0x7FFF;
	*((vu16 *)0xBF900792) = 0x7FFF;
	*((vu16 *)0xBF90076C) = 0;
	*((vu16 *)0xBF90076E) = 0;
	*((vu16 *)0xBF900794) = 0;
	*((vu16 *)0xBF900796) = 0;
}

static void start_inner2(void)
{
	spu_init_();
	// Unofficial: Remove setting of never read variables
	SpuStart();
}

static int do_spu_intr(void *userdata)
{
	(void)userdata;
	// Unofficial: Always return 0 (callback never set!)
	return 0;
}

static int intr_flag_spu12[2] = {0, 1};

static void SpuStart(void)
{
	CpuDisableIntr();
	ReleaseIntrHandler(IOP_IRQ_DMA_SPU);
	ReleaseIntrHandler(IOP_IRQ_DMA_SPU2);
	RegisterIntrHandler(IOP_IRQ_DMA_SPU, 1, do_dma_finish, &intr_flag_spu12[0]);
	RegisterIntrHandler(IOP_IRQ_DMA_SPU2, 1, do_dma_finish, &intr_flag_spu12[1]);
	CpuEnableIntr();
	ReleaseIntrHandler(IOP_IRQ_SPU);
	RegisterIntrHandler(IOP_IRQ_SPU, 1, do_spu_intr, 0);
}

static void spu_init_(void)
{
	vu16 *v0;
	vu16 *spu_RXX_;
	vu32 *spu_sys_pcr_;
	int i;

	// Unofficial: Make variables local
	spu_RXX_ = (vu16 *)0xBF900000;
	spu_sys_pcr_ = (vu32 *)0xBF8010F0;
	*spu_sys_pcr_ |= 0xB0000u;
	v0 = &spu_RXX_[0];
	v0[944] = 0;
	v0[945] = 0;
	v0[1456] = 0;
	v0[1457] = 0;
	do_dma_stop(0);
	do_dma_stop(1);
	for ( i = 0; i < 2; i += 1 )
	{
		vu16 *v3;
		vu16 *v4;
		unsigned int v5;
		vu16 *v6;

		// Unofficial: Remove setting of never read variables
		spu_RXX_[512 * i + 205] = 0;
		do_spu_wait();
		do_spu_wait();
		do_spu_wait();
		spu_RXX_[512 * i + 205] = 0x8000;
		do_spu_wait();
		do_spu_wait();
		do_spu_wait();
		v3 = &spu_RXX_[20 * i];
		v4 = &spu_RXX_[512 * i];
		v3[944] = 0;
		v3[945] = 0;
		v5 = 1;
		while ( (v4[418] & 0x7FF) != 0 )
		{
			if ( v5 >= 0xF01 )
			{
				printf("SPU:T/O [%s]\n", "wait (reset)");
				break;
			}
			++v5;
		}
		v6 = &spu_RXX_[512 * i];
		v6[210] = -1;
		v6[211] = -1;
	}
}

static int do_dma_finish(void *userdata)
{
	int which_core;

	which_core = (*(u32 *)userdata) & 0xFF;
	// Unofficial: Remove never used branch
	*(vu16 *)((which_core << 10) + 0xBF90019A) &= 0xFFCFu;
	*(vu16 *)((which_core << 10) + 0xBF900198) &= 0xFF3Fu;
	*(vu16 *)((which_core << 10) + 0xBF9001B0) = 0;
	// Unofficial: Remove calling of callback that is never set
	FlushDcache();
	return 1;
}

static void do_dma_stop(int which_core)
{
	*(vu16 *)((which_core << 10) + 0xBF90019A) &= 0xFFCFu;
	*(vu16 *)((which_core << 10) + 0xBF9001B0) = 0;
}

static int do_dma_get_status(int which_core)
{
	// Unofficial: Apply fixed variables
	if ( *(vu16 *)((which_core << 10) + 0xBF9001B0) )
		return (*(vu32 *)(1088 * which_core + 0xBF8010C0)) & 0xFFFFFF;
	return 0;
}

static void do_dma_start(void *addr, int size, int which_core)
{
	// Unofficial: Apply fixed variables
	*(vu16 *)((which_core << 10) + 0xBF90019A) &= 0xFFCFu;
	*(vu16 *)((which_core << 10) + 0xBF9001A8) = 0;
	*(vu16 *)((which_core << 10) + 0xBF9001AA) = 0;
	*(vu16 *)((which_core << 10) + 0xBF9001B0) = 1 << which_core;
	*(vu32 *)(1088 * which_core + 0xBF8010C0) = (uiptr)addr;
	*(vu16 *)(1088 * which_core + 0xBF8010C4) = 16;
	*(vu16 *)(1088 * which_core + 0xBF8010C6) = size / 64 + (size % 64 > 0);
	*(vu32 *)(1088 * which_core + 0xBF8010C8) = 16777729;
}

static void do_spu_wait(void)
{
	int i;
	int v1;

	v1 = 13;
	for ( i = 0; i < 80; ++i )
	{
		__asm__ __volatile__("" : "+g"(v1) : :);
		v1 *= 13;
	}
}
