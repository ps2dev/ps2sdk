/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Additional loadcore functions only found in newer IOPRP images
 */

#ifndef IOP_XLOADCORE_H
#define IOP_XLOADCORE_H
#include "loadcore.h"

int SetRebootTimeLibraryHandlingMode(struct irx_export_table *exports, int mode);
#define I_SetRebootTimeLibraryHandlingMode DECLARE_IMPORT(27, SetRebootTimeLibraryHandlingMode)

#endif /* IOP_XLOADCORE_H */
