/*
  _____     ___ ____ 
   ____|   |    ____|      PSX2 OpenSource Project
  |     ___|   |____       (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
  ------------------------------------------------------------------------
  abort.c
*/

#include <tamtypes.h>
#include <kernel.h>

__attribute__((weak))
void abort()
{
	printf("Program aborted.\n");

	while (1)
		_exit(1);
}

__attribute__((weak))
void exit(int retval)
{
	while (1)
		_exit(retval);
}
