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
 * Common USBHDFSD definitions.
 */

#ifndef _USBHDFSD_COMMON_H
#define _USBHDFSD_COMMON_H

//IOCTL function codes
/** Rename opened file. Data input to ioctl() -> new, full filename of file. */
#define USBMASS_IOCTL_RENAME	0x0000

//Device status bits.
/** CONNected */
#define USBMASS_DEV_STAT_CONN	0x01
/** CONFigured */
#define USBMASS_DEV_STAT_CONF	0x02
/** ERRor */
#define USBMASS_DEV_STAT_ERR	0x80

#endif //_USBHDFSD_COMMON_H
