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
# DevFS header file
*/

/** @defgroup devfs devfs: PS2 Device Filing System */

#ifndef __DEVFS_H__
#define __DEVFS_H__

/** Declares the start of devfs import table 
    @ingroup devfs
*/
#define devfs_IMPORTS_start DECLARE_IMPORT_TABLE(devfs, 1, 1)
/** Declares the end of devfs import table 
    @ingroup devfs
*/
#define devfs_IMPORTS_end END_IMPORT_TABLE

/** Type to hold a handle to a device 
    @ingroup devfs
*/
typedef s32 HDEV;
/** Maximum number of sub devices per device 
    @ingroup devfs
*/
#define DEVFS_MAX_SUBDEVS 16
/** Maximum size of a device name 
    @ingroup devfs
*/
#define DEVFS_MAX_DEVNAME_LENGTH 32
/** Maximum length of a description string 
    @ingroup devfs
*/
#define DEVFS_MAX_DESC_LENGTH 256

/** Enumeration of sub device access modes */
enum devfs_subdev_modes
{
    /** Indicates a sub device can only be opened by one application at a time 
        @ingroup devfs
    */
    DEVFS_MODE_EX = (1 << 0),
    /** Indicates a sub device can be opened for reading 
        @ingroup devfs
    */
    DEVFS_MODE_W  = (1 << 1),
    /** Indicates a sub device can be opened for writing 
        @ingroup devfs 
    */
    DEVFS_MODE_R  = (1 << 2),
    /** Indicates a sub device can be opened for reading and writing 
        @ingroup devfs
    */
    DEVFS_MODE_RW = (DEVFS_MODE_R | DEVFS_MODE_W)
};

/** Enumeration of device type. 
    @note These are currently not used 
    @ingroup devfs
*/
enum devfs_devtypes
{
    /** Indicates a device is character device 
        @ingroup devfs
    */
    DEVFS_DEVTYPE_CHAR = 1,
    /** Indicates a device is a block device 
        @ingroup devfs
    */
    DEVFS_DEVTYPE_BLOCK = 2,
    /** Indicates a device is a stream device (no seek) 
        @ingroup devfs
    */
    DEVFS_DEVTYPE_STREAM = 3
};

/** Specifies ioctl was called. No return value and the lengths are invalid 
    @ingroup devfs
*/
#define DEVFS_IOCTL_TYPE_1  1
/** ioctl2 was called. Parameters should all be valid  
    @ingroup devfs
*/
#define DEVFS_IOCTL_TYPE_2  2

/** ioctl command to return the description associated with a device 
    @ingroup devfs
*/ 
#define DEVFS_IOCTL_GETDESC 0

/** A union to make it easy to access the 32bit elements of a 64bit integer 
    @ingroup devfs
*/
typedef union 

{
  /** Array of two 32bit values 
      @ingroup devfs
  */
  u32 loc32[2];
  /** The 64bit integer 
      @ingroup devfs
  */
  u64 loc64;
} devfs_loc_t;

/** Structure passed to the application when an event occurs 
    @ingroup devfs
*/
typedef struct

{
  /** The sub devices data pointer as set by DevFSAddSubDevice 
      @ingroup devfs
   */
  void *data;
  /** The sub device number 
      @ingroup devfs
  */
  s32 subdev;
  /** The open mode.
      @note This is actually the posix mode bit fields as passed to open
      @ingroup devfs
  */
  u32 mode;
  /** The current seek location 
      @ingroup devfs
  */
  devfs_loc_t loc;
} devfs_info_t;

/** Typedef of the read event handler 
    @ingroup devfs
 */
typedef s32 (*read_handler)(const devfs_info_t *dev, u8 *buf, s32 len);
/** Typedef of the write event handler 
    @ingroup devfs*/
typedef s32 (*write_handler)(const devfs_info_t *dev, u8 *buf, s32 len);
/** Typedef of the ioctl handler 
    @ingroup devfs
*/
typedef s32 (*ioctl_handler)(const devfs_info_t *dev, int ioctl_type, int cmd, void *arg, 
                            size_t arglen, void *buf, size_t buflen);

/** Structure defining a device node for passing to DevFSAddDevice() 
    @ingroup devfs
*/
typedef struct
{
   /** Name of the device 
       @ingroup devfs
   */
   char *name;
   /** A textual description 
       @ingroup devfs
   */
   char *desc;
   /** The type of device, possible values in ::devfs_devtypes 
       @ingroup devfs
   */
   s32  devtype;
   /** The block size of the device. Not currently used 
       @ingroup devfs
   */
   u32  blocksize;
   /** Pointer to a read handler. Can be NULL 
       @ingroup devfs
   */
   read_handler read;
   /** Pointer to a write handler. Can be NULL 
       @ingroup devfs
   */
   write_handler write;
   /** Pointer to a ioctl handler. Can be NULL 
       @ingroup devfs
   */
   ioctl_handler ioctl;
} devfs_node_t;

/** Adds a new device to the filing system
    @param node: Pointer to a ::devfs_node_t structure
    @returns A device handle is returned if the device was added. 
    On error INVALID_HDEV is returned
    @ingroup devfs
*/
HDEV DevFSAddDevice(const devfs_node_t *node);
/** Define to add DevFSAddDevice to the imports list 
    @ingroup devfs
*/
#define I_DevFSAddDevice DECLARE_IMPORT(4, DevFSAddDevice)

/** Deletes an previously opened device.
    @param hDev: Handle to the device to delete
    @returns 0 if device deleted, -1 on error
    @ingroup devfs
*/
int DevFSDelDevice(HDEV hDev);
/** Define to add DevFSDelDevice to the imports list 
    @ingroup devfs
*/
#define I_DevFSDelDevice DECLARE_IMPORT(5, DevFSDelDevice)

/** Adds a sub device to a previously opened device
    @param hDev: Handle to an opened device
    @param subdev_no: The number of the subdevice. Can be 0 to DEVFS_MAX_SUBDEVS
    @param extent: A 64bit extent which reflects the size of the underlying device 
    @param data: Pointer to some private data to associate with this sub device
    @returns 0 if sub device added, else -1
    @ingroup devfs
*/
int DevFSAddSubDevice(HDEV hDev, u32 subdev_no, s32 mode, devfs_loc_t extent, void *data);
/** Define to add DevFSAddSubDevice to the imports list 
    @ingroup devfs
*/
#define I_DevFSAddSubDevice DECLARE_IMPORT(6, DevFSAddSubDevice)

/** Deletes a sub device.
    @param hDev: Handle to an opened device.
    @param subdev_no: The number of the subdevice to delete.
    @returns 0 if device deleted. -1 on error.
    @ingroup devfs
*/
int DevFSDelSubDevice(HDEV hDev, u32 subdev_no);
/** Define to add DevFSDelSubDevice to the imports list 
    @ingroup devfs
*/
#define I_DevFSDelSubDevice DECLARE_IMPORT(7, DevFSDelSubDevice)

/** Defines an invalid HDEV value 
    @ingroup devfs
*/
#define INVALID_HDEV -1

#endif
