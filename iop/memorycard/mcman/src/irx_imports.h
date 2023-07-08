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

/* Please keep these in alphabetical order!  */

#include <cdvdman.h>
#ifdef BUILDING_XFROMMAN
#include <fls.h>
#endif
#include <intrman.h>
#ifdef BUILDING_VMCMAN
#include <iomanX.h>
#else
#include <ioman.h>
#endif
#include <loadcore.h>
#if !defined(BUILDING_XFROMMAN) && !defined(BUILDING_VMCMAN)
#include <modload.h>
#include <secrman.h>
#ifdef BUILDING_XMCMAN
#ifndef SIO2MAN_V2
#include <xsio2man.h>
#else
#include <rsio2man.h>
#endif
#else
#include <sio2man.h>
#endif
#endif
#ifdef SIO_DEBUG
#include <sior.h>
#endif
#include <stdio.h>
#include <sysclib.h>
#if !defined(BUILDING_XFROMMAN) && !defined(BUILDING_VMCMAN)
#include <timrman.h>
#endif
#include <thbase.h>
#include <thsemap.h>

#endif /* IOP_IRX_IMPORTS_H */
