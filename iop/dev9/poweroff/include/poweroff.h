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
*/

#ifndef __POWEROFF_H__
#define __POWEROFF_H__

#include "types.h"
#include "defs.h"
#include "irx.h"
#include "loadcore.h"
#include "sysmem.h"
#include "stdio.h"
#include "sysclib.h"
#include "atad.h"
#include "list.h"
#include "errno.h"
#include "sys/fcntl.h"
#include "sys/stat.h"
#include "iomanX.h"
#include "thbase.h"
#include "thevent.h"
#include "thsemap.h"
#include "intrman.h"
#include "cdvdman.h"

typedef void (*pwoffcb)(void*);

void SetPowerOffCallback(pwoffcb func, void* param);

#endif /* __POWEROFF_H__ */