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
 * exit - Termination functions for libkernel.
 */

#include "kernel.h"
#include "string.h"

extern char **_kExecArg;
const char *SetArg(const char *filename, int argc, char *argv[]);

#ifdef F__exit_internals
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

#define SETARG_MAX_ARGS	15

const char *SetArg(const char *filename, int argc, char *argv[])
{
	const char *filenameOut;
	char *ptr;
	int len, i;

	ptr = (char*)(_kExecArg + 16);
	filenameOut = ptr;
	setup(SysEntry.syscall, SysEntry.function);
	argc = (argc > SETARG_MAX_ARGS) ? SETARG_MAX_ARGS : argc;
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

	return filenameOut;
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
	const char *pFilename;

	pFilename = SetArg(filename, num_args, args);
	TerminateLibrary();
	_LoadExecPS2(pFilename, num_args, &_kExecArg[1]);
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
