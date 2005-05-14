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
# IOP Sub-CPU library manipulation functions
*/

#include "types.h"
#include "defs.h"
#include "irx.h"

#include "loadcore.h"
#include "ioman.h"
#include "stdio.h"
#include "sysclib.h"

#include "iopmgr.h"

/*! \brief Get pointer to system library structure for named library.
 *  \ingroup iopmgr 
 *
 *  \param name Stringname of library (eg "ioman").
 *  \return Pointer to IOP library structure.
 *
 * return values:
 *   0 if not found.
 *   pointer to library structure for registered library if found.
 */
iop_library_t *slib_get_lib_by_name(const char *name)
{
  iop_library_table_t *libtable;
  iop_library_t *libptr;
  int len = strlen(name) + 1;

  libtable = GetLibraryEntryTable();
  libptr = libtable->tail;
  while ((libptr != 0))
  {
    if (!memcmp(libptr->name, name, len))
	return libptr;
    libptr = libptr->prev;
  }
  return 0;
}

/*! \brief Get pointer to export list for named library.
 *  \ingroup iopmgr 
 *
 *  \param name Stringname of library (eg "ioman").
 *  \return Pointer to export list.
 *
 * return values:
 *   0 if not found.
 *   pointer to export list for registered library if found.
 */
void *slib_get_exportlist_by_name(const char *name)
{
  iop_library_t *libptr;

  libptr = slib_get_lib_by_name(name);
  if (libptr != 0)
    return libptr->exports;
  return 0;
}

/*! \brief Get version number for named library.
 *  \ingroup iopmgr 
 *
 *  \param name Stringname of library (eg "ioman").
 *  \return version number.
 *
 * return values:
 *   0 if not found.
 *   Version number for registered library if found. (0xXXYY, XX=HIGH, YY=LOW)
 */
int slib_get_version_by_name(const char *name)
{
  iop_library_t *libptr;

  libptr = slib_get_lib_by_name(name);
  if (libptr != 0)
    return (int)libptr->version;
  return 0;
}

/*! \brief Release (Unregister) a given named registered library.
 *  \ingroup iopmgr 
 *
 *  \param name Stringname of library (eg "ioman").
 *  \return Status of operation.
 *
 * return values:
 *  0 if found and sucessfully unregistered.
 *  -1 if found, but failed to unregister.
 *  -2 if library was not found.
 */
int slib_release_library(const char *name)
{
  struct irx_export_table *libptr;

  libptr = (struct irx_export_table *)slib_get_lib_by_name(name);
  if (libptr != 0)
    return ReleaseLibraryEntries(libptr);
  return -2;
}
