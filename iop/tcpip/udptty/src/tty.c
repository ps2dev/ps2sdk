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
# TTY filesystem for UDPTTY.
*/

#include "udptty.h"

static int tty_sema = -1;

/* TTY driver.  */

static int tty_init(iop_device_t *device)
{
	int res;

	if ((res = smap_init()) != 0)
		return -res;

	if ((res = udp_init()) < 0)
		return res;

	if ((tty_sema = CreateMutex(IOP_MUTEX_UNLOCKED)) < 0)
		return -1;

	return 0;
}

static int tty_deinit(iop_device_t *device)
{
	DeleteSema(tty_sema);
	return 0;
}

static int tty_stdout_fd(void) { return 1; }

static int tty_write(iop_file_t *file, void *buf, size_t size)
{
	int res = 0;

	WaitSema(tty_sema);
	res = udp_send(buf, size);

	SignalSema(tty_sema);
	return res;
}

static int tty_error(void) { return -EIO; }

static iop_device_ops_t tty_ops = { tty_init, tty_deinit, (void *)tty_error, 
	(void *)tty_stdout_fd, (void *)tty_stdout_fd, (void *)tty_error, 
	(void *)tty_write, (void *)tty_error, (void *)tty_error,
	(void *)tty_error, (void *)tty_error, (void *)tty_error,
	(void *)tty_error, (void *)tty_error, (void *)tty_error,
	(void *)tty_error,  (void *)tty_error };

iop_device_t tty_device =
{ DEVNAME, IOP_DT_CHAR|IOP_DT_CONS, 1, "TTY via SMAP UDP", &tty_ops };
