/*
 * devscan.h - device driver scanner and handler headers 
 *
 * Copyright (c) 2004 adresd   <adresd_ps2dev@yahoo.com>
 * 
 * Rather than require iopmgr to be loaded, these are included here
 * as quite small and compact.
 * Also included as wanted a different format for return values.
 *
 * Licensed under the AFL v2.0. See the file LICENSE included with this
 * distribution for licensing terms.
 */

#define DEVSCAN_IOMAX  16
#define DEVSCAN_IOXMAX 32
/*! \brief Maximum number of devices handled.
 *  \ingroup ps2netfs 
 */
#define DEVSCAN_MAX (DEVSCAN_IOMAX+DEVSCAN_IOXMAX+1)

/*! \brief Device scan mask.
 *  \ingroup ps2netfs 
 */
#define DEVSCAN_MASK (IOP_DT_FS)

/*! \brief Device type structure.
 *  \ingroup ps2netfs 
 */
typedef struct {
  char name[256];
  int  devtype;
  int  len;
} dev_table_t;

int devscan_gettype(char *name);
int devscan_setup(int devtype);
int devscan_getdevlist(char *buffer);
smod_mod_info_t *devscan_getmodule(const char *name);
