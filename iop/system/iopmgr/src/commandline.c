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
# command line parser and handler for iop manager.
*/

#include "defs.h"
#include "iomanX.h"
#include "stdio.h"
#include "sysclib.h"

#include "iopmgr.h"

extern int shutdown();

/*! \brief Prints out a list of all currently registered libraries
 *         with their name and version.
 *  \ingroup iopmgr 
 */
void list_all_libraries()
{
  char name[9];
  iop_library_table_t *libtable;
  iop_library_t *libptr;

  printf("Registered Libraries:\n");
  libtable = GetLibraryEntryTable();
  libptr = libtable->tail;
  while (libptr)
  {
    /* We only have a 8-char buffer for the name, so we need to convert it into a C
       string just in case the name is 8 chars.  */
    memset(name, 0, sizeof(name));
    memcpy(name, libptr->name, 8);
    printf("    %-20s v%d.%d\n",name, 
     (libptr->version&0xff00)>>8,libptr->version&0xff);
    libptr = libptr->prev;
  }
  printf("\n");
}


/*! \brief Prints out a list of all currently loaded IRX modules
 *         with description and version number.
 *  \ingroup iopmgr 
 */
void list_all_modules()
{
  smod_mod_info_t *info;
  printf("Loaded Modules:\n");
  info = smod_get_next_mod(0);
  while (info != 0)
  {
    printf("    %-40s v%d.%d (%d)\n",info->name,
      (info->version&0xff00)>>8,(info->version&0xff),info->flags);
    info = info->next;
  }
  printf("\n");
}

/*! \brief Print out all devices for ONLY ioman
 *         and shows which one they belong to.
 *  \ingroup iopmgr 
 */
void list_devices_ioman()
{
	smod_mod_info_t *info;
	iop_device_t **devinfo_table;
	int i;

	if (!(info = smod_get_mod_by_name(IOPMGR_IOMAN_IDENT))) {
		printf("Unable to find '%s's module info!\n",IOPMGR_IOMAN_IDENT);
		return;
	}

	/* Find the start of the device info array, in .bss.  */
	devinfo_table = (iop_device_t **)(info->text_start + info->text_size + info->data_size + 0x0c);

	printf("Known '%s' devices:\n",IOPMGR_IOMAN_IDENT);
	/* The device info table had 16 entries, but some may be empty.  Just look at all of them.  */
	for (i = 0; i < 16; i++) {
		if (devinfo_table[i])
			printf("    %-10s: %-27s  v%d.%d type:%08X\n",
			  devinfo_table[i]->name, devinfo_table[i]->desc,
			  (devinfo_table[i]->version&0xff00)>>8,
			  (devinfo_table[i]->version&0xff),
			  devinfo_table[i]->type);
	}
	printf("\n");
}

/*! \brief Print out all devices for ONLY iomanx
 *         and shows which one they belong to.
 *  \ingroup iopmgr 
 */
void list_devices_iomanx()
{
	smod_mod_info_t *info;
	iop_device_t **devinfo_table;
	int i;

	if (!(info = smod_get_mod_by_name(IOPMGR_IOMANX_IDENT))) {
		printf("Unable to find '%s's module info!\n",IOPMGR_IOMANX_IDENT);
		return;
	}

	/* Find the start of the device info array, in .bss.  */
	devinfo_table = (iop_device_t **)(info->text_start + info->text_size + info->data_size + 0x00);

	printf("Known '%s' devices:\n",IOPMGR_IOMANX_IDENT);
	/* The device info table had 32 entries, but some may be empty.  Just look at all of them.  */
	for (i = 0; i < 32; i++) {
		if (devinfo_table[i])
			printf("    %-10s: %-28s v%d.%d type:%08X\n", 
			devinfo_table[i]->name, devinfo_table[i]->desc,
			(devinfo_table[i]->version&0xff00)>>8,
			(devinfo_table[i]->version&0xff),
			devinfo_table[i]->type);
	}
	printf("\n");
}

/*! \brief Print out all devices for both ioman and iomanx
 *         and shows which one they belong to.
 *
 *  \param mgrtype Device manager type search mask (IOPMGR_DEVTYPE_XXXX).
 *  \ingroup iopmgr 
 */
void list_fs_devices(int mgrtype)
{
  int count;
  int i;
  char *bufptr;
  char buffer[256];
  bufptr = &buffer[0];

  count = iopmgr_get_devicelist(mgrtype,IOP_DT_FS,bufptr);
  printf("filesystems found : %d\n",count);
  
  for (i=0;i<count;i++)
  {
    printf(" %02d : %-10s - ",i,bufptr);
    switch(iopmgr_get_devicetype(bufptr))
    {
      case IOPMGR_DEVTYPE_IOMAN:     printf("%s\n",IOPMGR_IOMAN_IDENT);break;
      case IOPMGR_DEVTYPE_IOMANX:    printf("%s\n",IOPMGR_IOMANX_IDENT);break;
      default: printf("Invalid\n");break;
    }
    bufptr += strlen(bufptr)+1;
  }
}


/*! \brief Handles basic parsing of commands, for command execution mode.
 *  \ingroup iopmgr 
 *
 *  \param command Pointer to command string.
 */
void cmdline_handle(char *command)
{
  if (!strncmp("modlist",command,7))
    list_all_modules();
  else if (!strncmp("iomanx",command,6))
  {
    list_devices_iomanx();
    list_fs_devices(IOPMGR_DEVTYPE_IOMANX);
  }
  else if (!strncmp("ioman",command,5))
  {
    list_devices_ioman();
    list_fs_devices(IOPMGR_DEVTYPE_IOMAN);
  }
  else if (!strncmp("devices",command,7))
  {
    list_devices_ioman();
    list_devices_iomanx();
    list_fs_devices(IOPMGR_DEVTYPE_ALL);
  }
  else if (!strncmp("libs",command,4))
    list_all_libraries();
  else if (!strncmp("release",command,7))
    shutdown();
  else if (!strncmp("help",command,4))
  {
    printf("\nCommands are:\n modlist - List of Loaded Modules\n iomanx  - List iomanx Devices\n ioman   - List ioman Devices\n devices - List all Devices\n libs    - List Registered Libraries\n release - release iopmgr lib if loaded\n help    - This page\n");
  }
  else printf("\nUnknown Command , try 'help'\n");
}
