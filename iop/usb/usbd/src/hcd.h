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
# USB Driver function prototypes and constants.
*/
#ifndef __HCD_H__
#define __HCD_H__

#include "usbdpriv.h"

int callUsbDriverFunc(int (*func)(int), int devId, void *gp);
Endpoint *openDeviceEndpoint(Device *dev, UsbEndpointDescriptor *endpDesc, uint32 alignFlag);
Endpoint *doOpenEndpoint(Device *dev, UsbEndpointDescriptor *endpDesc, uint32 alignFlag);
void *doGetDeviceStaticDescriptor(int devId, void *data, uint8 type);
int doCloseEndpoint(Endpoint *ep);
int hcdInit(void);

#endif // __HCD_H__

