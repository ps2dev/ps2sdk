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
 * iop device handling and inquiry functions.
 */

#include "defs.h"
#include "stdio.h"
#include "sysclib.h"

#include "iomanX.h" /* just for fstypes */

#include "iopmgr.h"

/** Get a pointer to an ioman device handler.
 * @ingroup iopmgr
 *
 * @param device Stringname of device (eg "host").
 * @return Pointer to device structure.
 *
 * return values:
 *   0 if not found.
 *   pointer to device structure for named driver if found.
 */
iop_device_t *iopmgr_get_iomandev(char *device)
{
  ModuleInfo_t *info = 0;
  iop_device_t **devinfo_table;
  int i;
  int len = strlen(device) + 1;

  if (!(info = smod_get_mod_by_name(IOPMGR_IOMAN_IDENT))) {
    return 0;
  }

  /* Find the start of the device info array, in .bss.  */
  devinfo_table = (iop_device_t **)(info->text_start + info->text_size + info->data_size + 0x0c);

  /* The device info table had 16 entries, but some may be empty.  Just look at all of them.  */
  for (i = 0; i < 16; i++) {
    if (devinfo_table[i])
      if (!memcmp(devinfo_table[i]->name, device, len))
        return(devinfo_table[i]);
  }
  return 0;
}

/** Get a pointer to an iomanx device handler.
 * @ingroup iopmgr
 *
 * @param device Stringname of device (eg "hdd").
 * @return Pointer to device structure.
 *
 * return values:
 *   0 if not found.
 *   pointer to device structure for named driver if found.
 */
iop_device_t *iopmgr_get_iomanxdev(char *device)
{
  ModuleInfo_t *info;
  iop_device_t **devinfo_table;
  int i;
  int len = strlen(device) + 1;

  if (!(info = smod_get_mod_by_name(IOPMGR_IOMANX_IDENT))) {
    return 0;
  }

  /* Find the start of the device info array, in .bss.  */
  devinfo_table = (iop_device_t **)(info->text_start + info->text_size + info->data_size);

  /* The device info table had 32 entries, but some may be empty.  Just look at all of them.  */
  for (i = 0; i < 32; i++) {
    if (devinfo_table[i])
      if (!memcmp(devinfo_table[i]->name, device, len))
        return(devinfo_table[i]);
  }
  return 0;
}

/** Get a list of devices of a certain type
 * @ingroup iopmgr
 *
 * @param man Device manager mask (IOPMGR_DEVTYPE_IOMAN or IOPMGR_DEVTYPE_IOMANX).
 * @param devtype Device type mask (0x10 for filesystems).
 * @return Number of devices found
 *
 * return values:
 *   0 if no matching devices found
 *   device count, if found, and buffer buffer filled with ascii serial
 *   string of device names.
 */
int iopmgr_get_devicelist(int man,int devtype,char *buffer)
{
  ModuleInfo_t *info;
  iop_device_t **devinfo_table;
  int i;
  char *bufptr = buffer;
  int  count = 0;

  /* do the ioman devices */
  if ((man & IOPMGR_DEVTYPE_IOMAN))
  if ((info = smod_get_mod_by_name(IOPMGR_IOMAN_IDENT)))
  {
    /* Find the start of the device info array, in .bss.  */
    devinfo_table = (iop_device_t **)(info->text_start + info->text_size + info->data_size + 0x0c);

    /* The device info table had 16 entries, but some may be empty.  Just look at all of them.  */
    for (i = 0; i < 16; i++)
    {
      if (devinfo_table[i])
        if ((devinfo_table[i]->type & devtype))
        {
          strcpy(bufptr,devinfo_table[i]->name);
          bufptr += strlen(bufptr)+1;
          count++;
        }
    }
  }

  /* do the iomanx devices */
  if ((man & IOPMGR_DEVTYPE_IOMANX))
  if ((info = smod_get_mod_by_name(IOPMGR_IOMANX_IDENT)))
  {
    /* Find the start of the device info array, in .bss.  */
    devinfo_table = (iop_device_t **)(info->text_start + info->text_size + info->data_size);

    /* The device info table had 32 entries, but some may be empty.  Just look at all of them.  */
    for (i = 0; i < 32; i++)
    {
      if (devinfo_table[i])
        /* only add iomanx ones here, so must have extended flag set  else we get duplication with new iomanx */
        if ((devinfo_table[i]->type & IOP_DT_FSEXT) && (devinfo_table[i]->type & devtype))
        {
          strcpy(bufptr,devinfo_table[i]->name);
          bufptr += strlen(bufptr)+1;
          count++;
        }
    }
  }
  return count;
}

/** Get a pointer to a device of either type
 * @ingroup iopmgr
 *
 * @param device Stringname of device (eg "host).
 * @return Pointer to device structure.
 *
 * return values:
 *   0 if not found.
 *   pointer to device structure for named driver if found.
 */
iop_device_t *iopmgr_get_device(char *device)
{
  iop_device_t *devptr;
  devptr = iopmgr_get_iomandev(device);
  if (devptr > 0) return(devptr);
  devptr = iopmgr_get_iomanxdev(device);
  if (devptr > 0) return(devptr);
  return 0;
}

/** Returns which i/o manager handles the given device.
 * @ingroup iopmgr
 *
 * @param device Stringname of device (eg "host).
 * @return numeric value for i/o manager.
 *
 * return values:
 *   IOPMGR_DEVTYPE_INVALID if not found,
 *   IOPMGR_DEVTYPE_IOMAN if found and ioman device,
 *   IOPMGR_DEVTYPE_IOMANX if found and iomanx device.
 */
int iopmgr_get_devicetype(char *device)
{
  if (iopmgr_get_iomandev(device) > 0)
    return(IOPMGR_DEVTYPE_IOMAN);
  if (iopmgr_get_iomanxdev(device) > 0)
    return(IOPMGR_DEVTYPE_IOMANX);
  return IOPMGR_DEVTYPE_INVALID;
}
