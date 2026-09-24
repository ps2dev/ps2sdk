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

#include "usbdpriv.h"
#include "driver.h"
#include "mem.h"
#include "hub.h"

#include "defs.h"
#include "stdio.h"
#include "sysclib.h"
#include "thbase.h"
#include "thevent.h"
#include "intrman.h"

sceUsbdLddOps *drvListStart = NULL, *drvListEnd = NULL;
sceUsbdLddOps *drvAutoLoader = NULL;
IoRequest *cbListStart = NULL, *cbListEnd = NULL;

int callbackEvent;
int callbackTid = -1;

int callUsbDriverFunc(int (*func)(int devId), int devId, void *gp)
{
    int res;

    if (func) {
        usbdUnlock();
#if USE_GP_REGISTER
        ChangeGP(gp);
#else
        (void)gp;
#endif
        res = func(devId);
#if USE_GP_REGISTER
        SetGP(&_gp);
#endif
        usbdLock();
        return res;
    } else
        return 0;
}

void probeDeviceTree(Device *tree, sceUsbdLddOps *drv)
{
    Device *curDevice;
    for (curDevice = tree->childListStart; curDevice != NULL; curDevice = curDevice->next)
        if (curDevice->deviceStatus == DEVICE_READY) {
            if (curDevice->devDriver == NULL) {
                if (callUsbDriverFunc(drv->probe, curDevice->id, drv->gp) != 0) {
                    curDevice->devDriver = drv;
                    callUsbDriverFunc(drv->connect, curDevice->id, drv->gp);
                }
            } else if (curDevice->childListStart)
                probeDeviceTree(curDevice, drv);
        }
}

int doRegisterDriver(sceUsbdLddOps *drv, void *drvGpSeg)
{
    if (drv->next || drv->prev)
        return USB_RC_BUSY;
    if (drvListStart == drv)
        return USB_RC_BUSY;

    if (!drv->name)
        return USB_RC_BADDRIVER;
    if (drv->reserved1 || drv->reserved2)
        return USB_RC_BADDRIVER;

    drv->gp = drvGpSeg;

    drv->prev = drvListEnd;
    if (drvListEnd)
        drvListEnd->next = drv;
    else
        drvListStart = drv;
    drvListEnd = drv;

    if (drv->probe)
        probeDeviceTree(memPool.deviceTreeRoot, drv);

    return 0;
}

int doRegisterAutoLoader(sceUsbdLddOps *drv, void *drvGpSeg)
{
    if (drv->next || drv->prev)
        return USB_RC_BADDRIVER;
    if (!drv->name)
        return USB_RC_BADDRIVER;
    if (drv->reserved1 || drv->reserved2)
        return USB_RC_BADDRIVER;

    if (drvAutoLoader != NULL)
        return USB_RC_BUSY;

    drv->gp = drvGpSeg;

    drvAutoLoader = drv;

    if (drv->probe)
        probeDeviceTree(memPool.deviceTreeRoot, drv);

    return 0;
}

void disconnectDriver(Device *tree, sceUsbdLddOps *drv)
{
    Endpoint *ep, *nextEp;
    if (tree->devDriver == drv) {
        if (tree->endpointListStart) {
            ep = tree->endpointListStart->next;

            while (ep) {
                nextEp = ep->next;
                removeEndpointFromDevice(tree, ep);
                ep = nextEp;
            }
        }
        tree->devDriver     = NULL;
        tree->privDataField = NULL;
    }

    for (tree = tree->childListStart; tree != NULL; tree = tree->next)
        disconnectDriver(tree, drv);
}

int doUnregisterAutoLoader(void)
{
    drvAutoLoader = NULL;
    return 0;
}

int doUnregisterDriver(sceUsbdLddOps *drv)
{
    sceUsbdLddOps *pos;
    for (pos = drvListStart; pos != NULL; pos = pos->next)
        if (pos == drv) {
            if (drv->next)
                drv->next->prev = drv->prev;
            else
                drvListEnd = drv->prev;

            if (drv->prev)
                drv->prev->next = drv->next;
            else
                drvListStart = drv->next;

            disconnectDriver(memPool.deviceTreeRoot, drv);
            return 0;
        }
    return USB_RC_BADDRIVER;
}

void connectNewDevice(Device *dev)
{
    sceUsbdLddOps *drv;
    dbg_printf("searching driver for dev %d, FA %02X\n", dev->id, dev->functionAddress);
    for (drv = drvListStart; drv != NULL; drv = drv->next)
        if (callUsbDriverFunc(drv->probe, dev->id, drv->gp) != 0) {
            dev->devDriver = drv;
            dbg_printf("Driver found (%s)\n", drv->name);
            callUsbDriverFunc(drv->connect, dev->id, drv->gp);
            return;
        }

    // No driver found yet. Call autoloader.
    if (drvAutoLoader != NULL) {
        drv = drvAutoLoader;

        if (callUsbDriverFunc(drv->probe, dev->id, drv->gp) != 0) {
            dev->devDriver = drv;
            dbg_printf("(autoloader) Driver found (%s)\n", drv->name);
            callUsbDriverFunc(drv->connect, dev->id, drv->gp);
            return;
        }
    }

    dbg_printf("no driver found\n");
}

void signalCallbackThreadFunc(IoRequest *req)
{
    int intrStat;

    CpuSuspendIntr(&intrStat);

    req->prev = cbListEnd;
    req->next = NULL;
    if (cbListEnd)
        cbListEnd->next = req;
    else
        cbListStart = req;
    cbListEnd = req;

    CpuResumeIntr(intrStat);

    SetEventFlag(callbackEvent, 1);
}

void callbackThreadFunc(void *arg)
{
    u32 eventRes;
    int intrStat;
    IoRequest *req;
    IoRequest reqCopy;

    (void)arg;

    while (1) {
        WaitEventFlag(callbackEvent, 1, WEF_CLEAR | WEF_OR, &eventRes);
        do {
            CpuSuspendIntr(&intrStat);

            req = cbListStart;
            if (req) {
                if (req->next)
                    req->next->prev = req->prev;
                else
                    cbListEnd = req->prev;

                if (req->prev)
                    req->prev->next = req->next;
                else
                    cbListStart = req->next;
            }
            CpuResumeIntr(intrStat);

            if (req) {
                memcpy(&reqCopy, req, sizeof(IoRequest));
                usbdLock();
                freeIoRequest(req);
                usbdUnlock();

                if (reqCopy.userCallbackProc) {
#if USE_GP_REGISTER
                    SetGP(req->gpSeg);
#endif
                    reqCopy.userCallbackProc(reqCopy.resultCode, reqCopy.transferedBytes, reqCopy.userCallbackArg);
#if USE_GP_REGISTER
                    SetGP(&_gp);
#endif
                }
            }
        } while (req);
    }
}

int initCallbackThread(void)
{
    iop_event_t event;
    iop_thread_t thread;

    event.attr = event.option = event.bits = 0;
    callbackEvent                          = CreateEventFlag(&event);

    thread.attr      = TH_C;
    thread.option    = 0;
    thread.thread    = callbackThreadFunc;
#ifndef MINI_DRIVER
    thread.stacksize = 0x4000; // 16KiB
#else
    thread.stacksize = 0x0800; //  2KiB
#endif
    thread.priority  = usbConfig.cbThreadPrio;
    callbackTid      = CreateThread(&thread);
    StartThread(callbackTid, NULL);

    return 0;
}
