/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * USB Driver function prototypes and constants.
 */

#ifndef __MEM_H__
#define __MEM_H__

#include "usbdpriv.h"

extern MemoryPool memPool;

HcIsoTD *allocIsoTd(void);
void freeIsoTd(HcIsoTD *argTd);

HcTD *allocTd(void);
void freeTd(HcTD *argTd);

Device *attachChildDevice(Device *parent, u32 portNum);
void freeDevice(Device *dev);

Device *fetchPortElemByNumber(Device *hub, int port);

void addToHcEndpointList(u8 type, HcED *ed);
void removeHcEdFromList(int type, const HcED *hcEd);

Endpoint *allocEndpointForDevice(Device *dev, u32 align);

Device *fetchDeviceById(int devId);
Endpoint *fetchEndpointById(int id);

IoRequest *allocIoRequest(void);
void freeIoRequest(IoRequest *req);

#endif
