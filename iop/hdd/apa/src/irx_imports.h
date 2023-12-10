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
#ifdef APA_USE_ATAD
#include <atad.h>
#endif
#ifdef APA_USE_BDM
#include <bdm.h>
#endif
#include <cdvdman.h>
#ifdef APA_USE_DEV9
#include <dev9.h>
#endif
#include <intrman.h>
#include <iomanX.h>
#include <stdio.h>
#include <sysclib.h>
#include <sysmem.h>
#include <thbase.h>
#include <thsemap.h>

#endif /* IOP_IRX_IMPORTS_H */
