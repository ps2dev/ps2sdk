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
# init code for iop manager.
*/

#include "types.h"
#include "defs.h"
#include "irx.h"
#include "iomanX.h"
#include "stdio.h"

#include "iopmgr.h"

/** \defgroup iopmgr iopmgr - IOP Manager*/ 


IRX_ID(IOPMGR_MODNAME, IOPMGR_VERSION_HIGH, IOPMGR_VERSION_LOW);

struct irx_export_table _exp_iopmgr;

extern void cmdline_handle(char *command, char *arg1);

/*! \brief Entry point for IRX.
 *  \ingroup iopmgr 
 *
 *  if argc <= 1 , install as library.
 *  if argc > 1 , run given command in argv[1].
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
  printf("%s - Version %d.%d\nCopyright (c) 2003,2004 adresd\n\n",
    IOPMGR_MODNAME, IOPMGR_VERSION_HIGH, IOPMGR_VERSION_LOW);

  if (argc > 1)
  { /* Execute in command line mode, so parse command */
    if(argc>2)
    {
      cmdline_handle(argv[1],argv[2]);
    }
    else
    {
      cmdline_handle(argv[1],NULL);
    }
  }
  else
  { /* no params, so set into resident mode */
    if (RegisterLibraryEntries(&_exp_iopmgr) == 0)
    {
      printf("Registered.\n");
      return MODULE_RESIDENT_END;
    } 
    else 
    {
      /* luckily the previously installed one will be higher up the list, so will
       *  be the first one found , so get the version
       */
      int version = smod_get_modversion_by_name(IOPMGR_MODNAME);
      printf ("Error Registering - ");
      if (version >= 0)
        printf("Already Registered , version : %d.%d\n",
                (version&0xff00)>>8,version&0xff);
      else
        printf("Error : %d\n",version);
    }
  }
  printf("\nExiting.\n");
  return MODULE_NO_RESIDENT_END;
}

/*! \brief shutdown the IRX.
 *  \ingroup iopmgr 
 *
 *  This checks if the library is registered, if it is then
 *  it unregisters it.
 *
 *  \return Module Status on Exit.
 *
 *  return values:
 *    MODULE_RESIDENT_END if loaded and registered as library.
 *    MODULE_NO_RESIDENT_END if just exiting normally.
 */
int shutdown() 
{ 
  switch(slib_release_library(IOPMGR_LIBNAME))
  {
    case 0: printf("UnRegistered\n"); break;
    case -1: printf("Failed to UnRegister\n"); return MODULE_RESIDENT_END; break;
    case -2: printf("Library Not Present\n"); break;
  }
  return MODULE_NO_RESIDENT_END;
}

