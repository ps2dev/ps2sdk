/*
  Copyright 2009-2010, jimmikaelkael
  Licenced under Academic Free License version 3.0
*/

#include "types.h"
#include "defs.h"
#include "irx.h"
#include "iomanX.h"
#include "loadcore.h"
#include "stdio.h"
#include "sysclib.h"

#include "smb_fio.h"
#include "smb.h"
#include "debug.h"

#define MODNAME   "smbman"
#define VER_MAJOR 2
#define VER_MINOR 2

IRX_ID(MODNAME, VER_MAJOR, VER_MINOR);

//-------------------------------------------------------------------------
int _start(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    printf("%s version 0x%01x%02x start!\n", MODNAME, VER_MAJOR, VER_MINOR);

    smb_initdev();

    return MODULE_RESIDENT_END;
}
