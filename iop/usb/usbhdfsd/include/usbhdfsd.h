/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * IOP USBHDFSD definitions.
 */

#include <irx.h>
#include "usbhdfsd-common.h"

#ifndef _USBHDFSD_IOP_H
#define _USBHDFSD_IOP_H

//Structure definitions
typedef struct UsbMassDeviceInfo{
	/** If the CONNected bit is not set, the contents of the other fields of this structure are undefined. */
	unsigned short int status;
	unsigned short int SectorSize;
	unsigned int MaxLBA;
} UsbMassDeviceInfo_t;

enum USBMASS_DEV_EV{
	USBMASS_DEV_EV_CONN	= 0,
	USBMASS_DEV_EV_DISCONN
};

typedef void (*usbmass_cb_t)(int cause);

//Exported functions
int UsbMassGetDeviceInfo(int device, UsbMassDeviceInfo_t *info);
int UsbMassRegisterCallback(int device, usbmass_cb_t callback);

#define usbmass_IMPORTS_start DECLARE_IMPORT_TABLE(usbmass, 1, 1)
#define usbmass_IMPORTS_end END_IMPORT_TABLE

#define I_UsbMassGetDeviceInfo DECLARE_IMPORT(5, UsbMassGetDeviceInfo)
#define I_UsbMassRegisterCallback DECLARE_IMPORT(5, UsbMassRegisterCallback)

#endif //_USBHDFSD_IOP_H
