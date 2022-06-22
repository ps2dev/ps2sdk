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

#ifndef __USBD_H__
#define __USBD_H__

#include <types.h>
#include <irx.h>

typedef struct
{
    u8 requesttype;
    u8 request;
    u16 value;
    u16 index;
    u16 length;
} UsbDeviceRequest;

typedef void (*sceUsbdDoneCallback)(int result, int count, void *arg);

typedef struct
{
    u8 bLength;
    u8 bDescriptorType;
    u8 bNbrPorts;
    u8 wHubCharacteristicsLb;
    u8 wHubCharacteristicsHb;
    u8 bPwrOn2PwrGood;
    u8 bHubContrCurrent;
    u8 deviceRemovable[8]; // arbitrary number, depends on number of ports
} UsbHubDescriptor;


/** USB driver bus event listener structure */
typedef struct _UsbDriver
{
    struct _UsbDriver *next, *prev;
    /** short sweet name for your driver, like "usbmouse" or "pl2301" */
    char *name;
    int (*probe)(int devID);
    int (*connect)(int devID);
    int (*disconnect)(int devID);
    u32 reserved1;
    u32 reserved2;
    u32 reserved3;
    u32 reserved4;
    u32 reserved5;
    void *gp;
} sceUsbdLddOps;

typedef struct
{
    u8 bLength;
    u8 bDescriptorType;
    u16 bcdUSB;
    u8 bDeviceClass;
    u8 bDeviceSubClass;
    u8 bDeviceProtocol;
    u8 bMaxPacketSize0;
    u16 idVendor;
    u16 idProduct;
    u16 bcdDevice;
    u8 iManufacturer;
    u8 iProduct;
    u8 iSerialNumber;
    u8 bNumConfigurations;
} UsbDeviceDescriptor;

typedef struct
{
    u8 bLength;
    u8 bDescriptorType;
    // u8 wTotalLengthLb;
    // u8 wTotalLengthHb;
    u16 wTotalLength; // apparently we can expect this to be aligned, for some reason
    u8 bNumInterfaces;
    u8 bConfigurationValue;
    u8 iConfiguration;
    u8 bmAttributes;
    u8 maxPower;
} UsbConfigDescriptor;

typedef struct
{
    u8 bLength;
    u8 bDescriptorType;
    u8 bInterfaceNumber;
    u8 bAlternateSetting;
    u8 bNumEndpoints;
    u8 bInterfaceClass;
    u8 bInterfaceSubClass;
    u8 bInterfaceProtocol;
    u8 iInterface;
} UsbInterfaceDescriptor;

typedef struct
{
    u8 bLength;
    u8 bDescritorLength;
    u8 bEndpointAddress;
    u8 bmAttributes;
    u8 wMaxPacketSizeLB;
    u8 wMaxPacketSizeHB;
    u8 bInterval;
} UsbEndpointDescriptor;

typedef struct
{
    u8 bLength;
    u8 bDescriptorType;
    u16 wData[1];
} UsbStringDescriptor;

typedef struct
{
    u16 bLength  : 11;
    u16 reserved : 1;
    u16 PSW      : 4;
} sceUsbdIsochronousPswLen;

#define USB_MAX_ISOCH_PACKETS 8

typedef struct
{
    void *bBufStart;
    u32 bRelStartFrame;
    u32 bNumPackets;
    sceUsbdIsochronousPswLen Packets[USB_MAX_ISOCH_PACKETS];
} sceUsbdMultiIsochronousRequest;

typedef void (*sceUsbdMultiIsochronousDoneCallback)(int result, sceUsbdMultiIsochronousRequest *req, void *arg);

/*
 * Device and/or Interface Class codes
 */
