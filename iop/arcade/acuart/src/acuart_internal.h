/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACUART_INTERNAL_H
#define _ACUART_INTERNAL_H

#include <acuart.h>
#include <irx_imports.h>

typedef volatile acUint16 *acUartReg;

struct uart_buf
{
	acUint8 *ub_buf;
	acUint32 ub_size;
	acUint32 ub_head;
	acUint32 ub_tail;
};

struct uart_softc
{
	int eve;
	acUint32 speed;
	acUint32 fifo;
	acUint32 loopback;
	struct uart_buf xmit;
	struct uart_buf recv;
};

#ifdef TTY_DEVICE
int CreateTTY();
#endif

#endif
