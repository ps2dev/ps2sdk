/*
 * Copyright (c) 2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

/**
 * @file
 * IOP pad driver
 */

#include "types.h"
#include "freepad.h"
#include "stdio.h"
#include "xsio2man.h"
#include "sio2Cmds.h"
#include "sysmem.h"
#include "thevent.h"
#include "thbase.h"
#include "vblank.h"
#include "irx.h"

// Global variables
extern struct irx_id _irx_id;

extern padState_t padState[2][4];
extern u32 openSlots[2];
extern vblankData_t vblankData;
extern int padman_init;

void DeleteThreadsEventFlag(vblankData_t *s)
{
	DeleteThread(s->tid_1);
	DeleteThread(s->tid_2);
	DeleteEventFlag(s->eventflag);
}

s32 padEnd(void)
{
	if(padman_init != 0)
	{
		int port,slot;

		for(port = 0; port < 2; port++)
			for(slot=0; slot < 4; slot++)
			{
				if((openSlots[port] >> slot) & 0x1)
					padPortClose(port, slot, 1);
			}

		SetEventFlag(vblankData.eventflag, EF_EXIT_THREAD);

		vblankData.padEnd = 1;

		WaitEventFlag(vblankData.eventflag, EF_VB_WAIT_THREAD_EXIT, 0x11, 0);

		DeleteThreadsEventFlag( &vblankData );

		while(ReleaseVblankHandler(0, (void*)VblankStart) != 0)
			M_PRINTF("Release VB_START failed.\n");

		while(ReleaseVblankHandler(1, VblankEnd) != 0)
			M_PRINTF("Release VB_END failed.\n");

		padman_init = 0;
	}

	return 1;
}

s32 padPortClose(s32 port, s32 slot, s32 wait)
{
	if(((openSlots[port] >> slot) & 1) == 0)
	{
		M_PRINTF("padPortClose: Port %i Slot %i is not open.\n", (int)port, (int)slot);
		return 0;
	}

	if(padState[port][slot].reqState == PAD_RSTAT_BUSY)
	{
		M_PRINTF("padPortClose: Port %i Slot %i request failed.\n", (int)port, (int)slot);
		return 0;
	}

	padState[port][slot].runTask = TASK_PORT_CLOSE;
	padState[port][slot].taskTid = 0;
	padState[port][slot].reqState = PAD_RSTAT_BUSY;

	if(wait)
	{
		u32 resbits;

		WaitEventFlag(padState[port][slot].eventflag, EF_PORT_CLOSE, 0x10, &resbits);
		DeleteEventFlag(padState[port][slot].eventflag);
		padState[port][slot].eventflag = 0;
	}

	return 1;
}

u32 padSetMainMode(u32 port, u32 slot, u32 mode, u32 lock)
{
	if( (padState[port][slot].currentTask != TASK_UPDATE_PAD) || ( padState[port][slot].modeConfig < MODE_CONFIG_READY) )
		return 0;

	if( mode >= padState[port][slot].numModes )
		return 0;

	if( padState[port][slot].reqState != PAD_RSTAT_BUSY)
	{
		padState[port][slot].mode = mode;
		padState[port][slot].lock = lock;
		padState[port][slot].runTask = TASK_SET_MAIN_MODE;
		padState[port][slot].reqState = PAD_RSTAT_BUSY;
		padState[port][slot].taskTid = padState[port][slot].setmainmodeTid;

		return 1;
	}

	return 0;
}

s32 padInfoAct(u32 port, u32 slot, s32 act, u32 val)
{
	if(padState[port][slot].currentTask != TASK_UPDATE_PAD)
		return 0;

	if(padState[port][slot].modeConfig < MODE_CONFIG_READY)
		return -1;

	if(act == -1) return padState[port][slot].numActuators;

	if( act < padState[port][slot].numActuators )
	{
		u8 *actData = padState[port][slot].actData.data[act];

		switch(val)
		{
			case 1:
				return actData[0];
			case 2:
				return actData[1];
			case 3:
				return actData[2];
			case 4:
				return actData[3];
		}
	}

	return -1;
}

