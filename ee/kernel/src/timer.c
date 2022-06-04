/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Some routines to do some timer work
 */

#include <kernel.h>
#include <timer.h>
#include <string.h>
#include <stdint.h>

#ifdef F_timer_data
timer_ee_global_struct g_Timer;
timer_ee_entry_struct g_CounterBuf[128];
alarm_ee_entry_struct g_AlarmBuf[64];
alarm_ee_entry_struct *g_pFreeAlarm = NULL;
#else
extern timer_ee_global_struct g_Timer;
extern timer_ee_entry_struct g_CounterBuf[128];
extern alarm_ee_entry_struct g_AlarmBuf[64];
extern alarm_ee_entry_struct *g_pFreeAlarm;
#endif

#ifdef F_SetT2
s32 SetT2(volatile void *a1, u32 a2)
{
	u32 v2;
	u32 v3;
	u32 v5;

	__asm__ __volatile__ ("mfc0\t%0, $12" : "=r" (v3));
	v3 = v3 & 0x10000;
	if (v3 != 0)
	{
		do
		{
			DI();
			EE_SYNCP();
			__asm__ __volatile__ ("mfc0\t%0, $12" : "=r" (v5));
		}
		while ((v5 & 0x10000) != 0);
	}
	__asm__ __volatile__ ("mfc0\t%0, $12" : "=r" (v5));
	EE_SYNCP();
	v5 = ((v5 | 6) ^ 2) | v3;
	__asm__ __volatile__ ("mtc0\t%0, $12" : "=r" (v5));
	EE_SYNCP();
	*((volatile u32 *)a1) = a2;
	__asm__ __volatile__ ("mtc0\t%0, $30" : "=r" (v2));
	EE_SYNCP();
	__asm__ __volatile__ ("eret");
	return v2;
}
#endif

#ifdef F_SetT2_COUNT
void SetT2_COUNT(u32 a1)
{
	SetT2((volatile void *)0xB0001000, a1);
}
#endif

#ifdef F_SetT2_MODE
void SetT2_MODE(u32 a1)
{
	SetT2((volatile void *)0xB0001010, a1);
}
#endif

#ifdef F_SetT2_COMP
void SetT2_COMP(u32 a1)
{
	SetT2((volatile void *)0xB0001020, a1);
}
#endif

#ifdef F_InitTimer
s32 InitTimer(s32 a1)
{
	s32 v6;
	u32 oldintr;
	u32 v8;

	if (g_Timer.intc_handler >= 0)
	{
		return 0x80008001;
	}
	g_Timer.timer_handled_count = 0;
	g_Timer.timer_counter = 0;
	memset(g_CounterBuf, 0, sizeof(g_CounterBuf));
	g_Timer.current_counter_buf = &g_CounterBuf[0];
	for (u32 i = 0; i < ((sizeof(g_CounterBuf) / sizeof(g_CounterBuf[0])) - 1); i += 1)
	{
		g_CounterBuf[i].counter_buf_next = &g_CounterBuf[i + 1];
	}
	g_CounterBuf[(sizeof(g_CounterBuf) / sizeof(g_CounterBuf[0])) - 1].counter_buf_next = 0;
	ForTimer_InitAlarm();
	v6 = AddIntcHandler2(INTC_TIM2, timer_intc_handler_callback, 0, NULL);
	if (v6 < 0)
	{
		return 0x80009021;
	}
	g_Timer.intc_handler = v6;
	oldintr = DIntr();
	v8 = ((*T2_MODE) & (~0x3)) | a1 | 0x300;
	if ((((*T2_MODE) & 0x80) | (a1 & 0x80)) == 0)
	{
		v8 = ((*T2_MODE) & (~0x3)) | a1 | 0xF80;
		SetT2_COUNT(0);
		SetT2_COMP(0xFFFF);
	}
	SetT2_MODE(v8);
	EnableIntc(INTC_TIM2);
	if (oldintr != 0)
	{
		EIntr();
	}
	return 0;
}
#endif

#ifdef F_EndTimer
s32 EndTimer(void)
{
	u32 oldintr;

	if (g_Timer.intc_handler < 0)
	{
		return 0x80008001;
	}
	if (g_Timer.timer_counter)
	{
		return 0x80000010;
	}
	oldintr = DIntr();
	if (RemoveIntcHandler(INTC_TIM2, g_Timer.intc_handler) == 0)
	{
		DisableIntc(INTC_TIM2);
		SetT2_MODE(0xC00);
		SetT2_COUNT(0);
	}
	g_Timer.timer_handled_count = 0;
	g_Timer.intc_handler = -1;
	if (oldintr != 0)
	{
		EIntr();
	}
	return 0;
}
#endif

