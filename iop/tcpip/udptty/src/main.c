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
# UDP-based TTY for Pukklink-compatible clients.
*/

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
