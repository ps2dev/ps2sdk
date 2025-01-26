/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "srxfixup_internal.h"

SLink *add_ring_top(SLink *tailp, SLink *elementp)
{
	if ( !elementp )
	{
		return tailp;
	}
	if ( tailp )
	{
		elementp->next = tailp->next;
		tailp->next = elementp;
	}
	else
	{
		tailp = elementp;
		elementp->next = elementp;
	}
	return tailp;
}

SLink *add_ring_tail(SLink *tailp, SLink *elementp)
{
	SLink *tailpa;

	tailpa = add_ring_top(tailp, elementp);
	if ( !elementp )
	{
		return tailpa;
	}
	if ( tailpa )
	{
		return tailpa->next;
	}
	return 0;
}

SLink *joint_ring(SLink *tailp, SLink *otherring)
{
	SLink *othertop;

	if ( !otherring )
	{
		return tailp;
	}
	if ( !tailp )
	{
		return otherring;
	}
	othertop = otherring->next;
	otherring->next = tailp->next;
	tailp->next = othertop;
	return tailp;
}

SLink *ring_to_liner(SLink *tailp)
{
	SLink *top;

	if ( !tailp )
	{
		return 0;
	}
	top = tailp->next;
	tailp->next = 0;
	return top;
}
