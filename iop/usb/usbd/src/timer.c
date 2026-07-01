/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "usbdpriv.h"

int addTimerCallback(UsbdTimerCbStruct_t *arg, TimerCallback func, void *cbArg, int delay)
{
	UsbdTimerCbStruct_t *pos;

	if ( arg->m_isActive )
		return -1;
	arg->m_isActive = 1;
	delay -= ( delay > 0 ) ? 1 : 0;
	arg->m_callbackProc = func;
	arg->m_callbackArg = cbArg;
	for ( pos = memPool->m_timerListStart; pos && delay >= (int)pos->m_delayCount;
				delay -= pos->m_delayCount, pos = pos->m_prev )
	{
	}
	arg->m_next = pos ? pos->m_next : memPool->m_timerListEnd;
	*( pos ? (pos->m_next ? &(pos->m_next->m_prev) : &(memPool->m_timerListStart)) : (memPool->m_timerListEnd ? &(memPool->m_timerListEnd->m_prev) : &(memPool->m_timerListStart)) ) = arg;
	arg->m_prev = pos;
	if ( pos )
	{
		pos->m_next = arg;
		pos->m_delayCount -= delay;
	}
	else
	{
		memPool->m_timerListEnd = arg;
	}
	arg->m_delayCount = delay;
	memPool->m_ohciRegs->HcInterruptEnable = OHCI_INT_SF;
	return 0;
}

int cancelTimerCallback(UsbdTimerCbStruct_t *arg)
{
	if ( !arg->m_isActive )
	{
		return -1;
	}
	*( arg->m_prev ? &(arg->m_prev->m_next) : &(memPool->m_timerListEnd) ) = arg->m_next;
	*( arg->m_next ? &(arg->m_next->m_prev) : &(memPool->m_timerListStart) ) = arg->m_prev;
	arg->m_isActive = 0;
	arg->m_next = NULL;
	arg->m_prev = NULL;
	return 0;
}

void handleTimerList(void)
{
	UsbdTimerCbStruct_t *timer;

	timer = memPool->m_timerListStart;
	if ( timer )
	{
		timer->m_delayCount -= ( timer->m_delayCount > 0 ) ? 1 : 0;
		while ( 1 )
		{
			timer = memPool->m_timerListStart;
			if ( !timer || (int)timer->m_delayCount > 0 )
				break;
			dbg_printf("timer expired\n");
			memPool->m_timerListStart = timer->m_prev;
			*( timer->m_prev ? &(timer->m_prev->m_next) : &(memPool->m_timerListEnd) ) = NULL;
			timer->m_next = NULL;
			timer->m_prev = NULL;
			timer->m_isActive = 0;
			timer->m_callbackProc(timer->m_callbackArg);
		}
	}
	// disable SOF interrupts if there are no timers left
	if ( !memPool->m_timerListStart )
		memPool->m_ohciRegs->HcInterruptDisable = OHCI_INT_SF;
}