#ifdef F_GetTimerPreScaleFactor
s32 GetTimerPreScaleFactor(void)
{
	if (g_Timer.intc_handler >= 0)
	{
		return (*T2_MODE) & 3;
	}
	else
	{
		return 0x80008001;
	}
}
#endif

#ifdef F_StartTimerSystemTime
s32 StartTimerSystemTime(void)
{
	u32 oldintr;

	oldintr = DIntr();
	if (((*T2_MODE) & 0x80) != 0)
	{
		if (oldintr != 0)
		{
			EIntr();
		}
		return 1;
	}
	else
	{
		SetT2_MODE(((*T2_MODE) & (~0xC80)) | 0x80);
		SetNextComp(iGetTimerSystemTime());
		if (oldintr != 0)
		{
			EIntr();
		}
		return 0;
	}
}
#endif

#ifdef F_StopTimerSystemTime
s32 StopTimerSystemTime(void)
{
	u32 oldintr;

	oldintr = DIntr();
	if (((*T2_MODE) & 0x80) != 0)
	{
		SetT2_MODE((*T2_MODE) & (~0xC80));
		if (oldintr != 0)
		{
			EIntr();
		}
		return 1;
	}
	else
	{
		if (oldintr != 0)
		{
			EIntr();
		}
		return 0;
	}
}
#endif

#ifdef F_SetNextComp
void SetNextComp(u64 time)
{
	u32 v2;
	timer_ee_entry_struct *counter_buf_next;
	u64 i;
	u64 v5;
	u64 v6;
	u32 v7;
	u64 timer_system_time;
	u64 timer_count;

	if (g_Timer.last_handled_timer_id < 0)
	{
		v2 = (*T2_MODE);
		if (g_Timer.timer_struct_head != NULL)
		{
			counter_buf_next = g_Timer.timer_struct_head->counter_buf_next;
			i = g_Timer.timer_struct_head->comparison_value + g_Timer.timer_struct_head->timer_system_time - g_Timer.timer_struct_head->timer_count;
			if (g_Timer.timer_struct_head->counter_buf_next != NULL)
			{
				v5 = counter_buf_next->timer_system_time;
				v6 = counter_buf_next->timer_count;
				v7 = (*T2_MODE) & 3;
				if (counter_buf_next->comparison_value + v5 - v6 < i + 0x7333)
				{
					i = counter_buf_next->comparison_value + v5 - v6;
					for (;;)
					{
						counter_buf_next = counter_buf_next->counter_buf_next;
						if (counter_buf_next == NULL)
						{
							break;
						}
						timer_system_time = counter_buf_next->timer_system_time;
						timer_count = counter_buf_next->timer_count;
						if (counter_buf_next->comparison_value + timer_system_time - timer_count >= i + 0x7333)
						{
							break;
						}
						i = counter_buf_next->comparison_value + timer_system_time - timer_count;
					}
				}
				else
				{
					v7 = (*T2_MODE) & 3;
				}
			}
			else
			{
				v7 = (*T2_MODE) & 3;
			}
			if ((i - time) >= 0x7333)
			{
				SetT2_MODE((*T2_MODE) & (~0x800));
				SetT2_COMP(i >> (v7 << 2));
			}
			else
			{
				SetT2_COMP((*T2_COUNT) + (0x7333 >> (v7 << 2)));
				SetT2_MODE(v2 & (~0x800));
			}
		}
		else
		{
			SetT2_COMP(0);
			SetT2_MODE(v2 & (~0x800));
		}
	}
}
#endif

#ifdef F_InsertAlarm_ForTimer
void InsertAlarm_ForTimer(timer_ee_entry_struct *a1)
{
	timer_ee_entry_struct *v1;
	timer_ee_entry_struct *v2;

	v1 = NULL;
	for (v2 = g_Timer.timer_struct_head; ; v2 = v2->counter_buf_next)
	{
		if (v2 == NULL)
		{
			break;
		}
		if (a1->comparison_value + a1->timer_system_time - a1->timer_count < v2->comparison_value + v2->timer_system_time - v2->timer_count)
		{
			break;
		}
		v1 = v2;
	}
	a1->counter_buf_previous = v1;
	a1->counter_buf_next = v2;
	if (v2 != NULL)
	{
		v2->counter_buf_previous = a1;
	}
	if (v1 != NULL)
	{
		v1->counter_buf_next = a1;
	}
	else
	{
		g_Timer.timer_struct_head = a1;
	}
}
#endif

