/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _USBMLOAD_H
#define _USBMLOAD_H

typedef struct _USBDEV_t
{
	struct _USBDEV_t *forw;
	char *dispname;
	int vendor;
	int product;
	int release;
	int class_;
	int subclass;
	int protocol;
	char *category;
	char *path;
	char *argv[8];
	int argc;
	char activate_flag;
	int modid;
	char modname[56];
	int load_result;
} USBDEV_t;

typedef USBDEV_t *(*sceUsbmlPopDevinfo)(void);

typedef void (*sceUsbmlLoadFunc)(sceUsbmlPopDevinfo pop_devinfo);

extern int sceUsbmlDisable(void);
extern int sceUsbmlEnable(void);
extern int sceUsbmlActivateCategory(const char *category);
extern int sceUsbmlInactivateCategory(const char *category);
extern int sceUsbmlRegisterLoadFunc(sceUsbmlLoadFunc loadfunc);
extern void sceUsbmlUnregisterLoadFunc(void);
extern int sceUsbmlLoadConffile(const char *conffile);
extern int sceUsbmlRegisterDevice(USBDEV_t *device);
extern int sceUsbmlChangeThreadPriority(int prio1);

#define usbmload_IMPORTS_start DECLARE_IMPORT_TABLE(usbmload, 1, 3)
#define usbmload_IMPORTS_end END_IMPORT_TABLE

#define I_sceUsbmlDisable DECLARE_IMPORT(4, sceUsbmlDisable)
#define I_sceUsbmlEnable DECLARE_IMPORT(5, sceUsbmlEnable)
#define I_sceUsbmlActivateCategory DECLARE_IMPORT(6, sceUsbmlActivateCategory)
#define I_sceUsbmlInactivateCategory DECLARE_IMPORT(7, sceUsbmlInactivateCategory)
#define I_sceUsbmlRegisterLoadFunc DECLARE_IMPORT(8, sceUsbmlRegisterLoadFunc)
#define I_sceUsbmlUnregisterLoadFunc DECLARE_IMPORT(9, sceUsbmlUnregisterLoadFunc)
#define I_sceUsbmlLoadConffile DECLARE_IMPORT(10, sceUsbmlLoadConffile)
#define I_sceUsbmlRegisterDevice DECLARE_IMPORT(11, sceUsbmlRegisterDevice)
#define I_sceUsbmlChangeThreadPriority DECLARE_IMPORT(12, sceUsbmlChangeThreadPriority)

#endif
