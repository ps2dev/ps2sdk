/* udptty.c - UDP-based TTY for Pukklink-compatible clients.
 *
 * Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
 *
 * Licensed under the Academic Free License version 2.0.
 */

/* This code implements a simple UDP TTY for Pukklink clients.  */

#include "udptty.h"

IRX_ID(MODNAME, 1, 1);

udptty_param_t udptty_param = {
	{ UDPTTY_ETH_DST },
	{ UDPTTY_ETH_SRC },
	UDPTTY_IP_ADDR_DST, UDPTTY_IP_ADDR_SRC,
	UDPTTY_IP_PORT_DST, UDPTTY_IP_PORT_SRC
};

int _start(int argc, char *argv[])
{
	close(0);
	close(1);
	DelDrv(DEVNAME);

	if (AddDrv(&tty_device) < 0)
		return 1;

	open(DEVNAME "00:", 0x1000|O_RDWR);
	open(DEVNAME "00:", O_WRONLY);

	return 0;
}