#ifdef F_UnlinkAlarm_ForTimer
timer_ee_entry_struct *UnlinkAlarm_ForTimer(timer_ee_entry_struct *a1)
{
	timer_ee_entry_struct *ret;

	ret = a1->counter_buf_next;
	if (a1->counter_buf_previous)
	{
		a1->counter_buf_previous->counter_buf_next = ret;
	}
	else
	{
		g_Timer.timer_struct_head = a1->counter_buf_next;
	}
	if (ret != NULL)
	{
		ret->counter_buf_previous = a1->counter_buf_previous;
	}
	a1->counter_buf_previous = 0;
	return ret;
}
#endif

#ifdef F_timer_intc_handler_callback
s32 timer_intc_handler_callback(s32 cause, void *arg, void *addr)
{
	timer_ee_entry_struct *timer_struct_head;
	u32 v5;
	u32 v6;
	u64 v7;
	u64 v8;
	timer_ee_entry_struct *v9;
	u32 v10;
	u64 timer_system_time;
	u64 v13;
	timer_ee_entry_struct *current_counter_buf;
	u32 v16;
	u32 v17;

	if (((*T2_MODE) & 0x400) != 0)
	{
		timer_struct_head = g_Timer.timer_struct_head;
		if (g_Timer.timer_struct_head != NULL)
		{
			for (;;)
			{
				v5 = (*T2_COUNT);
				v6 = (*T2_MODE);
				v7 = timer_struct_head->comparison_value + timer_struct_head->timer_system_time - timer_struct_head->timer_count;
				if (((*T2_MODE) & 0x800) != 0)
				{
					g_Timer.timer_handled_count += 1;
					SetT2_MODE((*T2_MODE) & (~0x400));
					v5 = (*T2_COUNT);
				}
				v8 = ((g_Timer.timer_handled_count << 16) | v5) << ((v6 & 3) << 2);
				if (v8 < v7)
				{
					break;
				}
				v9 = UnlinkAlarm_ForTimer(timer_struct_head);
				v10 = (uintptr_t)timer_struct_head << 4;
				timer_system_time = timer_struct_head->timer_system_time;
				g_Timer.last_handled_timer_id = v10 | timer_struct_head->timer_id_check_guard;
				SetGP(timer_struct_head->last_gp);
				v13 = timer_struct_head->callback_handler(
					v10 | timer_struct_head->timer_id_check_guard,
					timer_struct_head->comparison_value,
					(v8 + timer_struct_head->timer_count) - timer_system_time,
					timer_struct_head->callback_handler_argument,
					addr);
				if (v13 != 0)
				{
					if (v13 == (u64)-1)
					{
						current_counter_buf = g_Timer.current_counter_buf;
						g_Timer.current_counter_buf = timer_struct_head;
						timer_struct_head->counter_buf_next = current_counter_buf;
						timer_struct_head->timer_id_check_guard = 0;
						timer_struct_head->timer_flags = 0;
					}
					else
					{
						if (v13 <= 0x3998)
						{
							v13 = 0x3999;
						}
						timer_struct_head->comparison_value += v13;
						InsertAlarm_ForTimer(timer_struct_head);
					}
				}
				else
				{
					timer_struct_head->timer_flags &= ~2u;
				}
				timer_struct_head = v9;
				if (v9 == NULL)
				{
					break;
				}
			}
		}
	}
	g_Timer.last_handled_timer_id = -1;
	v16 = (*T2_COUNT);
	v17 = (*T2_MODE);
	if (((*T2_MODE) & 0x800) != 0)
	{
		g_Timer.timer_handled_count += 1;
		SetT2_MODE((*T2_MODE) & (~0x400));
		v16 = (*T2_COUNT);
	}
	SetNextComp(((g_Timer.timer_handled_count << 16) | v16) << ((v17 & 3) << 2));
	if (((*T2_MODE) & 0x800) != 0)
	{
		g_Timer.timer_handled_count += 1;
		SetT2_MODE((*T2_MODE) & (~0x400));
	}
	EE_SYNCL();
	__asm__ __volatile__ ("ei");
	return 0;
}
#endif

#ifdef F_iGetTimerSystemTime
u64 iGetTimerSystemTime(void)
{
	u64 v0;
	u64 timer_handled_count;
	u64 v2;
	u64 v3;

	v0 = (*T2_COUNT);
	timer_handled_count = g_Timer.timer_handled_count;
	v2 = 2;
	if (((*T2_MODE) & 0x800) != 0)
	{
		v0 = (*T2_COUNT);
		timer_handled_count += 1;
	}
	v3 = timer_handled_count << 16;
	if (((*T2_MODE) & 3) == 0)
	{
		v2 = 0;
	}
	return (v0 | v3) << (v2 << ((*T2_MODE) & 3));
}
#endif

