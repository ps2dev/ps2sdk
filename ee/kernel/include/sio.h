/*
  _____     ___ ____
   ____|   |    ____|      PS2 OpenSource Project
  |     ___|   |____       (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
  
  ------------------------------------------------------------------------
  sio.h                    EE Serial I/O
*/

#ifndef EE_SIO_H
#define EE_SIO_H

#include "tamtypes.h"

/* SIO Registers.  */
/* Most of these are based off of Toshiba documentation for the TX49 and the
   TX79 companion chip. However, looking at the kernel SIOP (Debug) exception
   handler, it looks like some registers are borrowed from the TX7901 UART
   (0x1000f110 or LSR, in particular). I'm still trying to find the correct
   register names and values.  */
#define SIO_LCR		0x1000f100	/* Line Control Register.  */
#define   SIO_LCR_UMODE_8BIT 0x00 /* UART Mode.  */
#define   SIO_LCR_UMODE_7BIT 0x01
#define   SIO_LCR_USBL_1BIT  0x00 /* UART Stop Bit Length.  */
#define   SIO_LCR_USBL_2BITS 0x01
#define   SIO_LCR_UPEN_OFF   0x00 /* UART Parity Enable.  */
#define   SIO_LCR_UPEN_ON    0x01
#define   SIO_LCR_UEPS_ODD   0x00 /* UART Even Parity Select.  */
#define   SIO_LCR_UEPS_EVEN  0x01

#define SIO_LSR		0x1000f110	/* Line Status Register.  */
#define   SIO_LSR_DR 0x01	/* Data Ready. (Not tested) */
#define   SIO_LSR_OE 0x02	/* Overrun Error.  */
#define   SIO_LSR_PE 0x04	/* Parity Error.  */
#define   SIO_LSR_FE 0x08	/* Framing Error.  */

#define SIO_IER		0x1000f120	/* Interrupt Enable Register.  */
#define   SIO_IER_ELSI 0x04	/* Enable Line Status Interrupt.  */

#define SIO_ISR		0x1000f130	/* Interrupt Status Register (?).  */

#define SIO_FCR		0x1000f140	/* FIFO Control Register.  */
#define   SIO_FCR_FRSTE 0x01	/* FIFO Reset Enable.  */
#define   SIO_FCR_RFRST 0x02	/* RX FIFO Reset.  */
#define   SIO_FCR_TFRST 0x04	/* TX FIFO Reset.  */

#define SIO_BGR		0x1000f150	/* Baud Rate Control Register.  */

#define SIO_TXFIFO	0x1000f180	/* Transmit FIFO.  */
#define SIO_RXFIFO	0x1000f1c0	/* Receive FIFO.  */

#ifdef __cplusplus
extern "C" {
#endif

void sio_init(u32 baudrate, u8 lcr_ueps, u8 lcr_upen, u8 lcr_usbl, u8 lcr_umode);

int sio_putc(int c);
int sio_getc(void);

size_t sio_write(void *buf, size_t size);
size_t sio_read(void *buf, size_t size);

int sio_puts(const char *str);
char *sio_gets(char *str);

int sio_printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* EE_SIO_H */
