/*
 * ps2netfs.c - iop tcp fs driver.
 *
 * Copyright (c) 2004 adresd <adresd_ps2dev@yahoo.com>
 *
 * Licensed under the AFL v2.0. See the file LICENSE included with this
 * distribution for licensing terms.
 */

#include <types.h>
#include <defs.h>
#include <irx.h>

#include <loadcore.h>
#include <ioman.h>
#include <stdio.h>
#include <iopmgr.h>

#include "ps2_fio.h"
#include "devscan.h"

#define PS2NETFS_MODNAME "PS2_TcpFileDriver"
#define PS2NETFS_VERSION_HIGH 1
#define PS2NETFS_VERSION_LOW  0

IRX_ID(PS2NETFS_MODNAME, PS2NETFS_VERSION_HIGH, PS2NETFS_VERSION_LOW);

/*! \brief Entry point for IRX.
 *  \ingroup ps2netfs 
 *
 *  argc and argv make no difference to runstate.
 *
 *  \param argc Number of arguments.
 *  \param argv Pointer to array of arguments.
 *  \return Module Status on Exit.
 *
 *  return values:
 *    MODULE_RESIDENT_END if loaded and registered as library.
 *    MODULE_NO_RESIDENT_END if just exiting normally.
 */
int _start(int argc, char *argv[])
{
  printf("%s - v%d.%d - Copyright (c) 2004 adresd\n",
    PS2NETFS_MODNAME,PS2NETFS_VERSION_HIGH,PS2NETFS_VERSION_LOW);

  if (!devscan_getmodule(IOPMGR_IOMAN_IDENT))
  {
    printf("ioman not found\n");
    return MODULE_NO_RESIDENT_END;
  }
  if (!devscan_getmodule(IOPMGR_IOMANX_IDENT))
  {
    printf("iomanx not found\n");
    return MODULE_NO_RESIDENT_END;
  }

  if (ps2netfs_Init() == 0)
  {
    printf("\nServer Started\n");
    return MODULE_RESIDENT_END;
  }

  printf("\nExiting.\n");
  return MODULE_NO_RESIDENT_END;
}
