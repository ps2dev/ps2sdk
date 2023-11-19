/*
  SMB2MAN
  Ronnie Sahlberg <ronniesahlberg@gmail.com> 2021

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

#define MODNAME 	"smbman"
#define VER_MAJOR	2
#define VER_MINOR	2

IRX_ID(MODNAME, VER_MAJOR, VER_MINOR);

int _start(int argc, char** argv)
{
	SMB2_initdev();

	return MODULE_RESIDENT_END;
}

void *malloc(int size)
{
    void* result;
    int OldState;

    CpuSuspendIntr(&OldState);
    result = AllocSysMemory(ALLOC_FIRST, size, NULL);
    CpuResumeIntr(OldState);

    return result;
}

void free(void *ptr)
{
    int OldState;

    CpuSuspendIntr(&OldState);
    FreeSysMemory(ptr);
    CpuResumeIntr(OldState);
}

void *calloc(size_t nmemb, size_t size)
{
    size_t s = nmemb * size;
    void *ptr;

    ptr = malloc(s);
    memset(ptr, 0, s);
  
    return ptr;
}
