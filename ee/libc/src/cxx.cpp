/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Simple C++ memory allocation operators.
*/

#include <stdlib.h>
#include <malloc.h>

__attribute__((weak))
void operator delete(void *ptr) 
{
	if (ptr)
	{
		free(ptr);
	}
}

__attribute__((weak))
void* operator new(size_t len) 
{
	return malloc(len);
}

__attribute__((weak))
void operator delete[](void *ptr) 
{
	::operator delete(ptr);
}

__attribute__((weak))
void* operator new[](size_t len) 
{
	return ::operator new(len);
}

extern "C" 
__attribute__((weak))
void __cxa_pure_virtual() 
{
	/* perror("Pure virtual method called"); */
	abort();
}
