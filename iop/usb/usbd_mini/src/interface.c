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

#include <defs.h>
#include <thbase.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#include "usbdpriv.h"
#include "driver.h"
#include "mem.h"
#include "hcd.h"
#include "usbio.h"

extern UsbdConfig usbConfig;
extern int hcdTid;
extern int callbackTid;

int sceUsbdRegisterLdd(sceUsbdLddOps *driver)
{
    int res;
#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif
    if (usbdLock() != 0) {
#if USE_GP_REGISTER
        SetGP(OldGP);
#endif
        return USB_RC_BADCONTEXT;
    }

#if USE_GP_REGISTER
    res = doRegisterDriver(driver, OldGP);
#else
    res = doRegisterDriver(driver, NULL);
#endif

    usbdUnlock();
#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return res;
}

int sceUsbdRegisterAutoloader(sceUsbdLddOps *drv)
{
    int res;
#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif
    if (usbdLock() != 0) {
#if USE_GP_REGISTER
        SetGP(OldGP);
#endif
        return USB_RC_BADCONTEXT;
    }

#if USE_GP_REGISTER
    res = doRegisterAutoLoader(drv, OldGP);
#else
    res = doRegisterAutoLoader(drv, NULL);
#endif

    usbdUnlock();
#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return res;
}

int sceUsbdUnregisterLdd(sceUsbdLddOps *driver)
{
    int res;
#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif
    if (usbdLock() != 0) {
#if USE_GP_REGISTER
        SetGP(OldGP);
#endif
        return USB_RC_BADCONTEXT;
    }

    res = doUnregisterDriver(driver);

    usbdUnlock();
#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return res;
}

int sceUsbdUnregisterAutoloader(void)
{
    int res;
#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif
    if (usbdLock() != 0) {
#if USE_GP_REGISTER
        SetGP(OldGP);
#endif
        return USB_RC_BADCONTEXT;
    }

    res = doUnregisterAutoLoader();

    usbdUnlock();
#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return res;
}

void *sceUsbdScanStaticDescriptor(int devId, void *data, u8 type)
{
    void *res;
#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif
    if (usbdLock() != 0) {
#if USE_GP_REGISTER
        SetGP(OldGP);
#endif
        return NULL;
    }

    res = doGetDeviceStaticDescriptor(devId, data, type);

    usbdUnlock();
#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return res;
}

int sceUsbdGetDeviceLocation(int devId, u8 *path)
{
    Device *dev;
    int res;
#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif
    if (usbdLock() != 0) {
#if USE_GP_REGISTER
        SetGP(OldGP);
#endif
        return USB_RC_BADCONTEXT;
    }

    dev = fetchDeviceById(devId);
    if (dev)
        res = doGetDeviceLocation(dev, path);
    else
        res = USB_RC_BADDEV;

    usbdUnlock();
#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return res;
}

int sceUsbdSetPrivateData(int devId, void *data)
{
    Device *dev;
    int res = USB_RC_OK;
#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif
    if (usbdLock() != 0) {
#if USE_GP_REGISTER
        SetGP(OldGP);
#endif
        return USB_RC_BADCONTEXT;
    }

    dev = fetchDeviceById(devId);
    if (dev)
        dev->privDataField = data;
    else
        res = USB_RC_BADDEV;

    usbdUnlock();
#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return res;
}

void *sceUsbdGetPrivateData(int devId)
{
    Device *dev;
    void *res = NULL;
#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif
    if (usbdLock() != 0) {
#if USE_GP_REGISTER
        SetGP(OldGP);
#endif
        return NULL;
    }

    dev = fetchDeviceById(devId);
    if (dev)
        res = dev->privDataField;

    usbdUnlock();
#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return res;
}

int sceUsbdOpenPipe(int devId, UsbEndpointDescriptor *desc)
{
    Device *dev;
    Endpoint *ep;
    int res = -1;
#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif
    if (usbdLock() != 0) {
#if USE_GP_REGISTER
        SetGP(OldGP);
#endif
        return -1;
    }

    dev = fetchDeviceById(devId);
    if (dev) {
        ep = doOpenEndpoint(dev, desc, 0);
        if (ep)
            res = ep->id;
    }

    usbdUnlock();
#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return res;
}

