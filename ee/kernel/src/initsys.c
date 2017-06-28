/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * initsys - basic initialization/termination functions for libkernel.
 */

#include "kernel.h"
#include "string.h"

extern char **_kExecArg;
void *SetArg(const char *filename, int argc, char *argv[]);

#ifdef F__InitSys
void _InitSys(void)
{
	InitAlarm();
	InitThread();
	InitExecPS2();
	InitTLBFunctions();
}
#endif

#ifdef F_TerminateLibrary
void TerminateLibrary(void)
{
	InitTLB();
}
#endif

#ifdef F__initsys_internals
char **_kExecArg;
#endif

#ifdef F_SetArg
struct SyscallData{
	int syscall;
	void *function;
};

struct SyscallData SysEntry = {
	0x5A,
	&kCopyBytes
};

void *SetArg(const char *filename, int argc, char *argv[])
{
	char *ptr;
	int len, i;

	ptr = (char*)((u32*)_kExecArg + 16);
	setup(SysEntry.syscall, SysEntry.function);
	argc = (argc >= 16) ? 15 : argc;
	Copy(&_kExecArg[0], &ptr, 4);
	len = strlen(filename) + 1;
	Copy(ptr, filename, len);
	ptr += len;

	for(i = 0; i < argc; i++)
	{
		Copy(&_kExecArg[1+i], &ptr, 4);
		len = strlen(argv[i]) + 1;
		Copy(ptr, argv[i], len);
		ptr += len;
	}

	return _kExecArg;
}
#endif

#ifdef F_Exit
void Exit(s32 exit_code)
{
	TerminateLibrary();
	_Exit(exit_code);
}
#endif

#ifdef F_ExecPS2
s32  ExecPS2(void *entry, void *gp, int num_args, char *args[])
{
	SetArg("", num_args, args);
	TerminateLibrary();
	return _ExecPS2(entry, gp, num_args, &_kExecArg[1]);
}
#endif

#ifdef F_LoadExecPS2
void LoadExecPS2(const char *filename, s32 num_args, char *args[])
{
	SetArg(filename, num_args, args);
	TerminateLibrary();
	_LoadExecPS2(_kExecArg[0], num_args, &_kExecArg[1]);
}
#endif

#ifdef F_ExecOSD
void ExecOSD(int num_args, char *args[])
{
	SetArg("", num_args, args);
	TerminateLibrary();
	_ExecOSD(num_args, &_kExecArg[1]);
}
#endif