#ifdef F_GetTimerSystemTime
u64 GetTimerSystemTime(void)
{
	u32 oldintr;
	u64 v1;
	u64 timer_handled_count;
	u64 v3;
	u64 v4;
	u64 v5;

	oldintr = DIntr();
	v1 = (*T2_COUNT);
	timer_handled_count = g_Timer.timer_handled_count;
	v3 = 2;
	if (((*T2_MODE) & 0x800) != 0)
	{
		v1 = (*T2_COUNT);
		timer_handled_count += 1;
	}
	v4 = timer_handled_count << 16;
	if (((*T2_MODE) & 3) == 0)
	{
		v3 = 0;
	}
	v5 = (v1 | v4) << (v3 << ((*T2_MODE) & 3));
	if (oldintr != 0)
	{
		EIntr();
	}
	return v5;
}
#endif

#ifdef F_iAllocTimerCounter
s32 iAllocTimerCounter(void)
{
	timer_ee_entry_struct *current_counter_buf;
	timer_ee_entry_struct *counter_buf_next;

	current_counter_buf = g_Timer.current_counter_buf;
	if (g_Timer.current_counter_buf == 0)
	{
		return 0x80008005;
	}
	counter_buf_next = g_Timer.current_counter_buf->counter_buf_next;
	g_Timer.current_counter_buf->callback_handler = NULL;
	g_Timer.timer_counter += 1;
	current_counter_buf->timer_flags = 0;
	g_Timer.current_counter_buf = counter_buf_next;
	g_Timer.timer_counter_total += 1;
	current_counter_buf->timer_count = 0;
	current_counter_buf->timer_id_check_guard = (((g_Timer.timer_counter_total) << 1) & 0x3FE) | 1;
	return ((uintptr_t)current_counter_buf << 4) | current_counter_buf->timer_id_check_guard;
}
#endif

#ifdef F_AllocTimerCounter
s32 AllocTimerCounter(void)
{
	u32 oldintr;
	s32 ret;

	oldintr = DIntr();
	ret = iAllocTimerCounter();
	if (oldintr != 0)
	{
		EIntr();
	}
	return ret;
}
#endif

#ifdef F_iFreeTimerCounter
s32 iFreeTimerCounter(s32 id)
{
	timer_ee_entry_struct *timer_ee_entry;

	timer_ee_entry = (timer_ee_entry_struct *)((uintptr_t)id >> 10 << 6);
	if (timer_ee_entry == NULL || id < 0 || (id & 0x3FF) != timer_ee_entry->timer_id_check_guard)
	{
		return 0x80008002;
	}
	if (g_Timer.last_handled_timer_id == id)
	{
		return 0x80000010;
	}
	if ((timer_ee_entry->timer_flags & 2) != 0)
	{
		UnlinkAlarm_ForTimer(timer_ee_entry);
	}
	timer_ee_entry->timer_id_check_guard = 0;
	timer_ee_entry->timer_flags = 0;
	timer_ee_entry->counter_buf_next = g_Timer.current_counter_buf;
	g_Timer.current_counter_buf = timer_ee_entry;
	g_Timer.timer_counter -= 1;
	return 0;
}
#endif

#ifdef F_FreeTimerCounter
s32 FreeTimerCounter(s32 id)
{
	u32 oldintr;
	s32 ret;

	oldintr = DIntr();
	ret = iFreeTimerCounter(id);
	if (oldintr != 0)
	{
		EIntr();
	}
	return ret;
}
#endif

#ifdef F_iGetTimerUsedUnusedCounters
s32 iGetTimerUsedUnusedCounters(u32 *used_counters, u32 *unused_counters)
{
	if (g_Timer.intc_handler < 0)
	{
		return 0x80008001;
	}
	if (used_counters != NULL)
	{
		*used_counters = g_Timer.timer_counter;
	}
	if (unused_counters != NULL)
	{
		*unused_counters = (sizeof(g_CounterBuf) / sizeof(g_CounterBuf[0])) - g_Timer.timer_counter;
	}
	return 0;
}
#endif

#ifdef F_GetTimerUsedUnusedCounters
s32 GetTimerUsedUnusedCounters(u32 *used_counters, u32 *unused_counters)
{
	u32 oldintr;
	s32 ret;

	oldintr = DIntr();
	ret = iGetTimerUsedUnusedCounters(used_counters, unused_counters);
	if (oldintr != 0)
	{
		EIntr();
	}
	return ret;
}
#endif

