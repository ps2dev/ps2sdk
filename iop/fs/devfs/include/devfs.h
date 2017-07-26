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
 * DevFS header file
 * @defgroup devfs devfs: PS2 Device Filing System
 * @addtogroup devfs
 * @{
 */

#ifndef __DEVFS_H__
#define __DEVFS_H__

#include <types.h>
#include <irx.h>

/** Type to hold a handle to a device */
typedef s32 HDEV;
/** Maximum number of sub devices per device */
#define DEVFS_MAX_SUBDEVS 16
/** Maximum size of a device name */
#define DEVFS_MAX_DEVNAME_LENGTH 32
/** Maximum length of a description string */
#define DEVFS_MAX_DESC_LENGTH 256

/** Enumeration of sub device access modes */
enum devfs_subdev_modes
{
    /** Indicates a sub device can only be opened by one application at a time */
    DEVFS_MODE_EX = (1 << 0),
    /** Indicates a sub device can be opened for reading */
    DEVFS_MODE_W  = (1 << 1),
    /** Indicates a sub device can be opened for writing */
    DEVFS_MODE_R  = (1 << 2),
    /** Indicates a sub device can be opened for reading and writing */
    DEVFS_MODE_RW = (DEVFS_MODE_R | DEVFS_MODE_W)
};

/** Enumeration of device type.
 * @note These are currently not used
 */
enum devfs_devtypes
{
    /** Indicates a device is character device */
    DEVFS_DEVTYPE_CHAR = 1,
    /** Indicates a device is a block device */
    DEVFS_DEVTYPE_BLOCK = 2,
    /** Indicates a device is a stream device (no seek) */
    DEVFS_DEVTYPE_STREAM = 3
};

/** Specifies ioctl was called. No return value and the lengths are invalid */
#define DEVFS_IOCTL_TYPE_1  1
/** ioctl2 was called. Parameters should all be valid */
#define DEVFS_IOCTL_TYPE_2  2

/** ioctl command to return the description associated with a device */
#define DEVFS_IOCTL_GETDESC 0

/** A union to make it easy to access the 32bit elements of a 64bit integer */
typedef union

{
  /** Array of two 32bit values */
  u32 loc32[2];
  /** The 64bit integer */
  u64 loc64;
} devfs_loc_t;

/** Structure passed to the application when an event occurs */
typedef struct

{
  /** The sub devices data pointer as set by DevFSAddSubDevice */
  void *data;
  /** The sub device number */
  s32 subdev;
  /** The open mode.
   * @note This is actually the posix mode bit fields as passed to open 
   */
  u32 mode;
  /** The current seek location */
  devfs_loc_t loc;
} devfs_info_t;

/** Typedef of the read event handler */
typedef s32 (*read_handler)(const devfs_info_t *dev, u8 *buf, s32 len);
/** Typedef of the write event handler */
typedef s32 (*write_handler)(const devfs_info_t *dev, u8 *buf, s32 len);
/** Typedef of the ioctl handler */
typedef s32 (*ioctl_handler)(const devfs_info_t *dev, int ioctl_type, int cmd, void *arg,
                            size_t arglen, void *buf, size_t buflen);

/** Structure defining a device node for passing to DevFSAddDevice() */
typedef struct
{
   /** Name of the device */
   char *name;
   /** A textual description */
   char *desc;
   /** The type of device, possible values in ::devfs_devtypes */
   s32  devtype;
   /** The block size of the device. Not currently used */
   u32  blocksize;
   /** Pointer to a read handler. Can be NULL */
   read_handler read;
   /** Pointer to a write handler. Can be NULL */
   write_handler write;
   /** Pointer to a ioctl handler. Can be NULL */
   ioctl_handler ioctl;
} devfs_node_t;

/** Adds a new device to the filing system
 * @param node: Pointer to a ::devfs_node_t structure
 * @returns A device handle is returned if the device was added.
 * On error INVALID_HDEV is returned
 */
HDEV DevFSAddDevice(const devfs_node_t *node);
/** Define to add DevFSAddDevice to the imports list */

/** Deletes an previously opened device.
 * @param hDev: Handle to the device to delete
 * @returns 0 if device deleted, -1 on error
 */
int DevFSDelDevice(HDEV hDev);

/** Adds a sub device to a previously opened device
 * @param hDev: Handle to an opened device
 * @param subdev_no: The number of the subdevice. Can be 0 to DEVFS_MAX_SUBDEVS
 * @param extent: A 64bit extent which reflects the size of the underlying device
 * @param data: Pointer to some private data to associate with this sub device
 * @returns 0 if sub device added, else -1
 */
int DevFSAddSubDevice(HDEV hDev, u32 subdev_no, s32 mode, devfs_loc_t extent, void *data);

/** Deletes a sub device.
 * @param hDev: Handle to an opened device.
 * @param subdev_no: The number of the subdevice to delete.
 * @returns 0 if device deleted. -1 on error.
 */
int DevFSDelSubDevice(HDEV hDev, u32 subdev_no);

/** Defines an invalid HDEV value */
#define INVALID_HDEV -1

#define devfs_IMPORTS_start DECLARE_IMPORT_TABLE(devfs, 1, 1)
#define devfs_IMPORTS_end END_IMPORT_TABLE

#define I_DevFSAddDevice DECLARE_IMPORT(4, DevFSAddDevice)
#define I_DevFSDelDevice DECLARE_IMPORT(5, DevFSDelDevice)
#define I_DevFSAddSubDevice DECLARE_IMPORT(6, DevFSAddSubDevice)
#define I_DevFSDelSubDevice DECLARE_IMPORT(7, DevFSDelSubDevice)

#endif /* __DEVFS_H__ */

/** @} */
