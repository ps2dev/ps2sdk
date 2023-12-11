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
 * device driver scanner and handler
 */

#include <types.h>
#include <defs.h>
#include <irx.h>

#include <loadcore.h>
#include <ioman_mod.h>
#include <iomanX.h>
#include <thbase.h>
#include <thsemap.h>
#include <stdio.h>
#include <sysclib.h>
#include <iopmgr.h>

#include "devscan.h"
#include "debug_printf.h"

/** Device type structure.
 * @ingroup ps2netfs
 */
static dev_table_t dev_info_list[DEVSCAN_MAX+1]; /* one for padding */

/** Get pointer to module structure for named module.
 * @ingroup ps2netfs
 *
 * @param name Stringname of module (eg "atad_driver").
 * @return Pointer to module structure.
 *
 * return values:
 *   0 if not found.
 *   pointer to module structure for loaded module if found.
 */
ModuleInfo_t *devscan_getmodule(const char *name)
{
  ModuleInfo_t *modptr;
  int len = strlen(name)+1;

  modptr = GetLoadcoreInternalData()->image_info;
  while (modptr != 0)
  {
    if (!memcmp(modptr->name, name, len))
      return modptr;
    modptr = modptr->next;
  }
  return 0;
}

/** Initialise the devices table
 * @ingroup ps2netfs
 *
 * @param devtype Mask for device types to return (0x10 = FS drivers).
 * @return Number of devices found.
 *
 * This scans ioman and iomanx for devices, then adds them to the
 * internal list, with basename and the handler type.
 *
 * Also used to re-init the devices table
 */
int devscan_setup(int devtype)
{
  ModuleInfo_t *info;
  iop_device_t **devinfo_table;
  int i;
  int  count = 0;

  DPRINTF("%s\n", __func__);
  // clear device list
  memset(&dev_info_list,0,sizeof(dev_info_list));

  /* do the ioman devices */
  if ((info = devscan_getmodule(IOPMGR_IOMAN_IDENT)))
  {
    /* Find the start of the device info array, in .bss.  */
    devinfo_table = (iop_device_t **)(info->text_start + info->text_size + info->data_size + 0x0c);

    /* The device info table had 16 entries, but some may be empty.  Just look at all of them.  */
    for (i = 0; i < DEVSCAN_IOMAX; i++)
    {
      if (devinfo_table[i])
        if ((devinfo_table[i]->type & devtype))
        {
          dev_info_list[count].devtype = IOPMGR_DEVTYPE_IOMAN;
          strncpy(dev_info_list[count].name,devinfo_table[i]->name,255);
          dev_info_list[count].name[255] = '\0';
          dev_info_list[count].len = strlen(dev_info_list[count].name);
          DPRINTF("ioman '%s'\n",dev_info_list[count].name);
          count++;
        }
    }
  }

  /* do the iomanx devices */
  if ((info = devscan_getmodule(IOPMGR_IOMANX_IDENT)))
  {
    /* Find the start of the device info array, in .bss.  */
    devinfo_table = (iop_device_t **)(info->text_start + info->text_size + info->data_size);

    /* The device info table had 32 entries, but some may be empty.  Just look at all of them.  */
    for (i = 0; i < DEVSCAN_IOXMAX; i++)
    {
      if (devinfo_table[i])
        /* only add iomanx ones here, so must have extended flag set  else we get duplication with new iomanx */
        if ((devinfo_table[i]->type & IOP_DT_FSEXT) && (devinfo_table[i]->type & devtype))
        {
          dev_info_list[count].devtype = IOPMGR_DEVTYPE_IOMANX;
          strncpy(dev_info_list[count].name,devinfo_table[i]->name,255);
          dev_info_list[count].name[255] = '\0';
          dev_info_list[count].len = strlen(dev_info_list[count].name);
          DPRINTF("iomanx '%s'\n",dev_info_list[count].name);
          count++;
        }
    }
  }
  return count;
}

/** Get device handler type for path.
 * @ingroup ps2netfs
 *
 * @param name Full pathname to check.
 * @return Device handler type.
 *
 * return values:
 *   MODULE_RESIDENT_END if loaded and registered as library.
 *   MODULE_NO_RESIDENT_END if just exiting normally.
 */
int devscan_gettype(char *name)
{
  int count = 0;
  DPRINTF("gettype '%s'\n",name);
  while (dev_info_list[count].name[0] != 0)
  {
    int ret = strncmp(dev_info_list[count].name,name,dev_info_list[count].len );
    DPRINTF("'%s'",dev_info_list[count].name);
    if (!ret)
      return dev_info_list[count].devtype;
    count++;
  }
  return IOPMGR_DEVTYPE_INVALID;
}

/** Get device list.
 * @ingroup ps2netfs
 *
 * @param  buffer    Pointer to dest buffer
 * @return number of devices returned.
 */
int devscan_getdevlist(char *buffer)
{
  int count;
  int i;
  char *bufptr = buffer;

  DPRINTF("getdevlist\n");
  /* rescan for devices, before returning list */
  count = devscan_setup(DEVSCAN_MASK);

  /* now convert this list into the right format */
  for (i=0;i<count;i++)
  {
    strcpy(bufptr,dev_info_list[i].name);
    DPRINTF("'%s'\n",bufptr);
    bufptr += strlen(bufptr)+1;
  }
  return count;
}

