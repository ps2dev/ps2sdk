/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# UDP-based TTY for Pukklink-compatible clients.
*/

#ifndef UDPTTY_H
#define UDPTTY_H

#include "types.h"
#include "defs.h"
#include "irx.h"

#include "thbase.h"
#include "thsemap.h"
#define IOMAN_NO_EXTENDED
#include "iomanX.h"
#include "dev9.h"
#include "errno.h"

#include "sys/fcntl.h"

#include "speedregs.h"
#include "smapregs.h"

#define MODNAME "udptty"

#define DEVNAME "tty" /* "test" */

/* These automatically convert the address and port to network order.  */
#define IP_ADDR(a, b, c, d)	(((d & 0xff) << 24) | ((c & 0xff) << 16) | \
				((b & 0xff) << 8) | ((a & 0xff)))
#define IP_PORT(port)	(((port & 0xff00) >> 8) | ((port & 0xff) << 8))

typedef struct {
	u8 eth_addr_dst[6];
	u8 eth_addr_src[6];
	u32 ip_addr_dst;
	u32 ip_addr_src;
	u16 ip_port_dst;
	u16 ip_port_src;
} udptty_param_t;

extern udptty_param_t udptty_param;
extern iop_device_t tty_device;

int smap_init(void);
int smap_txbd_check(void);
int smap_transmit(void *buf, size_t size);

int udp_init(void);
int udp_send(void *buf, size_t size);

#endif /* UDPTTY_H */
