/*
 * loadcore.h - Kernel module loader.
 *
 * Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
 *
 * This file is based off of the work [RO]man, Herben, and any others involved
 * in the "modules" project at http://ps2dev.pgamers.com/.  It is also based
 * off of the work of the many contributors to the ps2lib project at
 * http://ps2dev.org/.
 *
 * See the file LICENSE included with this distribution for licensing terms.
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