#define USB_CLASS_PER_INTERFACE 0 /* for DeviceClass */
#define USB_CLASS_AUDIO         1
#define USB_CLASS_COMM          2
#define USB_CLASS_HID           3
#define USB_CLASS_PHYSICAL      5
#define USB_CLASS_PRINTER       7
#define USB_CLASS_MASS_STORAGE  8
#define USB_CLASS_HUB           9
#define USB_CLASS_DATA          10
#define USB_CLASS_APP_SPEC      0xfe
#define USB_CLASS_VENDOR_SPEC   0xff

/*
 * USB types
 */
#define USB_TYPE_STANDARD (0x00 << 5)
#define USB_TYPE_CLASS    (0x01 << 5)
#define USB_TYPE_VENDOR   (0x02 << 5)
#define USB_TYPE_RESERVED (0x03 << 5)

/*
 * USB recipients
 */
#define USB_RECIP_DEVICE    0x00
#define USB_RECIP_INTERFACE 0x01
#define USB_RECIP_ENDPOINT  0x02
#define USB_RECIP_OTHER     0x03

#define USB_ENDPOINT_XFER_CONTROL  0
#define USB_ENDPOINT_XFER_ISOC     1
#define USB_ENDPOINT_XFER_BULK     2
#define USB_ENDPOINT_XFER_INT      3
#define USB_ENDPOINT_XFERTYPE_MASK 3

#define USB_REQ_GET_REPORT   0x01
#define USB_REQ_GET_IDLE     0x02
#define USB_REQ_GET_PROTOCOL 0x03
#define USB_REQ_SET_REPORT   0x09
#define USB_REQ_SET_IDLE     0x0A
#define USB_REQ_SET_PROTOCOL 0x0B

/*
 * USB directions
 */
#define USB_DIR_OUT           0
#define USB_DIR_IN            0x80
#define USB_ENDPOINT_DIR_MASK 0x80

#define USB_DT_DEVICE    1
#define USB_DT_CONFIG    2
#define USB_DT_STRING    3
#define USB_DT_INTERFACE 4
#define USB_DT_ENDPOINT  5
#define USB_DT_HUB       0x29

#define USB_CLASS_HUB 9

#define USB_RT_HUB  (USB_TYPE_CLASS | USB_RECIP_DEVICE)
#define USB_RT_PORT (USB_TYPE_CLASS | USB_RECIP_OTHER)

#define USB_REQ_GET_STATUS        0x00
#define USB_REQ_CLEAR_FEATURE     0x01
#define USB_REQ_SET_FEATURE       0x03
#define USB_REQ_SET_ADDRESS       0x05
#define USB_REQ_GET_DESCRIPTOR    0x06
#define USB_REQ_SET_DESCRIPTOR    0x07
#define USB_REQ_GET_CONFIGURATION 0x08
#define USB_REQ_SET_CONFIGURATION 0x09
#define USB_REQ_GET_INTERFACE     0x0A
#define USB_REQ_SET_INTERFACE     0x0B
#define USB_REQ_SYNCH_FRAME       0x0C

/** No Error */
#define USB_RC_OK           0x000
/** Bad CRC */
#define USB_RC_CRC          0x001
/** Bit Stuffing */
#define USB_RC_BITSTUFF     0x002
/** Bad Direction Toggle */
#define USB_RC_TOGGLE       0x003
/** Endpoint Stalled */
#define USB_RC_STALL        0x004
/** Device Is Not Responding */
#define USB_RC_NORESPONSE   0x005
/** PID Check Failed */
#define USB_RC_BADPID       0x006
/** Unexpected PID */
#define USB_RC_WRONGPID     0x007
/** Data Overrun */
#define USB_RC_DATAOVER     0x008
/** Data Underrun */
#define USB_RC_DATAUNDER    0x009
/** Buffer Overrun */
#define USB_RC_BUFFOVER     0x00C
/** Buffer Underrun */
#define USB_RC_BUFFUNDER    0x00D
/** Not Accessed */
#define USB_RC_NOTACCESSED  0x00E
/** Not Accessed */
#define USB_RC_NOTACCESSED2 0x00F