s32 padInfoComb(u32 port, u32 slot, s32 listno, u32 offs)
{
	if(padState[port][slot].currentTask != TASK_UPDATE_PAD)
		return 0;

	if(padState[port][slot].modeConfig < MODE_CONFIG_READY)
		return -1;

	if(listno == -1) return padState[port][slot].numActComb;

	if( listno < padState[port][slot].numActComb)
	{
		u8 *combData = padState[port][slot].combData.data[listno];

		switch(offs)
		{
			case -1:
				return combData[0];
			case 0:
				return combData[1];
			case 1:
				return combData[2];
			case 2:
				return combData[3];
		}
	}

	return -1;
}

s32 padInfoMode(u32 port, u32 slot, s32 term, u32 offs)
{
	if((padState[port][slot].currentTask != TASK_UPDATE_PAD) || (padState[port][slot].reqState == PAD_RSTAT_BUSY))
		return 0;

	switch(term)
	{
		case 2:
			if(padState[port][slot].modeConfig == MODE_CONFIG_QUERY_PAD)
				return 0;
			else
				return padState[port][slot].modeTable.data[padState[port][slot].modeCurOffs];
		case 1:
			if( padState[port][slot].modeCurId != PAD_ID_CONFIG)
				return (PAD_ID_HI(PAD_ID_CONFIG));
			else
				return 0;
		case 3:
			if(padState[port][slot].modeConfig == MODE_CONFIG_QUERY_PAD)
				return 0;
			else
				return padState[port][slot].modeCurOffs;
		case 4:
			if(padState[port][slot].modeConfig == MODE_CONFIG_QUERY_PAD)
				return 0;
			else
			{
				if(offs == -1)
				{
					return padState[port][slot].numModes;
				}
				else
				{
					u16* mode = padState[port][slot].modeTable.data;

					if(offs < padState[port][slot].numModes)
						return mode[offs];
					else
						return 0;
				}
			}
	}

	return -1;
}

u32 ActDirectTotal(u32 port, u32 slot)
{
	u32 ret = 0;
	u32 p, s;

	for(p = 0; p < padGetPortMax(); p++)
	{
		for(s = 0; s < padGetSlotMax(port); s++)
		{
			if( (p == port) && (s == slot) && (((openSlots[port] >> slot) & 1) != 0))
			{
				if(padState[port][slot].modeCurId != 0)
				{
					int i;

					for(i=0; i < padState[port][slot].ee_actDirectSize; i++)
					{
						if((padState[port][slot].ee_actAlignData.data[i] != 0xFF)
							&& (padState[port][slot].ee_actDirectData.data[i] != 0))
						{
							u8 *act = padState[port][slot].actData.data[padState[port][slot].ee_actAlignData.data[i]];
							ret += act[3];
						}
					}
				}
			}
		}
	}

	return ret;
}

u32 CheckAirDirectTotal(u32 port, u32 slot, u8 *actData)
{
	int i;
	u32 total = ActDirectTotal(port, slot);

	for(i=0; i < 6; i++)
	{
		u32 a = padState[port][slot].ee_actAlignData.data[i];

		if(a != 0xFF)
		{
			if( actData[a] != 0 )
			{
				u8 *act = padState[port][slot].actData.data[a];

				total += act[3];

				if(total >= 0x3D)
				{
					M_KPRINTF("Over Consumpt Max 600mA[%d][%d]\n", (int)port, (int)slot);
					actData[a] = 0;
				}
			}
		}
	}

	return 1;
}

u32 padSetActDirect(u32 port, u32 slot, u8 *actData)
{
	if(padState[port][slot].currentTask == TASK_UPDATE_PAD)
	{
		if( CheckAirDirectTotal(port, slot, actData) == 0)
		{	//This doesn't seem to be ever used.
			M_KPRINTF("Over consumpt Max [%d][%d]\n", (int)port, (int)slot);
		}
		else
		{
			int i;

			for(i=0; i < 6; i++)
				padState[port][slot].ee_actDirectData.data[i] = actData[i];

			padState[port][slot].ee_actDirectSize = 6;

			return 1;
		}
	}

	return 0;
}

