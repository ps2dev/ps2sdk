/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# EE kernel sbrk function
# Primary function to malloc - reserved
*/

#include <tamtypes.h>
#include <kernel.h>

extern void * _end;
static void * _heap_ptr = &_end;

void *ps2_sbrk(size_t increment)
{
	void *mp, *ret = (void *)-1;
	int state = 0;

	/* XXX: Workaround for the obscure cases when _heap_ptr isn't
	   resolved by the linker.  */
	if (!_heap_ptr)
		_heap_ptr = &_end;

	/* increment is 0 if the caller just wants the top of the heap.  */
	if (!increment)
		return _heap_ptr;

	asm volatile ("mfc0 %0, $12" : "=r" (state));
	state = state & 0x10000;

	if (state)
		DI();

	/* If the area we want to allocated is past the end of our heap, we
	   have a problem.  */
	mp = _heap_ptr + increment;
	if (mp > EndOfHeap())
		goto out;

	ret = _heap_ptr;
	_heap_ptr = mp;

out:
	if (state)
		EI();

	return ret;
}