#ifdef F_iStartTimerCounter
s32 iStartTimerCounter(s32 id)
{
	timer_ee_entry_struct *timer_ee_entry;
	u64 timer_system_time_new;
	u32 timer_flags;

	timer_ee_entry = (timer_ee_entry_struct *)((uintptr_t)id >> 10 << 6);
	if (timer_ee_entry == NULL || id < 0 || (id & 0x3FF) != timer_ee_entry->timer_id_check_guard)
	{
		return 0x80008002;
	}
	if (g_Timer.last_handled_timer_id == id)
	{
		return 0x80000010;
	}
	if ((timer_ee_entry->timer_flags & 1) == 0)
	{
		timer_system_time_new = iGetTimerSystemTime();
		timer_flags = timer_ee_entry->timer_flags;
		timer_ee_entry->timer_system_time = timer_system_time_new;
		timer_flags |= 1;
		timer_ee_entry->timer_flags = timer_flags;
		if ((timer_flags & 2) != 0)
		{
			InsertAlarm_ForTimer(timer_ee_entry);
			SetNextComp(timer_system_time_new);
		}
		return 0;
	}
	return 1;
}
#endif

#ifdef F_StartTimerCounter
s32 StartTimerCounter(s32 id)
{
	u32 oldintr;
	s32 ret;

	oldintr = DIntr();
	ret = iStartTimerCounter(id);
	if (oldintr != 0)
	{
		EIntr();
	}
	return ret;
}
#endif

#ifdef F_iStopTimerCounter
s32 iStopTimerCounter(s32 id)
{
	timer_ee_entry_struct *timer_ee_entry;
	u64 timer_system_time_new;
	u32 timer_flags;

	timer_ee_entry = (timer_ee_entry_struct *)((uintptr_t)id >> 10 << 6);
	if (timer_ee_entry == NULL || id < 0 || (id & 0x3FF) != timer_ee_entry->timer_id_check_guard)
	{
		return 0x80008002;
	}
	if (g_Timer.last_handled_timer_id == id)
	{
		return 0x80000010;
	}
	if ((timer_ee_entry->timer_flags & 1) != 0)
	{
		timer_system_time_new = iGetTimerSystemTime();
		timer_flags = timer_ee_entry->timer_flags;
		timer_ee_entry->timer_count += timer_system_time_new - timer_ee_entry->timer_system_time;
		timer_ee_entry->timer_flags = timer_flags & 0xFFFFFFFE;
		if ((timer_flags & 2) != 0)
		{
			UnlinkAlarm_ForTimer(timer_ee_entry);
			SetNextComp(timer_system_time_new);
		}
		return 1;
	}
	return 0;
}
#endif

#ifdef F_StopTimerCounter
s32 StopTimerCounter(s32 id)
{
	u32 oldintr;
	s32 ret;

	oldintr = DIntr();
	ret = iStopTimerCounter(id);
	if (oldintr != 0)
	{
		EIntr();
	}
	return ret;
}
#endif

#ifdef F_SetTimerCount
u64 SetTimerCount(s32 id, u64 timer_count)
{
	timer_ee_entry_struct *timer_ee_entry;
	u32 oldintr;
	u64 timer_count_old;
	u64 timer_system_time_new;
	u64 timer_system_time_old;

	timer_ee_entry = (timer_ee_entry_struct *)((uintptr_t)id >> 10 << 6);
	oldintr = DIntr();
	if (timer_ee_entry != NULL && id >= 0 && (id & 0x3FF) == timer_ee_entry->timer_id_check_guard && g_Timer.last_handled_timer_id != id)
	{
		timer_count_old = timer_ee_entry->timer_count;
		if ((timer_ee_entry->timer_flags & 1) != 0)
		{
			timer_system_time_new = iGetTimerSystemTime();
			timer_system_time_old = timer_ee_entry->timer_system_time;
			timer_ee_entry->timer_count = timer_count;
			timer_ee_entry->timer_system_time = timer_system_time_new;
			timer_count_old += timer_system_time_new - timer_system_time_old;
		}
		else
		{
			timer_ee_entry->timer_count = timer_count;
		}
		if (oldintr != 0)
		{
			EIntr();
		}
		return timer_count_old;
	}
	if (oldintr != 0)
	{
		EIntr();
	}
	return -1;
}
#endif

