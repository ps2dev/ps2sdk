/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Kernel module loader.
*/

#ifndef IOP_MODLOAD_H
#define IOP_MODLOAD_H

#include "types.h"
#include "irx.h"

#define modload_IMPORTS_start DECLARE_IMPORT_TABLE(modload, 1, 1)
#define modload_IMPORTS_end END_IMPORT_TABLE

int LoadStartModule(const char *name, int arglen, const char *args, int *result);
#define I_LoadStartModule DECLARE_IMPORT(7, LoadStartModule)

#endif /* IOP_MODLOAD_H */
