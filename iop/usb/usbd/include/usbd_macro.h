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

#ifndef __USBD_MACRO_H__
#define __USBD_MACRO_H__

#include <usbd.h>

static int sceUsbdControlTransfer(int epID, int reqtyp, int req, int val, int index, int leng, void *dataptr, void *doneCB, void *arg)
{
    UsbDeviceRequest devreq;
    devreq.requesttype = reqtyp;
    devreq.request     = req;
    devreq.value       = val;
    devreq.index       = index;
    devreq.length      = leng;

    return sceUsbdTransferPipe(epID, dataptr, devreq.length, &devreq, doneCB, arg);
}

#define sceUsbdIsochronousTransfer(epID, dataptr, len, delta, doneCB, arg) \
    sceUsbdTransferPipe((epID), (dataptr), (len), (void *)(delta), (doneCB), (arg))

#define sceUsbdBulkTransfer(epID, dataptr, len, doneCB, arg) \
    sceUsbdTransferPipe((epID), (dataptr), (len), NULL, (doneCB), (arg))

#define sceUsbdInterruptTransfer(epID, dataptr, len, doneCB, arg) \
    sceUsbdTransferPipe((epID), (dataptr), (len), NULL, (doneCB), (arg))

/* standard control transfers */

#define sceUsbdClearDeviceFeature(epID, feature, doneCB, arg)                             \
    sceUsbdControlTransfer((epID), USB_DIR_OUT | USB_RECIP_DEVICE, USB_REQ_CLEAR_FEATURE, \
                           (feature), 0, 0, NULL, (doneCB), (arg))

#define sceUsbdSetDeviceFeature(epID, feature, doneCB, arg)                             \
    sceUsbdControlTransfer((epID), USB_DIR_OUT | USB_RECIP_DEVICE, USB_REQ_SET_FEATURE, \
                           (feature), 0, 0, NULL, (doneCB), (arg))

#define sceUsbdGetConfiguration(epID, dataptr, doneCB, arg)                                  \
    sceUsbdControlTransfer((epID), USB_DIR_IN | USB_RECIP_DEVICE, USB_REQ_GET_CONFIGURATION, \
                           0, 0, 1, (dataptr), (doneCB), (arg))

#define sceUsbdSetConfiguration(epID, config, doneCB, arg) \
    sceUsbdControlTransfer((epID), USB_DIR_OUT | USB_RECIP_DEVICE, USB_REQ_SET_CONFIGURATION, (config), 0, 0, NULL, (doneCB), (arg))

#define sceUsbdGetDescriptor(epID, type, index, language, dataptr, len, doneCB, arg)      \
    sceUsbdControlTransfer((epID), USB_DIR_IN | USB_RECIP_DEVICE, USB_REQ_GET_DESCRIPTOR, \
                           ((type) << 8) | (index), (language), (len), (dataptr), (doneCB), (arg))

#define sceUsbdSetDeviceDescriptor(epID, type, index, language, dataptr, len, doneCB, arg) \
    sceUsbdControlTransfer((epID), USB_DIR_OUT | USB_RECIP_DEVICE, USB_REQ_SET_DESCRIPTOR, \
                           ((type) << 8) | (index), (language), (len), (dataptr), (doneCB), (arg))

#define sceUsbdGetDeviceStatus(epID, dataptr, doneCB, arg)                            \
    sceUsbdControlTransfer((epID), USB_DIR_IN | USB_RECIP_DEVICE, USB_REQ_GET_STATUS, \
                           0, 0, 2, (dataptr), (doneCB), (arg))

#define sceUsbdSetAddress(epID, address, doneCB, arg)                                   \
    sceUsbdControlTransfer((epID), USB_DIR_OUT | USB_RECIP_DEVICE, USB_REQ_SET_ADDRESS, \
                           (address), 0, 0, NULL, (doneCB), (arg))

#define sceUsbdClearInterfaceFeature(epID, feature, interface, doneCB, arg)                  \
    sceUsbdControlTransfer((epID), USB_DIR_OUT | USB_RECIP_INTERFACE, USB_REQ_CLEAR_FEATURE, \
                           (feature), (interface), 0, NULL, (doneCB), (arg))

#define sceUsbdSetInterfaceFeature(epID, feature, interface, doneCB, arg)                  \
    sceUsbdControlTransfer((epID), USB_DIR_OUT | USB_RECIP_INTERFACE, USB_REQ_SET_FEATURE, \
                           (feature), (interface), 0, NULL, (doneCB), (arg))

#define sceUsbdGetInterface(epID, interface, dataptr, doneCB, arg)                          \
    sceUsbdControlTransfer((epID), USB_DIR_IN | USB_RECIP_INTERFACE, USB_REQ_GET_INTERFACE, \
                           0, (interface), 1, (dataptr), (doneCB), (arg))

#define sceUsbdSetInterface(epID, interface, alt_setting, doneCB, arg)                       \
    sceUsbdControlTransfer((epID), USB_DIR_OUT | USB_RECIP_INTERFACE, USB_REQ_SET_INTERFACE, \
                           (alt_setting), (interface), 0, NULL, (doneCB), (arg))

