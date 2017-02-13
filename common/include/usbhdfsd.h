#ifndef _USBHDFSD_H
#define _USBHDFSD_H

//IOCTL function codes
#define USBMASS_IOCTL_RENAME 0x0000 //Rename opened file. Data input to ioctl() -> new, full filename of file.

//Device status bits.
#define USBMASS_DEV_STAT_CONN 0x01 //CONNected
#define USBMASS_DEV_STAT_CONF 0x02 //CONFigured
#define USBMASS_DEV_STAT_ERR 0x80  //ERRor

//Structure definitions
typedef struct UsbMassDeviceInfo
{
	unsigned short int status; // If the CONNected bit is not set, the contents of the other fields of this structure are undefined.
	unsigned short int SectorSize;
	unsigned int MaxLBA;
} UsbMassDeviceInfo_t;

enum USBMASS_DEV_EV {
	USBMASS_DEV_EV_CONN = 0,
	USBMASS_DEV_EV_DISCONN
};

typedef void (*usbmass_cb_t)(int cause);

//Exported functions
int UsbMassGetDeviceInfo(int device, UsbMassDeviceInfo_t *info);
int UsbMassRegisterCallback(int device, usbmass_cb_t callback);

#ifdef _IOP
#include <irx.h>

#define usbmass_IMPORTS_start DECLARE_IMPORT_TABLE(usbmass, 1, 1)
#define usbmass_IMPORTS_end END_IMPORT_TABLE

#define I_UsbMassGetDeviceInfo DECLARE_IMPORT(5, UsbMassGetDeviceInfo)
#define I_UsbMassRegisterCallback DECLARE_IMPORT(5, UsbMassRegisterCallback)
#endif

#endif //_USBHDFSD_H
