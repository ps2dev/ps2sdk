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

#ifndef __USBHDFSD_COMMON_H__
#define __USBHDFSD_COMMON_H__

#include <tamtypes.h>

typedef struct bd_fragment {
    u64 sector; /// start sector of fragmented bd/file
    u32 count;  /// number of sector in this fragment
} __attribute__((packed)) bd_fragment_t;

// IOCTL function codes
/** Rename opened file. Data input to ioctl() -> new, full filename of file. */
#define USBMASS_IOCTL_RENAME         0x0000
/** Returns first cluster number of opened file, within the filesystem. */
#define USBMASS_IOCTL_GET_CLUSTER    0x0001
/** Returns the absolute LBA of the opened file. */
#define USBMASS_IOCTL_GET_LBA        0x0002
/** Returns the block device driver name */
#define USBMASS_IOCTL_GET_DRIVERNAME 0x0003
/** Check if fragments exist */
#define USBMASS_IOCTL_CHECK_CHAIN    0x0004
/** Return fragment table **/
#define USBMASS_IOCTL_GET_FRAGLIST   0x0005
/** Get the device number for the block device backing the mass partition */
#define USBMASS_IOCTL_GET_DEVICE_NUMBER     0x0006

// DEVCTL function codes
/** Issues the SCSI STOP UNIT command to the specified device. Use this to shut down devices properly. */
#define USBMASS_DEVCTL_STOP_UNIT 0x0000
/** Issues the SCSI STOP UNIT command too all devices. Use this to shut down devices properly. */
#define USBMASS_DEVCTL_STOP_ALL  0x0001

// Device status bits.
/** CONNected */
#define USBMASS_DEV_STAT_CONN  0x01
/** CONFigured */
#define USBMASS_DEV_STAT_CONF  0x02
/** READY for I/O */
#define USBMASS_DEV_STAT_READY 0x04
/** ERRor */
#define USBMASS_DEV_STAT_ERR   0x80

// Device events
enum USBMASS_DEV_EV {
    USBMASS_DEV_EV_CONN = 0,
    USBMASS_DEV_EV_DISCONN
};

#endif /* __USBHDFSD_COMMON_H__ */
