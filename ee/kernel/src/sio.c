/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Routines for accessing the EE's on-chip serial port.
*/

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>

#include "tamtypes.h"
#include "kernel.h"
#include "string.h"
#include "sio.h"

#ifndef PS2LIB_STR_MAX
#define PS2LIB_STR_MAX 4096
#endif

#define CPUCLK		294912000	/* Use this to determine the baud divide value.  */
#define LCR_SCS_VAL	(1<<5)		/* Baud rate generator output that divided CPUCLK.  */

#ifdef F_sio_init
/* Initialize the SIO. The lcr_* parameters are passed as is, so
   you'll need to use the SIO_LCR_* register values. You can pass 0 for all of
   the lcr_* params to get the standard 8N1 setting (8 data bits, no parity
   checking, 1 stop bit). Note: Unlike the BIOS sio_init() routine, we always
   base the baud rate on the CPU clock.  */
void sio_init(u32 baudrate, u8 lcr_ueps, u8 lcr_upen, u8 lcr_usbl, u8 lcr_umode)
{
	u32 brd;		/* Baud rate divisor.  */
	u8 bclk = 0;		/* Baud rate generator clock.  */

	_sw(LCR_SCS_VAL|((lcr_ueps & 1) << 4)|((lcr_upen & 1) << 3)|
			((lcr_usbl & 1) << 2)|(lcr_umode & 1), SIO_LCR);

	/* Disable all interrupts.  */
	_sw(0, SIO_IER);

	/* Reset the FIFOs.  */
	_sw(SIO_FCR_FRSTE|SIO_FCR_RFRST|SIO_FCR_TFRST, SIO_FCR);
	/* Enable them.  */
	_sw(0, SIO_FCR);

	brd = CPUCLK / (baudrate * 256);

	while ((brd >= 256) && (++bclk < 4))
		brd /= 4;

	_sw((bclk << 8) | brd, SIO_BGR);
}
#endif

#ifdef F_sio_putc
int sio_putc(int c)
{
	/* Block until we're ready to transmit.  */
	while ((_lw(SIO_ISR) & 0xf000) == 0x8000)
		;

	_sb(c, SIO_TXFIFO);
	return c;
}
#endif

#ifdef F_sio_getc
int sio_getc()
{
	/* Do we have something in the RX FIFO?  */
	if (_lw(SIO_ISR) & 0xf00)
		return _lb(SIO_RXFIFO);

	/* Return EOF.  */
	return -1;
}
#endif

#ifdef F_sio_write
/* Not really much to do in the way of error-handling.  sio_putc() already
   checks to see if there's room in the TX FIFO, and my cable doesn't support
   hardware flow control.  */
size_t sio_write(void *buf, size_t size)
{
	u8 *p = (u8 *)buf;
	size_t i;

	for (i = 0; i < size; i++)
		sio_putc(p[i]);
	
	return size;
}
#endif

#ifdef F_sio_read
/* This will read from the serial port until size characters have been read or
   EOF (RX FIFO is empty).  */
size_t sio_read(void *buf, size_t size)
{
	u8 *p = (u8 *)buf;
	size_t i;
	int c;

	for (i = 0; i < size; i++) {
		if ((c = sio_getc()) == -1)
			break;

		p[i] = (u8)c;
	}

	return i;
}
#endif

#ifdef F_sio_puts
int sio_puts(const char *str)
{
	int res;

	res = sio_write((void *)str, strlen(str));

	sio_putc('\r');
	sio_putc('\n');
	return res + 2;
}
#endif

#ifdef F_sio_gets
/* Hmm ... this might not work as intended.  I think as soon as this is called,
   unless there were already chars in the RX FIFO, it will return EOF
   immediately.  */
char *sio_gets(char *str)
{
	char *s = str;
	int c;

	while ((c = sio_getc()) != -1) {
		/* Check for newline.  */
		if (c == '\n' || c == '\r')
			break;

		*s++ = c;
	}

	*s = '\0';
	return str;
}
#endif

#ifdef F_sio_printf
int sio_printf(const char *format, ...)
{
	static char buf[PS2LIB_STR_MAX];
	va_list args;
	int size;

	va_start(args, format);
	size = vsnprintf(buf, PS2LIB_STR_MAX, format, args);
	va_end(args);

	/* A bit hackish, but if the last character is '\n' then strip it off
	   and pass the string to sio_puts().  */
	if (buf[size - 1] == '\n') {
		buf[size - 1] = '\0';
		size++;			/* Account for the '\r'.  */
		sio_puts(buf);
	} else {
		sio_write(buf, size);
	}

	return size;
}
#endif