#ifdef F_iGetTimerBaseTime
u64 iGetTimerBaseTime(s32 id)
{
	timer_ee_entry_struct *timer_ee_entry;

	timer_ee_entry = (timer_ee_entry_struct *)((uintptr_t)id >> 10 << 6);
	if (timer_ee_entry == NULL || id < 0 || (id & 0x3FF) != timer_ee_entry->timer_id_check_guard)
	{
		return -1;
	}
	if ((timer_ee_entry->timer_flags & 1) != 0)
	{
		return timer_ee_entry->timer_system_time - timer_ee_entry->timer_count;
	}
	return 0;
}
#endif

#ifdef F_GetTimerBaseTime
u64 GetTimerBaseTime(s32 id)
{
	u32 oldintr;
	u64 ret;

	oldintr = DIntr();
	ret = iGetTimerBaseTime(id);
	if (oldintr != 0)
	{
		EIntr();
	}
	return ret;
}
#endif

#ifdef F_iGetTimerCount
u64 iGetTimerCount(s32 id)
{
	u64 ret;
	timer_ee_entry_struct *timer_ee_entry;

	timer_ee_entry = (timer_ee_entry_struct *)((uintptr_t)id >> 10 << 6);
	if (timer_ee_entry == NULL || id < 0 || (id & 0x3FF) != timer_ee_entry->timer_id_check_guard)
	{
		return -1;
	}
	ret = timer_ee_entry->timer_count;
	if ((timer_ee_entry->timer_flags & 1) != 0)
	{
		ret += iGetTimerSystemTime() - timer_ee_entry->timer_system_time;
	}
	return ret;
}
#endif

#ifdef F_GetTimerCount
u64 GetTimerCount(s32 id)
{
	u32 oldintr;
	u64 ret;

	oldintr = DIntr();
	ret = iGetTimerCount(id);
	if (oldintr != 0)
	{
		EIntr();
	}
	return ret;
}
#endif

#ifdef F_iSetTimerHandler
s32 iSetTimerHandler(s32 id, u64 scheduled_time, timer_alarm_handler_t callback_handler, void *arg)
{
	timer_ee_entry_struct *timer_ee_entry;
	u32 timer_flags;

	timer_ee_entry = (timer_ee_entry_struct *)((uintptr_t)id >> 10 << 6);
	if (timer_ee_entry == NULL || id < 0 || (id & 0x3FF) != timer_ee_entry->timer_id_check_guard)
	{
		return 0x80008002;
	}
	if (g_Timer.last_handled_timer_id == id)
	{
		return 0x80000010;
	}
	timer_flags = timer_ee_entry->timer_flags;
	if ((timer_flags & 2) != 0)
	{
		UnlinkAlarm_ForTimer(timer_ee_entry);
		timer_flags = timer_ee_entry->timer_flags;
	}
	timer_ee_entry->callback_handler = callback_handler;
	if (callback_handler != NULL)
	{
		timer_ee_entry->comparison_value = scheduled_time;
		timer_ee_entry->timer_flags = timer_flags | 2;
		timer_ee_entry->last_gp = GetGP();
		timer_ee_entry->callback_handler_argument = arg;
		if ((timer_ee_entry->timer_flags & 1) != 0)
		{
			InsertAlarm_ForTimer(timer_ee_entry);
		}
	}
	else
	{
		timer_ee_entry->timer_flags = timer_flags & 0xFFFFFFFD;
	}
	SetNextComp(iGetTimerSystemTime());
	return 0;
}
#endif

#ifdef F_SetTimerHandler
s32 SetTimerHandler(s32 id, u64 scheduled_time, timer_alarm_handler_t callback_handler, void *arg)
{
	u32 oldintr;
	s32 ret;

	oldintr = DIntr();
	ret = iSetTimerHandler(id, scheduled_time, callback_handler, arg);
	if (oldintr != 0)
	{
		EIntr();
	}
	return ret;
}
#endif

#ifdef F_TimerBusClock2USec
void TimerBusClock2USec(u64 clocks, u32 *seconds_result, u32 *microseconds_result)
{
	u32 seconds;

	seconds = (u32)(clocks / 147456000);
	if (seconds_result != NULL)
	{
		*seconds_result = seconds;
	}
	if (microseconds_result != NULL)
	{
		*microseconds_result = (u32)(1000000 * (clocks - (((s64)(s32)((147456000 * (u64)seconds) >> 32) << 32) | (u32)(147456000 * seconds))) / 147456000);
	}
}
#endif