#define sceUsbdGetInterfaceDescriptor(epID, type, index, language, dataptr, len, doneCB, arg) \
    sceUsbdControlTransfer((epID), USB_DIR_IN | USB_RECIP_INTERFACE, USB_REQ_GET_DESCRIPTOR,  \
                           ((type) << 8) | (index), (language), (len), (dataptr), (doneCB), (arg))

#define sceUsbdSetInterfaceDescriptor(epID, type, index, language, dataptr, len, doneCB, arg) \
    sceUsbdControlTransfer((epID), USB_DIR_OUT | USB_RECIP_INTERFACE, USB_REQ_SET_DESCRIPTOR, \
                           ((type) << 8) | (index), (language), (len), (dataptr), (doneCB), (arg))

#define sceUsbdGetInterfaceStatus(epID, interface, dataptr, doneCB, arg)                 \
    sceUsbdControlTransfer((epID), USB_DIR_IN | USB_RECIP_INTERFACE, USB_REQ_GET_STATUS, \
                           0, (interface), 2, (dataptr), (doneCB), (arg))

#define sceUsbdClearEndpointFeature(epID, feature, endpoint, doneCB, arg)                   \
    sceUsbdControlTransfer((epID), USB_DIR_OUT | USB_RECIP_ENDPOINT, USB_REQ_CLEAR_FEATURE, \
                           (feature), (endpoint), 0, NULL, (doneCB), (arg))

#define sceUsbdSetEndpointFeature(epID, feature, endpoint, doneCB, arg)                   \
    sceUsbdControlTransfer((epID), USB_DIR_OUT | USB_RECIP_ENDPOINT, USB_REQ_SET_FEATURE, \
                           (feature), (endpoint), 0, NULL, (doneCB), (arg))

#define sceUsbdGetEndpointStatus(epID, endpoint, dataptr, doneCB, arg)                  \
    sceUsbdControlTransfer((epID), USB_DIR_IN | USB_RECIP_ENDPOINT, USB_REQ_GET_STATUS, \
                           0, (endpoint), 2, (dataptr), (doneCB), (arg))

#define sceUsbdGetEndpointDescriptor(epID, type, index, language, dataptr, len, doneCB, arg) \
    sceUsbdControlTransfer((epID), USB_DIR_IN | USB_RECIP_ENDPOINT, USB_REQ_GET_DESCRIPTOR,  \
                           ((type) << 8) | (index), (language), (len), (dataptr), (doneCB), (arg))

#define sceUsbdSetEndpointDescriptor(epID, type, index, language, dataptr, len, doneCB, arg) \
    sceUsbdControlTransfer((epID), USB_DIR_OUT | USB_RECIP_ENDPOINT, USB_REQ_SET_DESCRIPTOR, \
                           ((type) << 8) | (index), (language), (len), (dataptr), (doneCB), (arg))

#define sceUsbdSynchFrame(epID, endpoint, pfn, doneCB, arg)                              \
    sceUsbdControlTransfer((epID), USB_DIR_IN | USB_RECIP_ENDPOINT, USB_REQ_SYNCH_FRAME, \
                           0, (endpoint), 2, (pfn), (doneCB), (arg))

// For backwards compatibility:
#define UsbControlTransfer        sceUsbdControlTransfer
#define UsbIsochronousTransfer    sceUsbdIsochronousTransfer
#define UsbBulkTransfer           sceUsbdBulkTransfer
#define UsbInterruptTransfer      sceUsbdInterruptTransfer
#define UsbClearDeviceFeature     sceUsbdClearDeviceFeature
#define UsbSetDeviceFeature       sceUsbdSetDeviceFeature
#define UsbGetDeviceConfiguration sceUsbdGetConfiguration
#define UsbSetDeviceConfiguration sceUsbdSetConfiguration
#define UsbGetDeviceDescriptor    sceUsbdGetDescriptor
#define UsbSetDeviceDescriptor    sceUsbdSetDeviceDescriptor
#define UsbGetDeviceStatus        sceUsbdGetDeviceStatus
#define UsbSetDeviceAddress       sceUsbdSetAddress
#define UsbClearInterfaceFeature  sceUsbdClearInterfaceFeature
#define UsbSetInterfaceFeature    sceUsbdSetInterfaceFeature
#define UsbGetInterface           sceUsbdGetInterface
#define UsbSetInterface           sceUsbdSetInterface
#define UsbGetInterfaceDescriptor sceUsbdGetInterfaceDescriptor
#define UsbSetInterfaceDescriptor sceUsbdSetInterfaceDescriptor
#define UsbGetInterfaceStatus     sceUsbdGetInterfaceStatus
#define UsbClearEndpointFeature   sceUsbdClearEndpointFeature
#define UsbSetEndpointFeature     sceUsbdSetEndpointFeature
#define UsbGetEndpointStatus      sceUsbdGetEndpointStatus
#define UsbGetEndpointDescriptor  sceUsbdGetEndpointDescriptor
#define UsbSetEndpointDescriptor  sceUsbdSetEndpointDescriptor
#define UsbSynchEndpointFrame     sceUsbdSynchFrame

#endif /* __USBD_MACRO_H__ */
