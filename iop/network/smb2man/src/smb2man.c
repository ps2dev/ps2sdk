/*
  SMB2MAN
  Ronnie Sahlberg <ronniesahlberg@gmail.com> 2021
  André Guilherme <andregui17@outlook.com> 2023-2024
  
  Based on SMBMAN:
  Copyright 2009-2010, jimmikaelkael
  Licenced under Academic Free License version 3.0
*/

#include "types.h"
#include "defs.h"
#include "intrman.h"
#include "irx.h"
#include "iomanX.h"
#include "loadcore.h"
#include "stdio.h"
#include "sysclib.h"
#include "sysmem.h"

#include "smb2_fio.h"
#include "lib/compat.h"

#define MODNAME 	"smb2man"
#define VER_MAJOR	2
#define VER_MINOR	2

IRX_ID(MODNAME, VER_MAJOR, VER_MINOR);

int _start(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    printf("%s version 0x%01x%02x - Copyright(c) 2021-2024 Ronnie Sahlberg and André Guilherme\n", MODNAME, VER_MAJOR, VER_MINOR);

	SMB2_initdev();

	return MODULE_RESIDENT_END;
}
