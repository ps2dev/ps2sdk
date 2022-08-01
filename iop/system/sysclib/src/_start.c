/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"

#define MODNAME "System_C_lib"
IRX_ID(MODNAME, 1, 1);

extern struct irx_export_table _exp_sysclib;
extern struct irx_export_table _exp_stdio;

int _start(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

    if (RegisterLibraryEntries(&_exp_sysclib) != 0)
        return MODULE_NO_RESIDENT_END;
    if (RegisterLibraryEntries(&_exp_stdio) != 0)
        return MODULE_NO_RESIDENT_END;

    return MODULE_RESIDENT_END;
}
