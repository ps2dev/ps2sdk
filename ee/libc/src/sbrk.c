/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (C)2001, Gustavo Scotti (gustavo@scotti.com)
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# EE kernel sbrk function
# Primary function to malloc - reserved
*/

#include <kernel.h>

extern void * _end;

void *ps2_sbrk(size_t increment)
{
	static void * _heap_ptr = &_end;
	void *mp, *ret = (void *)-1;

	if (increment == 0)
		return _heap_ptr;

	/* If the area we want to allocated is past the end of our heap, we have a problem. */
	mp = _heap_ptr + increment;
	if (mp <= EndOfHeap()) {
		ret = _heap_ptr;
		_heap_ptr = mp;
	}

	return ret;
}
