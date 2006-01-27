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
#ifndef __USBIO_H__
#define __USBIO_H__

void removeEndpointFromQueue(Endpoint *ep);
void checkTdQueue(int type);
void handleIoReqList(Endpoint *ep);
int doControlTransfer(Endpoint *ep, IoRequest *req,
	uint8 requestType, uint8 request, uint16 value, uint16 index, uint16 length,
	void *destdata, void *callback);
int attachIoReqToEndpoint(Endpoint *ep, IoRequest *req, void *destdata, uint16 length, void *callback);
void handleIoReqList(Endpoint *ep);

#endif // __USBIO_H__
