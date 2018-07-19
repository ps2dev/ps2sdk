/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "tamtypes.h"
#include "timer.h"
#include "ee_regs.h"
#include "kernel.h"

#include "internal.h"

#define USER_MODE_DISPATCHER	0x00082000 //Call the replacement dispatcher (the original dispatcher function is located at 0x00081fe0).

#define INTC_TIM3 12
#define T3_COUNT_W ((vu32*)0xB0001800)
#define T3_MODE_W ((vu32*)0xB0001810)
#define T3_COMP_W ((vu32*)0xB0001820)

static int AlarmCount = 0;
static u64 AlarmStatus = 0;

struct alarm{
	u16 target;		//0x00
	u16 time;		//0x02
	int id;			//0x04
	void *callback;		//0x08
	void *common;		//0x0c
	void *gp;		//0x10
};

static struct alarm alarms[MAX_ALARMS];

//Function prototypes
static u32 CalculateTimeDiff(u32 t1, u32 t2);
static int InsertAlarm(u32 now, u32 target);
static s32 SetAlarmInternal(u16 time, void (*callback)(s32 dispatch_id, u16 time, void *common), void *common);
static void SetupTIM3(u16 ticks);

static u32 CalculateTimeDiff(u32 t1, u32 t2)
{
	return(t2 < t1 ? (0x10000 | t2) : t2);
}

static int InsertAlarm(u32 now, u32 target)
{
	int pos, i;

	pos = 0;
	for(pos = 0; pos < AlarmCount; pos++)
	{
		if(target < CalculateTimeDiff(now, alarms[pos].time))
		{	//Spot found. If it is not at the end of the list, move back the list.
			for(i = AlarmCount - 1; i >= pos; i--)
				alarms[i+1] = alarms[i];

			break;
		}
	}

	return pos;
}

static s32 SetAlarmInternal(u16 time, void (*callback)(s32 dispatch_id, u16 time, void *common), void *common)
{
	void *gp;
	u32 target, now;
	struct alarm *alarm;
	int i, alarmID, pos;

	now = *T3_COUNT_W;
	target = now + time;

	if(AlarmCount >= MAX_ALARMS)
		return -1;

	alarmID = -1;
	for(i = 0; i < MAX_ALARMS; i++)
	{
		if(((AlarmStatus >> i) & 1) == 0)
		{
			AlarmStatus |= (1 << i);
			alarmID = i;
			break;
		}
	}

	if(alarmID < 0)
		return alarmID;

	asm volatile("move %0, $gp\n" :"=r"(gp):);
	pos = InsertAlarm(now, target);
	alarm = &alarms[pos];
	alarm->time = time;
	alarm->target = (u16)target;
	alarm->id = alarmID;
	alarm->gp = gp;
	alarm->callback = callback;
	alarm->common = common;
	AlarmCount++;
	SetupTIM3(alarms[0].target);

	return alarm->id;
}

s32 ReleaseAlarm(s32 id)
{
	u16 time;
	s32 result;
	int i, j;

	result = -1;
	for(i = 0; i < AlarmCount; i++)
	{
		if(alarms[i].id == id)
		{
			if(alarms[i].target == *T3_COMP_W)
			{
				if(*R_EE_I_STAT & (1 << INTC_TIM3))	//Cannot release alarm that has already triggered.
					return -1;
			}

			time = alarms[i].time;
			//Move forward list.
			for(j = i; j < AlarmCount - 1; j++)
				alarms[j] = alarms[j+1];

			--AlarmCount;
			AlarmStatus &= ~(1 << id);

			if(i == 0)	//If the first alarm was released, update timer comparator.
				SetupTIM3(alarms[0].target);

			if(AlarmCount == 0)	//Stop timer if there are no alarms left.
				*T3_MODE_W = Tn_MODE(3,0,0,0,0,1,0,0,0,0);

			result = (CalculateTimeDiff(time, *T3_COUNT_W) - time);
		}
	}

	EE_SYNC();
	return result;
}

s32 SetAlarm(u16 time, void (*callback)(s32 dispatch_id, u16 time, void *common), void *common)
{
	s32 result;

	result = SetAlarmInternal(time, callback, common);
	EE_SYNC();

	return result;
}

static void SetupTIM3(u16 ticks)
{
	*T3_COMP_W = ticks;
	EE_SYNC();
	*T3_MODE_W = Tn_MODE(3,0,0,0,0,1,1,0,1,0);
}

//INTC 12 (TIM3) handler
void Intc12Handler(void)
{
	struct alarm alarm;
	void *gp;
	int i;

	for(i = 0; i < AlarmCount; i++)
	{	//Attempt to find another alarm request that has a different target. Update TIM3's comparator.
		if(alarms[i].target != alarms[0].target)
		{
			SetupTIM3(alarms[i].target);
			break;
		}
	}

	do{
		alarm = alarms[0];
		AlarmCount--;
		for(i = 0; i < AlarmCount; i++)
			alarms[i] = alarms[i+1];

		gp = ChangeGP(alarm.gp);
		AlarmStatus &= ~(1 << alarm.id);
		InvokeUserModeCallback((void*)USER_MODE_DISPATCHER, alarm.callback, alarm.id, alarm.target, alarm.common);
		SetGP(gp);

		if(AlarmCount <= 0)
			break;
	}while(alarms[0].target == alarm.target);

	if(AlarmCount <= 0)	//If there are no further alarms, disable CMPE.
		*T3_MODE_W = Tn_MODE(3,0,0,0,0,1,0,0,1,0);
	else
		SetupTIM3(alarms[0].target);

	ExitHandler();
}