u32 padSetActAlign(u32 port, u32 slot, u8 *actData)
{
	if( (padState[port][slot].currentTask != TASK_UPDATE_PAD) || ( padState[port][slot].modeConfig < MODE_CONFIG_READY) )
		return 0;

	if( padState[port][slot].reqState != PAD_RSTAT_BUSY)
	{
		int i;

		for(i=0; i < 6; i++)
			padState[port][slot].ee_actAlignData.data[i] = actData[i];

		padState[port][slot].reqState = PAD_RSTAT_BUSY;
		padState[port][slot].runTask = TASK_SET_ACT_ALIGN;
		padState[port][slot].taskTid = padState[port][slot].setactalignTid;

		return 1;
	}

	return 0;
}

u32 padGetButtonMask(u32 port, u32 slot)
{
	u32 ret = 0;

	if( (padState[port][slot].currentTask != TASK_UPDATE_PAD) || ( padState[port][slot].modeConfig < MODE_CONFIG_READY) )
		return ret;

	if( padState[port][slot].model < 2)
		return ret;

	ret  = (u32)padState[port][slot].buttonMask[0];
	ret |= (u32)padState[port][slot].buttonMask[1] << 8;
	ret |= (u32)padState[port][slot].buttonMask[2] << 16;
	ret |= (u32)padState[port][slot].buttonMask[3] << 24;

	return ret;
}

u32 padSetButtonInfo(u32 port, u32 slot, u32 info)
{
	if( (padState[port][slot].currentTask != TASK_UPDATE_PAD) || ( padState[port][slot].modeConfig < MODE_CONFIG_READY) )
		return 0;

	if( padState[port][slot].model < 2)
		return 0;

	if( padState[port][slot].reqState != PAD_RSTAT_BUSY)
	{
		int i;

		info = (info << 6) | 0x3F;

		padState[port][slot].buttonInfo[0] = (u8)(info);
		padState[port][slot].buttonInfo[1] = (u8)(info >> 8);
		padState[port][slot].buttonInfo[2] = (u8)(info >> 16);
		padState[port][slot].buttonInfo[3] = (u8)(info >> 24);

		for(i=0; i < 11; i++)
			padState[port][slot].modeParam[i] = 0x2;

		padState[port][slot].reqState = PAD_RSTAT_BUSY;
		padState[port][slot].runTask = TASK_SET_BUTTON_INFO;
		padState[port][slot].taskTid = padState[port][slot].setbuttoninfoTid;

		return 1;
	}


	return 0;
}

u32 padSetVrefParam(u32 port, u32 slot, u8 *vparam)
{
	if( (padState[port][slot].currentTask != TASK_UPDATE_PAD) || ( padState[port][slot].modeConfig < MODE_CONFIG_READY) )
		return 0;

	if( padState[port][slot].model < 2)
		return 0;

	if( padState[port][slot].reqState != PAD_RSTAT_BUSY)
	{
		int i;

		for(i=0; i < 12; i++)
			padState[port][slot].modeParam[i] = vparam[i];

		padState[port][slot].runTask = TASK_SET_VREF_PARAM;
		padState[port][slot].reqState = PAD_RSTAT_BUSY;
		padState[port][slot].taskTid = padState[port][slot].setvrefparamTid;

		return 1;
	}

	return 0;
}

u32 padGetPortMax(void)
{
	return 2;
}

u32 padGetSlotMax(u32 port)
{
	return sio2_mtap_get_slot_max(port);
}

u32 padGetModVersion(void)
{
	return _irx_id.v;
}

u32 padGetInBuffer(u32 port, u32 slot, u8 *buf)
{
	int i;

	for(i=0; i < 32 ; i++)
		buf[i] = padState[port][slot].inbuffer[i];

	return 32;
}

u32 padGetModeConfig(u32 port, u32 slot)
{
	return padState[port][slot].modeConfig;
}
