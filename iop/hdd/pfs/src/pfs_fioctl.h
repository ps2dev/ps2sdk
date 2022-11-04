/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _PFS_FIOCTL_H
#define _PFS_FIOCTL_H

///////////////////////////////////////////////////////////////////////////////
//	Function declarations

int pfsFioIoctl(iomanX_iop_file_t *f, int cmd, void *param);
int pfsFioIoctl2(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
int pfsFioDevctl(iomanX_iop_file_t *f, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);

void pfsFioDevctlCloseAll(void);

#endif /* _PFS_FIOCTL_H */
