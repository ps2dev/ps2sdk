/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003       Marcus R. Brown <mrbrown@0xd6.org>
# Copyright (c) 2003, 2004 adresd   <adresd_ps2dev@yahoo.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# iop Sub-CPU module library
*/

#include "types.h"
#include "defs.h"
#include "irx.h"

#include "loadcore.h"
#include "ioman.h"
#include "stdio.h"
#include "sysclib.h"

#include "iopmgr.h"

/*! \brief Get pointer to head of module list, or next module in list.
 *  \ingroup iopmgr 
 *
 *  \param cur_mod Pointer to module structure, or 0 to return the head.
 *  \return Pointer to module structure.
 *
 * return values:
 *   0 if end of list.
 *   pointer to head of module list if cur_mod=0.
 *   pointer to next module, if cur_mod!=0.
 */
smod_mod_info_t *smod_get_next_mod(smod_mod_info_t *cur_mod)
{
  /* If cur_mod is 0, return the head of the list (IOP address 0x800).  */
  if (!cur_mod) {
    return (smod_mod_info_t *)0x800;
  } else {
    if (!cur_mod->next)
      return 0;
    else
      return cur_mod->next;
  }
  return 0;
}

/*! \brief Get pointer to module structure for named module.
 *  \ingroup iopmgr 
 *
 *  \param name Stringname of module (eg "atad_driver").
 *  \return Pointer to module structure.
 *
 * return values:
 *   0 if not found.
 *   pointer to module structure for loaded module if found.
 */
smod_mod_info_t *smod_get_mod_by_name(const char *name)
{
  smod_mod_info_t *modptr;
  int len = strlen(name)+1;

  modptr = smod_get_next_mod(0);
  while (modptr != 0)
  {
    if (!memcmp(modptr->name, name, len))
      return modptr;
    modptr = modptr->next;
  }
  return 0;
}

/*! \brief Get instance count for given module name.
 *  \ingroup iopmgr 
 *
 *  \param name Stringname of module (eg "atad_driver").
 *  \return Number of instances loaded.
 *
 * NOTE: if doing this to check for own module, the run being used to check
 *       automatically adds one to the count.. so ret=2 means another,
 *       instance loaded as well as current one.
 */
int smod_get_modcount_by_name(const char *name)
{
  smod_mod_info_t *modptr = 0;
  int len = strlen(name)+1;
  int count = 0;

  modptr = smod_get_next_mod(0);
  while (modptr != 0)
  {
    if (!memcmp(modptr->name, name, len))
      count++;
    modptr = modptr->next;
  }

  return count;
}

/*! \brief Get version number for given module name.
 *  \ingroup iopmgr 
 *
 *  \param name Stringname of module (eg "atad_driver").
 *  \return Version number.
 *
 * return values:
 *   0 if not found.
 *   Version number for module if found. (0xXXYY, XX=HIGH, YY=LOW)
 */
int smod_get_modversion_by_name(const char *name)
{
  smod_mod_info_t *modptr = 0;
  modptr = smod_get_mod_by_name(name);
  if (modptr != 0)
    return (int)modptr->version;
  else
    return -1;
}

/*! \brief Unload the named module.
 *  \ingroup iopmgr 
 *
 *  \param name Stringname of module (eg "atad_driver").
 *  \return Status of operation.
 *
 * return values:
 *  0 if found and sucessfully unloaded.
 *  -1 if found, but failed to unload.
 *  -2 if module was not found.
 */
int smod_unload_module(const char *name)
{
  return -2;
}