/** Invalid device ID */
#define USB_RC_BADDEV      0x101
/** Invalid pipe ID */
#define USB_RC_BADPIPE     0x102
/** Invalid length */
#define USB_RC_BADLENGTH   0x103
/** Invalid driver */
#define USB_RC_BADDRIVER   0x104
/** Invalid context */
#define USB_RC_BADCONTEXT  0x105
#define USB_RC_BADALIGN    0x106
#define USB_RC_BADHUBDEPTH 0x107

/** No space for Endpoint Descriptor */
#define USB_RC_ED        0x111
/** No space for Input/Output Request */
#define USB_RC_IOREQ     0x112
/** Bad Option */
#define USB_RC_BADOPTION 0x113

/** Device or Bus Busy */
#define USB_RC_BUSY    0x121
/** Operation Aborted */
#define USB_RC_ABORTED 0x122

/** Unsupported Operation (not implemented) */
#define USB_RC_NOSUPPORT 0x131
/** Unknown Error (USBD.IRX doesn't know what went wrong) */
#define USB_RC_UNKNOWN   0x132

int sceUsbdRegisterLdd(sceUsbdLddOps *driver);
int sceUsbdUnregisterLdd(sceUsbdLddOps *driver);
void *sceUsbdScanStaticDescriptor(int devId, void *data, u8 type);
int sceUsbdGetDeviceLocation(int devId, u8 *path);
int sceUsbdSetPrivateData(int devId, void *data);
void *sceUsbdGetPrivateData(int devId);
int sceUsbdOpenPipe(int devId, UsbEndpointDescriptor *desc);
int sceUsbdOpenPipeAligned(int devId, UsbEndpointDescriptor *desc);
int sceUsbdClosePipe(int id);
int sceUsbdTransferPipe(int id, void *data, u32 len, void *option, sceUsbdDoneCallback callback, void *cbArg);

int sceUsbdRegisterAutoloader(sceUsbdLddOps *drv); // Arbitrarily named
int sceUsbdUnregisterAutoloader(void);             // Arbitrarily named
int sceUsbdChangeThreadPriority(int prio1, int prio2);

// these aren't implemented:
int sceUsbdGetReportDescriptor(int devId, int cfgNum, int ifNum, void **desc, u32 *len);
int sceUsbdMultiIsochronousTransfer(int pipeId, sceUsbdMultiIsochronousRequest *request, sceUsbdMultiIsochronousDoneCallback callback, void *cbArg);

// For backwards compatibility:
#define UsbCallbackProc                 sceUsbdDoneCallback
#define UsbDriver                       sceUsbdLddOps
#define UsbIsochronousPswLen            sceUsbdIsochronousPswLen
#define UsbMultiIsochronousRequest      sceUsbdMultiIsochronousRequest
#define UsbMultiIsochronousDoneCallback sceUsbdMultiIsochronousDoneCallback
#define UsbRegisterDriver               sceUsbdRegisterLdd
#define UsbUnregisterDriver             sceUsbdUnregisterLdd
#define UsbGetDeviceStaticDescriptor    sceUsbdScanStaticDescriptor
#define UsbGetDeviceLocation            sceUsbdGetDeviceLocation
#define UsbSetDevicePrivateData         sceUsbdSetPrivateData
#define UsbGetDevicePrivateData         sceUsbdGetPrivateData
#define UsbOpenEndpoint                 sceUsbdOpenPipe
#define UsbOpenEndpointAligned          sceUsbdOpenPipeAligned
#define UsbCloseEndpoint                sceUsbdClosePipe
#define UsbTransfer                     sceUsbdTransferPipe
#define UsbRegisterAutoloader           sceUsbdRegisterAutoloader
#define UsbUnregisterAutoloader         sceUsbdUnregisterAutoloader
#define UsbChangeThreadPriority         sceUsbdChangeThreadPriority
#define UsbGetReportDescriptor          sceUsbdGetReportDescriptor
#define UsbMultiIsochronousTransfer     sceUsbdMultiIsochronousTransfer

