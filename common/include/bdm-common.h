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
 * Common BDM (Block Device Manager) definitions.
 */

#ifndef __BDM_COMMON_H__
#define __BDM_COMMON_H__

// Upper 16bits = 'BD' for uniqueness
#define BDM_IOCTL_CODE(cmd)             (0x42440000 | cmd)

// Get the device index for the device backing the current partition
// Ex: usb0 = 0, usb1 = 1, ata master = 0, ata slave = 1, etc.
#define BDM_IOCTL_GET_DEVICE_INDEX      BDM_IOCTL_CODE(0x0001)

// Get the number of bits in a logical block address for the device
// Ex: LBA28 = 28, LBA32 = 32, LBA48 = 48, etc.
#define BDM_IOCTL_GET_LBA_BITS          BDM_IOCTL_CODE(0x0002)

#endif