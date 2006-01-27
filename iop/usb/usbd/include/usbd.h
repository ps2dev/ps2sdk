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
#ifndef __USBD_H__
#define __USBD_H__

#include "irx.h"

#define usbd_IMPORTS_start DECLARE_IMPORT_TABLE(usbd,1,1)
#define usbd_IMPORTS_end END_IMPORT_TABLE

typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef signed int int32;
typedef signed short int16;
typedef signed char int8;

typedef struct {
	uint8  requesttype;
	uint8  request;
	uint16 value;
	uint16 index;
	uint16 length;
} UsbDeviceRequest __attribute__ ((packed));

typedef void (*UsbCallbackProc)(int result, int count, void *arg);

typedef struct {
	uint8 bLength;
	uint8 bDescriptorType;
	uint8 bNbrPorts;
	uint8 wHubCharacteristicsLb;
	uint8 wHubCharacteristicsHb;
	uint8 bPwrOn2PwrGood;
	uint8 bHubContrCurrent;
	uint8 deviceRemovable[8]; // arbitrary number, depends on number of ports
} UsbHubDescriptor __attribute__ ((packed));


/* USB driver bus event listener structure */
typedef struct _UsbDriver {
	struct _UsbDriver *next, *prev;

	/* short sweet name for your driver, like "usbmouse" or "pl2301" */
	char *name;

	int (*probe)(int devID);

	int (*connect)(int devID);

	int (*disconnect)(int devID);
	uint32 reserved1;
	uint32 reserved2;
	uint32 reserved3;
	uint32 reserved4;
	uint32 reserved5;
	void *gp;
} UsbDriver __attribute__ ((packed));

typedef struct {
	uint8 bLength;
	uint8 bDescriptorType;
	uint16 bcdUSB;
	uint8 bDeviceClass;
	uint8 bDeviceSubClass;
	uint8 bDeviceProtocol;
	uint8 bMaxPacketSize0;
	uint16 idVendor;
	uint16 idProduct;
	uint16 bcdDevice;
	uint8 iManufacturer;
	uint8 iProduct;
	uint8 iSerialNumber;
	uint8 bNumConfigurations;
} UsbDeviceDescriptor __attribute__ ((packed));

typedef struct {
	uint8 bLength;
	uint8 bDescriptorType;
	//uint8 wTotalLengthLb;
	//uint8 wTotalLengthHb;
	uint16 wTotalLength; // apparently we can expect this to be aligned, for some reason
	uint8 bNumInterfaces;
	uint8 bConfigurationValue;
	uint8 iConfiguration;
	uint8 bmAttributes;
	uint8 maxPower;
} UsbConfigDescriptor __attribute__ ((packed));

typedef struct {
	uint8 bLength;
	uint8 bDescriptorType;
	uint8 bInterfaceNumber;
	uint8 bAlternateSetting;
	uint8 bNumEndpoints;
	uint8 bInterfaceClass;
	uint8 bInterfaceSubClass;
	uint8 bInterfaceProtocol;
	uint8 iInterface;
} UsbInterfaceDescriptor __attribute__ ((packed));

typedef struct {
	uint8 bLength;
	uint8 bDescritorLength;
	uint8 bEndpointAddress;
	uint8 bmAttributes;
	uint8 wMaxPacketSizeLB;
	uint8 wMaxPacketSizeHB;
	uint8 bInterval;
} UsbEndpointDescriptor __attribute__ ((packed));

typedef struct {
	uint8  bLength;
	uint8  bDescriptorType;
	uint16 wData[1];
} UsbStringDescriptor __attribute__ ((packed));

/*
 * Device and/or Interface Class codes
 */
#define USB_CLASS_PER_INTERFACE		0	/* for DeviceClass */
#define USB_CLASS_AUDIO				1
#define USB_CLASS_COMM				2
#define USB_CLASS_HID				3
#define USB_CLASS_PHYSICAL			5
#define USB_CLASS_PRINTER			7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB				9
#define USB_CLASS_DATA				10
#define USB_CLASS_APP_SPEC			0xfe
#define USB_CLASS_VENDOR_SPEC		0xff

/*
 * USB types
 */
#define USB_TYPE_STANDARD		(0x00 << 5)
#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)
#define USB_TYPE_RESERVED		(0x03 << 5)

/*
 * USB recipients
 */
#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#define USB_RECIP_OTHER			0x03

#define USB_ENDPOINT_XFER_CONTROL		0
#define USB_ENDPOINT_XFER_ISOC			1
#define USB_ENDPOINT_XFER_BULK			2
#define USB_ENDPOINT_XFER_INT			3
#define USB_ENDPOINT_XFERTYPE_MASK		3

#define USB_REQ_GET_REPORT		0x01
#define USB_REQ_GET_IDLE		0x02
#define USB_REQ_GET_PROTOCOL		0x03
#define USB_REQ_SET_REPORT		0x09
#define USB_REQ_SET_IDLE		0x0A
#define USB_REQ_SET_PROTOCOL		0x0B

