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
#include <iop_mmio_hwport.h>
#include <loadcore.h>

#ifdef _IOP
IRX_ID("rmreset", 1, 1);
#endif
// Based on the module from ROM 1.20+.

static void sio2_ctrl_set(u32 val)
{
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->sio2.ctrl = val;
}

static u32 sio2_ctrl_get(void)
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->sio2.ctrl;
}

static u32 sio2_stat6c_get(void)
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->sio2.recv1;
}

static void sio2_portN_ctrl1_set(int N, u32 val)
{
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->sio2.send1_2_buf[N * 2] = val;
}

// Unused func removed

static void sio2_portN_ctrl2_set(int N, u32 val)
{
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->sio2.send1_2_buf[(N * 2) + 1] = val;
}

// Unused func removed

// Unused func removed

static void sio2_regN_set(int N, u32 val)
{
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->sio2.send3_buf[N] = val;
}

// Unused func removed

// Unused func removed

// Unused func removed

// Unused func removed

// Unused func removed

// Unused func removed

static void sio2_data_out(u8 val)
{
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->sio2.out_fifo = val;
}

static u8 sio2_data_in(void)
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->sio2.in_fifo;
}

// Unused func removed

// Unused func removed

int _start(int ac, char **av)
{
	u32 ctrl_save;
	int i;
	// Unofficial: shrink buffer
	char inoutbuf[7];

	(void)ac;
	(void)av;

	printf("rmreset start\n");
	ctrl_save = sio2_ctrl_get();
	sio2_ctrl_set(0xCu);
	for ( i = 0; i < 4; i += 1 )
	{
		sio2_portN_ctrl1_set(i, 0xC0C0050F);
		sio2_portN_ctrl2_set(i, 0x1060014u);
	}
	sio2_regN_set(0, 0x1C0740u);
	sio2_regN_set(1, 0);
	inoutbuf[0] = 0x61;
	inoutbuf[1] = 6;
	inoutbuf[2] = 3;
	for ( i = 3; i < (int)(sizeof(inoutbuf)); i += 1 )
		inoutbuf[i] = 0;
	for ( i = 0; i < (int)(sizeof(inoutbuf)); i += 1 )
		sio2_data_out(inoutbuf[i]);
	sio2_ctrl_set(0xB1u);
	while ( !((sio2_stat6c_get() >> 12) & 1) )
		;
	for ( i = 0; i < 7; i += 1 )
		sio2_data_in();
	sio2_ctrl_set(0xCu);
	for ( i = 0; i < 4; i += 1 )
	{
		sio2_portN_ctrl1_set(i, 0xC0C0050F);
		sio2_portN_ctrl2_set(i, 0x1060014u);
	}
	sio2_regN_set(0, 0x1C0741u);
	sio2_regN_set(1, 0);
	inoutbuf[0] = 0x61;
	inoutbuf[1] = 6;
	inoutbuf[2] = 3;
	for ( i = 3; i < (int)(sizeof(inoutbuf)); i += 1 )
		inoutbuf[i] = 0;
	for ( i = 0; i < (int)(sizeof(inoutbuf)); i += 1 )
		sio2_data_out(inoutbuf[i]);
	sio2_ctrl_set(0xB1u);
	while ( !((sio2_stat6c_get() >> 12) & 1) )
		;
	for ( i = 0; i < 7; i += 1 )
		sio2_data_in();
	sio2_ctrl_set(0xCu);
	sio2_ctrl_set(ctrl_save & ~1);
	printf("rmreset end\n");
	return MODULE_NO_RESIDENT_END;
}