#define usbd_IMPORTS_start DECLARE_IMPORT_TABLE(usbd, 1, 1)
#define usbd_IMPORTS_end   END_IMPORT_TABLE

#define I_sceUsbdRegisterLdd              DECLARE_IMPORT(4, sceUsbdRegisterLdd)
#define I_sceUsbdUnregisterLdd            DECLARE_IMPORT(5, sceUsbdUnregisterLdd)
#define I_sceUsbdScanStaticDescriptor     DECLARE_IMPORT(6, sceUsbdScanStaticDescriptor)
#define I_sceUsbdSetPrivateData           DECLARE_IMPORT(7, sceUsbdSetPrivateData)
#define I_sceUsbdGetPrivateData           DECLARE_IMPORT(8, sceUsbdGetPrivateData)
#define I_sceUsbdOpenPipe                 DECLARE_IMPORT(9, sceUsbdOpenPipe)
#define I_sceUsbdClosePipe                DECLARE_IMPORT(10, sceUsbdClosePipe)
#define I_sceUsbdTransferPipe             DECLARE_IMPORT(11, sceUsbdTransferPipe)
#define I_sceUsbdOpenPipeAligned          DECLARE_IMPORT(12, sceUsbdOpenPipeAligned)
#define I_sceUsbdGetDeviceLocation        DECLARE_IMPORT(13, sceUsbdGetDeviceLocation)
#define I_sceUsbdRegisterAutoloader       DECLARE_IMPORT(14, sceUsbRegisterAutoloader)
#define I_sceUsbdUnregisterAutoloader     DECLARE_IMPORT(15, sceUsbUnregisterAutoloader)
#define I_sceUsbdChangeThreadPriority     DECLARE_IMPORT(16, sceUsbdChangeThreadPriority)
#define I_sceUsbdGetReportDescriptor      DECLARE_IMPORT(17, sceUsbdGetReportDescriptor)
#define I_sceUsbdMultiIsochronousTransfer DECLARE_IMPORT(18, sceUsbdMultiIsochronousTransfer)

// For backwards compatibility:
#define I_UsbCallbackProc                 I_sceUsbdDoneCallback
#define I_UsbDriver                       I_sceUsbdLddOps
#define I_UsbIsochronousPswLen            I_sceUsbdIsochronousPswLen
#define I_UsbMultiIsochronousRequest      I_sceUsbdMultiIsochronousRequest
#define I_UsbMultiIsochronousDoneCallback I_sceUsbdMultiIsochronousDoneCallback
#define I_UsbRegisterDriver               I_sceUsbdRegisterLdd
#define I_UsbUnregisterDriver             I_sceUsbdUnregisterLdd
#define I_UsbGetDeviceStaticDescriptor    I_sceUsbdScanStaticDescriptor
#define I_UsbGetDeviceLocation            I_sceUsbdGetDeviceLocation
#define I_UsbSetDevicePrivateData         I_sceUsbdSetPrivateData
#define I_UsbGetDevicePrivateData         I_sceUsbdGetPrivateData
#define I_UsbOpenEndpoint                 I_sceUsbdOpenPipe
#define I_UsbOpenEndpointAligned          I_sceUsbdOpenPipeAligned
#define I_UsbCloseEndpoint                I_sceUsbdClosePipe
#define I_UsbTransfer                     I_sceUsbdTransferPipe
#define I_UsbRegisterAutoloader           I_sceUsbdRegisterAutoloader
#define I_UsbUnregisterAutoloader         I_sceUsbdUnregisterAutoloader
#define I_UsbChangeThreadPriority         I_sceUsbdChangeThreadPriority
#define I_UsbGetReportDescriptor          I_sceUsbdGetReportDescriptor
#define I_UsbMultiIsochronousTransfer     I_sceUsbdMultiIsochronousTransfer

#endif /* __USBD_H__ */