#ifdef F_TimerUSec2BusClock
u64 TimerUSec2BusClock(u32 seconds, u32 microseconds)
{
	// XXX: check if this is correct
	return (u64)(147456000 * (u64)seconds) + (((s64)(s32)((147456000 * (u64)microseconds) >> 32) << 32) | (u64)(147456000 * (u64)microseconds)) / 1000000;
}
#endif

#ifdef F_TimerBusClock2Freq
float TimerBusClock2Freq(s64 clocks)
{
	float timer_frequency;

	if (clocks < 0)
	{
		timer_frequency = (float)((clocks & 1) | ((u64)clocks >> 1)) + (float)((clocks & 1) | ((u64)clocks >> 1));
	}
	else
	{
		timer_frequency = (float)clocks;
	}
	return 147456000.0 / timer_frequency;
}
#endif

#ifdef F_TimerFreq2BusClock
u64 TimerFreq2BusClock(float timer_frequency)
{
	return (u64)(float)(147456000.0 / timer_frequency);
}

#endif

#ifdef F_ForTimer_InitAlarm
void ForTimer_InitAlarm(void)
{
	g_pFreeAlarm = &g_AlarmBuf[0];
	for (u32 i = 0; i < ((sizeof(g_AlarmBuf) / sizeof(g_AlarmBuf[0])) - 1); i += 1)
	{
		g_AlarmBuf[i].next_alarm_ptr = &g_AlarmBuf[i + 1];
	}
	g_AlarmBuf[(sizeof(g_AlarmBuf) / sizeof(g_AlarmBuf[0])) - 1].next_alarm_ptr = NULL;
}
#endif

#ifdef F_AlarmHandler
u64 AlarmHandler(s32 alarm_id, u64 scheduled_time, u64 actual_time, void *arg, void *last_pc)
{
	u64 result;
	alarm_ee_entry_struct *alarm_ee_entry;

	alarm_ee_entry = (alarm_ee_entry_struct *)arg;
	result = alarm_ee_entry->callback_handler(
		((uintptr_t)arg << 4) | (u32)(alarm_id & 0xFE) | 1,
		scheduled_time,
		actual_time,
		alarm_ee_entry->callback_handler_argument, last_pc);
	if (result == 0)
	{
		{
			alarm_ee_entry_struct *tmp_swap;

			tmp_swap = g_pFreeAlarm;
			g_pFreeAlarm = alarm_ee_entry;
			alarm_ee_entry->next_alarm_ptr = tmp_swap;
		}
		return -1;
	}
	return result;
}
#endif

#ifdef F_SetTimerAlarm
s32 SetTimerAlarm(u64 clock_cycles, timer_alarm_handler_t callback_handler, void *arg)
{
	u32 oldintr;
	alarm_ee_entry_struct *alarm_ee_entry;
	s32 timer_counter;

	if (callback_handler == NULL)
	{
		return 0x80000016;
	}
	oldintr = DIntr();
	alarm_ee_entry = g_pFreeAlarm;
	if (g_pFreeAlarm != NULL)
	{
		g_pFreeAlarm = g_pFreeAlarm->next_alarm_ptr;
	}
	if (alarm_ee_entry == NULL)
	{
		if (oldintr != 0)
		{
			EIntr();
		}
		return 0x80008005;
	}
	timer_counter = AllocTimerCounter();
	if (timer_counter < 0)
	{
		{
			alarm_ee_entry_struct *tmp_swap;

			tmp_swap = g_pFreeAlarm;
			g_pFreeAlarm = alarm_ee_entry;
			alarm_ee_entry->next_alarm_ptr = tmp_swap;
		}
		alarm_ee_entry->timer_counter_id = 0;
		if (oldintr != 0)
		{
			EIntr();
		}
		return timer_counter;
	}
	alarm_ee_entry->callback_handler = callback_handler;
	alarm_ee_entry->callback_handler_argument = arg;
	alarm_ee_entry->timer_counter_id = timer_counter;
	SetTimerHandler(timer_counter, clock_cycles, AlarmHandler, alarm_ee_entry);
	StartTimerCounter(timer_counter);
	if (oldintr != 0)
	{
		EIntr();
	}
	return ((uintptr_t)alarm_ee_entry << 4) | (timer_counter & 0xFE) | 1;
}
#endif

