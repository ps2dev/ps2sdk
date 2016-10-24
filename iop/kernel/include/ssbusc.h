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
# SSBUSC service control function definitions.
*/

#define ssbusc_IMPORTS_start DECLARE_IMPORT_TABLE(ssbusc, 1, 1)
#define ssbusc_IMPORTS_end END_IMPORT_TABLE

enum SSBUSC_DEV_IDS {
    SSBUSC_DEV0 = 0,    //Seems to affect the memory map entirely.
    SSBUSC_DEV_DVDROM,  //DVD ROM chip (contains rom1 and erom), DEV1
    SSBUSC_DEV2,
    SSBUSC_DEV3,
    SSBUSC_DEV_SPU,   //SPU, DEV4
    SSBUSC_DEV_CDVD,  //CD/DVD hardware, DEV5
    SSBUSC_DEV6,
    SSBUSC_DEV7,
    SSBUSC_DEV8,
    SSBUSC_DEV_SPU2,  //SPU2
    //These 3 seem to be controls for the DEV9 expansion interface. Only SSBUSC_DEV_DEV9_3 can have its address adjusted.
    SSBUSC_DEV_DEV9_2,
    SSBUSC_DEV_DEV9_3,
    SSBUSC_DEV_DEV9_1
};

//These functions control the timing (access timing?) and memory map address of the devices.
int sceSsbuscSetTiming(int device, unsigned int value);
#define I_sceSsbuscSetTiming DECLARE_IMPORT(4, sceSsbuscSetTiming)
int sceSsbuscGetTiming(int device);
#define I_sceSsbuscGetTiming DECLARE_IMPORT(5, sceSsbuscGetTiming)
int sceSsbuscSetAddress(int device, unsigned int value);
#define I_sceSsbuscSetAddress DECLARE_IMPORT(6, sceSsbuscSetAddress)
int sceSsbuscGetAddress(int device);
#define I_sceSsbuscGetAddress DECLARE_IMPORT(7, sceSsbuscGetAddress)

int sceSsbuscSetCOMDELAY1st(unsigned int value);
#define I_sceSsbuscSetCOMDELAY1st DECLARE_IMPORT(8, sceSsbuscSetCOMDELAY1st)
int sceSsbuscGetCOMDELAY1st(void);
#define I_sceSsbuscGetCOMDELAY1st DECLARE_IMPORT(9, sceSsbuscGetCOMDELAY1st)
int sceSsbuscSetCOMDELAY2nd(unsigned int value);
#define I_sceSsbuscSetCOMDELAY2nd DECLARE_IMPORT(10, sceSsbuscSetCOMDELAY2nd)
int sceSsbuscGetCOMDELAY2nd(void);
#define I_sceSsbuscGetCOMDELAY2nd DECLARE_IMPORT(11, sceSsbuscGetCOMDELAY2nd)
int sceSsbuscSetCOMDELAY3rd(unsigned int value);
#define I_sceSsbuscSetCOMDELAY3rd DECLARE_IMPORT(12, sceSsbuscSetCOMDELAY3rd)
int sceSsbuscGetCOMDELAY3rd(void);
#define I_sceSsbuscGetCOMDELAY3rd DECLARE_IMPORT(13, sceSsbuscGetCOMDELAY3rd)
int sceSsbuscSetCOMDELAY4th(unsigned int value);
#define I_sceSsbuscSetCOMDELAY4th DECLARE_IMPORT(14, sceSsbuscSetCOMDELAY4th)
int sceSsbuscGetCOMDELAY4th(void);
#define I_sceSsbuscGetCOMDELAY4th DECLARE_IMPORT(15, sceSsbuscGetCOMDELAY4th)
int sceSsbuscSetCOMDELAY(unsigned int value);
#define I_sceSsbuscSetCOMDELAY DECLARE_IMPORT(16, sceSsbuscSetCOMDELAY)
int sceSsbuscGetCOMDELAY(void);
#define I_sceSsbuscGetCOMDELAY DECLARE_IMPORT(17, sceSsbuscGetCOMDELAY)
