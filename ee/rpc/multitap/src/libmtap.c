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
# Functions to provide access to multi-taps.
*/

#include <tamtypes.h>
#include <string.h>
#include <kernel.h>
#include <sifrpc.h>
#include <stdarg.h>

#include "libmtap.h"

#define MTAPSERV_PORT_OPEN			0x80000901
#define MTAPSERV_PORT_CLOSE			0x80000902
#define MTAPSERV_GET_CONNECTION		0x80000903

static unsigned int mtapRpcBuffer[32] __attribute__((aligned (64)));
static struct t_SifRpcClientData clientPortOpen __attribute__((aligned (64)));
static struct t_SifRpcClientData clientPortClose __attribute__((aligned (64)));
static struct t_SifRpcClientData clientGetConnection __attribute__((aligned (64)));
static int mtapInited = 0;

int mtapInit(void)
{
	int i;

	if(mtapInited) return -1;

	while(1)
	{
		if (SifBindRpc(&clientPortOpen, MTAPSERV_PORT_OPEN, 0) < 0) return -1;
 		if (clientPortOpen.server != 0) break;

    	i = 0x10000;
    	while(i--);
	}

	while(1)
	{
		if (SifBindRpc(&clientPortClose, MTAPSERV_PORT_CLOSE, 0) < 0) return -1;
 		if (clientPortClose.server != 0) break;

    	i = 0x10000;
    	while(i--);
	}

	while(1)
	{
		if (SifBindRpc(&clientGetConnection, MTAPSERV_GET_CONNECTION, 0) < 0) return -1;
 		if (clientGetConnection.server != 0) break;

    	i = 0x10000;
    	while(i--);
	}

	mtapInited = 1;

	return 1;
}

int mtapPortOpen(int port)
{
	if(!mtapInited) return -1;

	mtapRpcBuffer[0] = port;
	SifCallRpc(&clientPortOpen, 1, 0, mtapRpcBuffer, 4, mtapRpcBuffer, 8, 0, 0);

	return mtapRpcBuffer[1];
}

int mtapPortClose(int port)
{
	if(!mtapInited) return -1;

	mtapRpcBuffer[0] = port;
	SifCallRpc(&clientPortClose, 1, 0, mtapRpcBuffer, 4, mtapRpcBuffer, 8, 0, 0);

	return mtapRpcBuffer[1];
}

int mtapGetConnection(int port)
{
	if(!mtapInited) return -1;

	mtapRpcBuffer[0] = port;
	SifCallRpc(&clientGetConnection, 1, 0, mtapRpcBuffer, 4, mtapRpcBuffer, 8, 0, 0);

	return mtapRpcBuffer[1];
}
