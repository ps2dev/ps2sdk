/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Defines all IRX imports.
*/

#ifndef IOP_IRX_IMPORTS_H
#define IOP_IRX_IMPORTS_H

#include <irx.h>

/* Please keep these in alphabetical order!  */
#ifdef ATA_USE_AIFDEV9
#include <aifdev9.h>
#endif
#ifdef ATA_ENABLE_BDM
#include <bdm.h>
#endif
#ifdef ATA_USE_DEV9
#include <dev9.h>
#endif
#include <loadcore.h>
#include <stdio.h>
#include <sysclib.h>
#include <thbase.h>
#include <thevent.h>

#endif /* IOP_IRX_IMPORTS_H */
