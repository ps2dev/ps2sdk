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
 * Common CDVD DEVCTL and IOCTL2 command definitions
 */

#ifndef __CDVD_IOCTL_H__
#define __CDVD_IOCTL_H__

///////////////////////////////////////////////////////////////////////////////
//	CDVDMAN.IRX

//
// IOCTL2 commands
//
#define CIOCSTREAMPAUSE			0x630D
#define CIOCSTREAMRESUME		0x630E
#define CIOCSTREAMSTAT			0x630F

//
// DEVCTL commands
//
#define CDIOC_READCLOCK			0x430C
#define CDIOC_GETDISKTYP		0x431F
#define CDIOC_GETERROR			0x4320
#define CDIOC_TRAYREQ			0x4321
#define CDIOC_STATUS			0x4322
#define CDIOC_POWEROFF			0x4323
#define CDIOC_MMODE				0x4324
#define CDIOC_DISKRDY			0x4325
#define CDIOC_STREAMINIT		0x4327
#define CDIOC_BREAK				0x4328

#define CDIOC_SPINNOM			0x4380
#define CDIOC_SPINSTM			0x4381
#define CDIOC_TRYCNT			0x4382
#define CDIOC_STANDBY			0x4384
#define CDIOC_STOP				0x4385
#define CDIOC_PAUSE				0x4386
#define CDIOC_GETTOC			0x4387
#define CDIOC_SETTIMEOUT		0x4388
#define CDIOC_READDVDDUALINFO	0x4389
#define CDIOC_INIT				0x438A
#define CDIOC_FSCACHEINIT		0x4395
#define CDIOC_FSCACHEDELETE		0x4397

#endif /* __CDVD_IOCTL_H__ */
