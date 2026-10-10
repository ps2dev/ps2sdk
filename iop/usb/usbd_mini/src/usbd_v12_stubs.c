/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * usbd 1.2 export stubs for FreeUsbd-based usbd_mini.
 *
 * Full usbd (SCE rewrite) implements these; mass-storage / OPL only need 1.1.
 * Keep the export table at 1.2 so modules that probe the library version still load.
 */

#include "usbd.h"

void usbdReboot(int ac)
{
	(void)ac;
}

int sceUsbdGetReportDescriptor(int devId, int cfgNum, int ifNum, void **desc, u32 *len)
{
	(void)devId;
	(void)cfgNum;
	(void)ifNum;
	(void)desc;
	(void)len;
	return USB_RC_NOSUPPORT;
}

int sceUsbdMultiIsochronousTransfer(
	int pipeId, sceUsbdMultiIsochronousRequest *request, sceUsbdMultiIsochronousDoneCallback callback, void *cbArg)
{
	(void)pipeId;
	(void)request;
	(void)callback;
	(void)cbArg;
	return USB_RC_NOSUPPORT;
}
