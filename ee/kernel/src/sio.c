/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Routines for accessing the EE's on-chip serial port.
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

/** Use this to determine the baud divide value.  */
#define CPUCLK		294912000
/** Baud rate generator output that divided CPUCLK.  */
#define LCR_SCS_VAL	(1<<5)	

#ifdef F_sio_init
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
static u8 ___last_sio_putc = 0;
int sio_putc(int c)
{
    if((c == '\n') && (___last_sio_putc != '\r'))
    {
        // hack: if the character to be outputted is a '\n'
        //  and the previously-outputted character is not a '\r',
        //  output a '\r' first.
        sio_putc('\r');
    }

    /* Block until we're ready to transmit.  */
    while ((_lw(SIO_ISR) & 0xf000) == 0x8000);

    _sb(c, SIO_TXFIFO);
    ___last_sio_putc = c;
    return c;
}
#endif

#ifdef F_sio_getc
int sio_getc()
{
	/* Do we have something in the RX FIFO?  */
	if (_lw(SIO_ISR) & 0xf00) {
		u8 b = _lb(SIO_RXFIFO);
		_sw(7, SIO_ISR);
		return b;
	}

	/* Return EOF.  */
	return -1;
}
#endif

#ifdef F_sio_getc_block
int sio_getc_block()
{
	/* Do we have something in the RX FIFO?  */
	while (!(_lw(SIO_ISR) & 0xf00));
	return sio_getc();
}
#endif

#ifdef F_sio_write
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

	res = sio_putsn(str);

	sio_putc('\r');
	sio_putc('\n');
	return res + 2;
}
#endif

#ifdef F_sio_putsn
int sio_putsn(const char *str)
{
	int res;

	for (res = 0; *str; res++, str++)
	    sio_putc(*str);

	return res;
}
#endif

#ifdef F_sio_gets
char *sio_gets(char *str)
{
	char *s = str;
	int c;

	while (1) {
		c = sio_getc_block();
		/* Check for newline.  */
		if (c == '\n' || c == '\r')
			break;

		*s++ = c;
	}

	*s = '\0';
	return str;
}
#endif

#ifdef F_sio_flush
void sio_flush()
{
    while (_lw(SIO_ISR) & 0xf00)
	_lb(SIO_RXFIFO);
}
#endif
