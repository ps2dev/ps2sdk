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
 * EE Serial I/O
 */

#ifndef __SIO_H__
#define __SIO_H__

#include <tamtypes.h>
#include <stddef.h>

/* SIO Registers.  */
/* Most of these are based off of Toshiba documentation for the TX49 and the
   TX79 companion chip. However, looking at the kernel SIOP (Debug) exception
   handler, it looks like some registers are borrowed from the TX7901 UART
   (0x1000f110 or LSR, in particular). I'm still trying to find the correct
   register names and values.  */
/** Line Control Register.  */
#define SIO_LCR            0x1000f100
/** UART Mode.  */
#define SIO_LCR_UMODE_8BIT 0x00
#define SIO_LCR_UMODE_7BIT 0x01
/** UART Stop Bit Length.  */
#define SIO_LCR_USBL_1BIT  0x00
#define SIO_LCR_USBL_2BITS 0x01
/** UART Parity Enable.  */
#define SIO_LCR_UPEN_OFF   0x00
#define SIO_LCR_UPEN_ON    0x01
/** UART Even Parity Select.  */
#define SIO_LCR_UEPS_ODD   0x00
#define SIO_LCR_UEPS_EVEN  0x01

/** Line Status Register.  */
#define SIO_LSR    0x1000f110
/** Data Ready. (Not tested) */
#define SIO_LSR_DR 0x01
/** Overrun Error.  */
#define SIO_LSR_OE 0x02
/** Parity Error.  */
#define SIO_LSR_PE 0x04
/** Framing Error.  */
#define SIO_LSR_FE 0x08

/** Interrupt Enable Register.  */
#define SIO_IER       0x1000f120
/** Enable Received Data Available Interrupt */
#define SIO_IER_ERDAI 0x01
/** Enable Line Status Interrupt.  */
#define SIO_IER_ELSI  0x04

/** Interrupt Status Register (?).  */
#define SIO_ISR          0x1000f130
#define SIO_ISR_RX_DATA  0x01
#define SIO_ISR_TX_EMPTY 0x02
#define SIO_ISR_RX_ERROR 0x04

/** FIFO Control Register.  */
#define SIO_FCR       0x1000f140
/** FIFO Reset Enable.  */
#define SIO_FCR_FRSTE 0x01
/** RX FIFO Reset.  */
#define SIO_FCR_RFRST 0x02
/** TX FIFO Reset.  */
#define SIO_FCR_TFRST 0x04

/** Baud Rate Control Register.  */
#define SIO_BGR 0x1000f150

/** Transmit FIFO.  */
#define SIO_TXFIFO 0x1000f180
/** Receive FIFO.  */
#define SIO_RXFIFO 0x1000f1c0

/** The bit in the EE cause register which indicates an SIO exception */
#define SIO_CAUSE_BIT (1 << 12)

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the SIO.
 * The lcr_* parameters are passed as is, so you'll need to use the SIO_LCR_* register values.
 * You can pass 0 for all of the lcr_* params to get the standard 8N1 setting (8 data bits, no parity checking, 1 stop bit).
 * Note: Unlike the BIOS sio_init() routine, we always base the baud rate on the CPU clock.
 */
void sio_init(u32 baudrate, u8 lcr_ueps, u8 lcr_upen, u8 lcr_usbl, u8 lcr_umode);

int sio_putc(int c);
int sio_getc(void);
/** Same as sio_getc, but blocking.
 * Note that getc should be blocking by default. Ho well.
 */
int sio_getc_block(void);

/** Not really much to do in the way of error-handling.  sio_putc() already
 * checks to see if there's room in the TX FIFO, and my cable doesn't support
 * hardware flow control.
 */
size_t sio_write(void *buf, size_t size);
/** This will read from the serial port until size characters have been read or EOF (RX FIFO is empty).  */
size_t sio_read(void *buf, size_t size);

int sio_puts(const char *str);
int sio_putsn(const char *str); // no newline for this one
/** Will block until it recieves \n or \r. */
char *sio_gets(char *str);

/** Flushes the input buffer. */
void sio_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* __SIO_H__ */