/*
 * USB directions
 */
#define USB_DIR_OUT					0
#define USB_DIR_IN					0x80
#define USB_ENDPOINT_DIR_MASK		0x80

#define USB_DT_DEVICE		   1
#define USB_DT_CONFIG		   2
#define USB_DT_STRING		   3
#define USB_DT_INTERFACE	   4
#define USB_DT_ENDPOINT		   5
#define USB_DT_HUB			0x29

#define USB_CLASS_HUB		9

#define USB_RT_HUB		(USB_TYPE_CLASS | USB_RECIP_DEVICE)
#define USB_RT_PORT		(USB_TYPE_CLASS | USB_RECIP_OTHER)

#define USB_REQ_GET_STATUS			0x00
#define USB_REQ_CLEAR_FEATURE		0x01
#define USB_REQ_SET_FEATURE			0x03
#define USB_REQ_SET_ADDRESS			0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0A
#define USB_REQ_SET_INTERFACE		0x0B
#define USB_REQ_SYNCH_FRAME			0x0C

#define USB_RC_OK			0x000	// No Error
#define USB_RC_CRC			0x001	// Bad CRC
#define USB_RC_BITSTUFF		0x002	// Bit Stuffing
#define USB_RC_TOGGLE		0x003	// Bad Direction Toggle
#define USB_RC_STALL		0x004	// Endpoint Stalled
#define USB_RC_NORESPONSE	0x005	// Device Is Not Responding
#define USB_RC_BADPID		0x006	// PID Check Failed
#define USB_RC_WRONGPID		0x007	// Unexpected PID 
#define USB_RC_DATAOVER		0x008	// Data Overrun
#define USB_RC_DATAUNDER	0x009	// Data Underrun
#define USB_RC_BUFFOVER		0x00C	// Buffer Overrun
#define USB_RC_BUFFUNDER	0x00D	// Buffer Underrun
#define USB_RC_NOTACCESSED	0x00E	// Not Accessed
#define USB_RC_NOTACCESSED2	0x00F	// Not Accessed

#define USB_RC_BADDEV		0x101	// Invalid device ID
#define USB_RC_BADPIPE		0x102	// Invalid pipe ID
#define USB_RC_BADLENGTH	0x103	// Invalid length
#define USB_RC_BADDRIVER	0x104	// Invalid driver
#define USB_RC_BADCONTEXT	0x105	// Invalid context
#define USB_RC_BADALIGN		0x106
#define USB_RC_BADHUBDEPTH	0x107

//#define USB_RC_ED			0x111	// No space for Endpoint Descriptor
#define USB_RC_IOREQ		0x112	// No space for Input/Output Request
#define USB_RC_BADOPTION	0x113	// Bad Option

#define USB_RC_BUSY			0x121	// Device or Bus Busy
#define USB_RC_ABORTED		0x122	// Operation Aborted

//#define USB_RC_NOSUPPORT	0x131	// Unsupported Operation (not implemented)
//#define USB_RC_UNKNOWN		0x132	// Unknown Error (USBD.IRX doesn't know what went wrong)

int UsbRegisterDriver(UsbDriver *driver);
int UsbUnregisterDriver(UsbDriver *driver);
void *UsbGetDeviceStaticDescriptor(int devId, void *data, uint8 type);
int UsbGetDeviceLocation(int devId, uint8 *path);
int UsbSetDevicePrivateData(int devId, void *data);
void *UsbGetDevicePrivateData(int devId);
int UsbOpenEndpoint(int devId, UsbEndpointDescriptor *desc);
int UsbCloseEndpoint(int id);
int UsbTransfer(int id, void *data, uint32 len, void *option, UsbCallbackProc callback, void *cbArg);
int UsbOpenEndpointAligned(int devId, UsbEndpointDescriptor *desc);

// these aren't implemented:
int UsbRegisterAutoloader(UsbDriver *drv);
int UsbUnregisterAutoloader(UsbDriver *drv);
int UsbChangeThreadPriority(void);


#define I_UsbRegisterDriver DECLARE_IMPORT(4,UsbRegisterDriver)
#define I_UsbUnregisterDriver DECLARE_IMPORT(5,UsbUnregisterDriver)
#define I_UsbGetDeviceStaticDescriptor DECLARE_IMPORT(6,UsbGetDeviceStaticDescriptor)
#define I_UsbSetDevicePrivateData DECLARE_IMPORT(7,UsbSetDevicePrivateData)
#define I_UsbGetDevicePrivateData DECLARE_IMPORT(8,UsbGetDevicePrivateData)
#define I_UsbOpenEndpoint DECLARE_IMPORT(9, UsbOpenEndpoint)
#define I_UsbCloseEndpoint DECLARE_IMPORT(10, UsbCloseEndpoint)
#define I_UsbTransfer DECLARE_IMPORT(11,UsbTransfer)
#define I_UsbOpenEndpointAligned DECLARE_IMPORT(12,UsbOpenEndpointAligned)

#endif // __USBD_H__


