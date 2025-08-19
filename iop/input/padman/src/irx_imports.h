/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Defines all IRX imports.
*/

#ifndef IOP_IRX_IMPORTS_H
#define IOP_IRX_IMPORTS_H

#include <irx.h>

#include <intrman.h>
#include <loadcore.h>
#include <sifcmd.h>
#include <sifman.h>
#include <stdio.h>
#include <sysclib.h>
#include <thbase.h>
#include <thevent.h>
#include <vblank.h>
#ifdef BUILDING_XPADMAN
#ifdef BUILDING_XPADMAN_V2
#include <rsio2man.h>
#else
#include <xsio2man.h>
#endif
#else
#include <sio2man.h>
#endif
#include <sysmem.h>

#endif