#ifdef F_iSetTimerAlarm
s32 iSetTimerAlarm(u64 clock_cycles, timer_alarm_handler_t callback_handler, void *arg)
{
	s32 timer_counter;
	alarm_ee_entry_struct *alarm_ee_entry;

	if (callback_handler == NULL)
	{
		return 0x80000016;
	}
	alarm_ee_entry = g_pFreeAlarm;
	if (g_pFreeAlarm != NULL)
	{
		g_pFreeAlarm = g_pFreeAlarm->next_alarm_ptr;
	}
	if (alarm_ee_entry == NULL)
	{
		return 0x80008005;
	}
	timer_counter = iAllocTimerCounter();
	if (timer_counter < 0)
	{
		{
			alarm_ee_entry_struct *tmp_swap;

			tmp_swap = g_pFreeAlarm;
			g_pFreeAlarm = alarm_ee_entry;
			alarm_ee_entry->next_alarm_ptr = tmp_swap;
		}
		return timer_counter;
	}
	alarm_ee_entry->callback_handler = callback_handler;
	alarm_ee_entry->callback_handler_argument = arg;
	alarm_ee_entry->timer_counter_id = timer_counter;
	iSetTimerHandler(timer_counter, clock_cycles, AlarmHandler, alarm_ee_entry);
	iStartTimerCounter(timer_counter);
	return ((uintptr_t)alarm_ee_entry << 4) | (timer_counter & 0xFE) | 1;
}
#endif

#ifdef F_ReleaseTimerAlarm
s32 ReleaseTimerAlarm(s32 id)
{
	alarm_ee_entry_struct *alarm_ee_entry;
	u32 oldintr;

	alarm_ee_entry = (alarm_ee_entry_struct *)(((uintptr_t)id >> 8) << 4);
	oldintr = DIntr();
	if (alarm_ee_entry == NULL || id < 0 || (id & 0xFF) != (alarm_ee_entry->timer_counter_id & 0xFF))
	{
		if (oldintr != 0)
		{
			EIntr();
		}
		return 0x80008002;
	}
	FreeTimerCounter(alarm_ee_entry->timer_counter_id);
	{
		alarm_ee_entry_struct *tmp_swap;

		tmp_swap = g_pFreeAlarm;
		g_pFreeAlarm = alarm_ee_entry;
		alarm_ee_entry->next_alarm_ptr = tmp_swap;
	}
	alarm_ee_entry->timer_counter_id = 0;
	if (oldintr != 0)
	{
		EIntr();
	}
	return 0;
}
#endif

#ifdef F_iReleaseTimerAlarm
s32 iReleaseTimerAlarm(s32 id)
{
	alarm_ee_entry_struct *alarm_ee_entry;
	s32 ret;

	alarm_ee_entry = (alarm_ee_entry_struct *)(((uintptr_t)id >> 8) << 4);
	if (alarm_ee_entry == NULL || id < 0 || (id & 0xFF) != (alarm_ee_entry->timer_counter_id & 0xFF))
	{
		return 0x80008002;
	}
	ret = iFreeTimerCounter(alarm_ee_entry->timer_counter_id);
	if (ret == 0)
	{
		alarm_ee_entry_struct *tmp_swap;

		tmp_swap = g_pFreeAlarm;
		g_pFreeAlarm = alarm_ee_entry;
		alarm_ee_entry->next_alarm_ptr = tmp_swap;
	}
	return ret;
}
#endif

#ifdef F_DelayThreadHandler_callback
u64 DelayThreadHandler_callback(s32 alarm_id, u64 scheduled_time, u64 actual_time, void *arg, void *pc_value)
{
	iSignalSema((s32)arg);
	EE_SYNCL();
	__asm__ __volatile__ ("ei");
	return 0;
}
#endif

#ifdef F_DelayThread
s32 DelayThread(s32 microseconds)
{
	u32 eie;
	s32 sema_id;
	s32 timer_alarm_id;
	ee_sema_t sema;

	__asm__ __volatile__ ("mfc0\t%0, $12" : "=r" (eie));
	if ((eie & 0x10000) == 0)
	{
		return 0x80008008;
	}
	sema.max_count = 1;
	sema.option = (u32)"DelayThread";
	sema.init_count = 0;
	sema_id = CreateSema(&sema);
	if (sema_id < 0)
	{
		return 0x80008003;
	}
	timer_alarm_id = SetTimerAlarm(TimerUSec2BusClock(0, microseconds), DelayThreadHandler_callback, (void *)sema_id);
	if (timer_alarm_id >= 0)
	{
		WaitSema(sema_id);
		DeleteSema(sema_id);
		return 0LL;
	}
	else
	{
		DeleteSema(sema_id);
		return timer_alarm_id;
	}
}
#endif


#ifdef F_cpu_ticks
u32 cpu_ticks(void)
{
    u32 out;

    asm("mfc0\t%0, $9\n"
        : "=r"(out));
    return out;
}
#endif