int sceUsbdClosePipe(int id)
{
    Endpoint *ep;
    int res;
#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif
    if (usbdLock() != 0) {
#if USE_GP_REGISTER
        SetGP(OldGP);
#endif
        return -1;
    }

    ep = fetchEndpointById(id);
    if (ep)
        res = doCloseEndpoint(ep);
    else
        res = USB_RC_BADPIPE;

    usbdUnlock();
#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return res;
}

int sceUsbdTransferPipe(int id, void *data, u32 len, void *option, sceUsbdDoneCallback callback, void *cbArg)
{
    UsbDeviceRequest *ctrlPkt = (UsbDeviceRequest *)option;
    IoRequest *req;
    Endpoint *ep;
    int res = 0;
#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif
    if (usbdLock() != 0) {
#if USE_GP_REGISTER
        SetGP(OldGP);
#endif
        return USB_RC_BADCONTEXT;
    }

    ep = fetchEndpointById(id);
    if (!ep) {
        dbg_printf("sceUsbdTransferPipe: Endpoint %d not found\n", id);
        res = USB_RC_BADPIPE;
    }

    if ((res == 0) && data && len) {
        if ((((u32)((u8 *)data + len - 1) >> 12) - ((u32)data >> 12)) > 1)
            res = USB_RC_BADLENGTH;
        else if (ep->alignFlag && ((u32)data & 3))
            res = USB_RC_BADALIGN;
        else if ((ep->endpointType == TYPE_ISOCHRON) && ((ep->hcEd.maxPacketSize & 0x7FF) < len))
            res = USB_RC_BADLENGTH;
    }
    if (res == 0) {
        req = allocIoRequest();
        if (!req) {
            dbg_printf("Ran out of IoReqs\n");
            res = USB_RC_IOREQ;
        }
    }
    if (res == 0) {
        req->userCallbackProc = callback;
        req->userCallbackArg  = cbArg;
#if USE_GP_REGISTER
        req->gpSeg = GetGP(); // gp of the calling module
#endif
        if (ep->endpointType == TYPE_CONTROL) {
            if (!ctrlPkt) {
                res = USB_RC_BADOPTION;
                freeIoRequest(req);
            } else if (ctrlPkt->length != len) {
                res = USB_RC_BADLENGTH;
                freeIoRequest(req);
            } else {
                res = doControlTransfer(ep, req,
                                        ctrlPkt->requesttype, ctrlPkt->request, ctrlPkt->value, ctrlPkt->index, ctrlPkt->length,
                                        data, signalCallbackThreadFunc);
            }
        } else {
            if (ep->endpointType == TYPE_ISOCHRON)
                req->waitFrames = (u32)option;
            res = attachIoReqToEndpoint(ep, req, data, len, signalCallbackThreadFunc);
        }
    }

    usbdUnlock();
#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return res;
}

int sceUsbdOpenPipeAligned(int devId, UsbEndpointDescriptor *desc)
{
    Device *dev;
    Endpoint *ep;
    int res = -1;
#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif
    if (usbdLock() != 0) {
#if USE_GP_REGISTER
        SetGP(OldGP);
#endif
        return -1;
    }

    dev = fetchDeviceById(devId);
    if (dev) {
        ep = doOpenEndpoint(dev, desc, 1);
        if (ep)
            res = ep->id;
    }

    usbdUnlock();
#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return res;
}

int sceUsbdChangeThreadPriority(int prio1, int prio2)
{
    int res;
#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif
    if (usbdLock() != 0) {
#if USE_GP_REGISTER
        SetGP(OldGP);
#endif
        return USB_RC_BADCONTEXT;
    }

    res = 0;

    if (usbConfig.hcdThreadPrio != prio1) {
        usbConfig.hcdThreadPrio = prio1;
        res                     = ChangeThreadPriority(hcdTid, prio1);
    }

    if (usbConfig.cbThreadPrio != prio2) {
        usbConfig.cbThreadPrio = prio2;
        res                    = ChangeThreadPriority(callbackTid, prio2);
    }

    usbdUnlock();
#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return res;
}
