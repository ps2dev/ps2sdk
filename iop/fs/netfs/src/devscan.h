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
# device driver scanner and handler headers 
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
#define DEVSCAN_MASK (IOP_DT_FS | IOP_DT_BLOCK)

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
