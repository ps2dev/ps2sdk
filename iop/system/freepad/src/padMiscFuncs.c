/*
 * freepad - IOP pad driver
 * Copyright (c) 2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include "types.h"
#include "freepad.h"
#include "stdio.h"
#include "xsio2man.h"
#include "thevent.h"
#include "thbase.h"
#include "vblank.h"

void DeleteThreadsEventFlag(vblankData_t *s)
{
	DeleteThread(s->tid_1);
	DeleteThread(s->tid_2);
	DeleteEventFlag(s->eventflag);
}

s32 padEnd()
{
	if(freepad_init != 0)
	{
		u32 port,slot;

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

		freepad_init = 0;
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

	if( mode > padState[port][slot].numModes )
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
	if( (padState[port][slot].currentTask != TASK_UPDATE_PAD) || ( padState[port][slot].modeConfig < MODE_CONFIG_READY) )
		return 0;

	if(act == -1) return padState[port][slot].numActuators;

	if( act < padState[port][slot].numActuators )
	{
		u8 *actData = (u8*)&padState[port][slot].actData[act];

		if( val == 1 ) return actData[0];
		if( val == 2 ) return actData[1];
		if( val == 3 ) return actData[2];
		if( val == 4 ) return actData[3];

	}

	return -1;
}

s32 padInfoComb(u32 port, u32 slot, s32 val1, u32 val2)
{
	if( (padState[port][slot].currentTask != TASK_UPDATE_PAD) || ( padState[port][slot].modeConfig < MODE_CONFIG_READY) )
		return 0;

	if(val1 == -1) return padState[port][slot].numActComb;

	if( val1 < padState[port][slot].numActComb)
	{
		u8 *combData = (u8*)&padState[port][slot].combData[val1];

		if(val2 == -1) return combData[0];
		if(val2 == 0)  return combData[1];
		if(val2 == 1)  return combData[2];
		if(val2 == 2)  return combData[3];
	}

	return -1;
}

s32 padInfoMode(u32 port, u32 slot, s32 val1, u32 val2)
{
	if((padState[port][slot].currentTask != TASK_UPDATE_PAD))
		return 0;

	if( padState[port][slot].reqState != PAD_RSTAT_BUSY)
	{
		if(val1 == 2)
		{
			if( padState[port][slot].currentTask == padState[port][slot].modeConfig)
				return 0;
			else
				return padState[port][slot].modeTable[padState[port][slot].modeCurOffs];
		}

		if(val1 < 3)
		{
			if(val1 == 1)
			{
				if( padState[port][slot].modeCurId != 0xF3)
					return (0xF3 >> 4);
				else
					return 0;
			}
			else
				return -1;
		}

		if(val1 == 3)
		{
			if(padState[port][slot].modeConfig == 1)
				return 0;
			else
				return padState[port][slot].modeCurOffs;
		}		


		if(val1 == 4)
		{
			u16* mode = (u16*)padState[port][slot].modeTable;

			if(padState[port][slot].modeConfig == 1)
			{
				return 0;
			}
			else
			{
				if(val2 == -1)
				{
					return padState[port][slot].numModes;
				}
				else
				{
					if(val2 < padState[port][slot].numModes)
						return 0;
					else
						return mode[val2];
				}
			}
		}



	}

	return 0;
}

u32 SetActDirect3(u32 port, u32 slot)
{
	u32 ret = 0;
	u32 p = 0, s = 0;	

	while(p < padGetPortMax())
	{
		s = 0;

		while(s < padGetSlotMax(port))
		{
			if( (p == port) && (s == slot) && (openSlots[port] != 0))
			{
				if(padState[port][slot].modeCurId != 0)
				{
					if(padState[port][slot].ee_actDirectSize >= 0)
					{
						u32 i;

						for(i=0; i < padState[port][slot].ee_actDirectSize; i++)
						{
							if((padState[port][slot].ee_actAlignData[i] != 0xFF)
								&& (padState[port][slot].ee_actDirectData[i] != 0))
							{
								u8 *act = (u8*)padState[port][slot].actData;
								ret += act[3+i];
							}	
						}
					}
				}
			}
			s++;
		}

		p++;
	}


	return ret;
}

u32 SetActDirect2(u32 port, u32 slot, u8 *actData)
{
	u32 i;
	u32 res = SetActDirect3(port, slot);

	for(i=0; i < 6; i++)
	{
		u32 a = padState[port][slot].ee_actAlignData[i];

		if(a != 0xFF)
		{
			if( actData[a] != 0 )
			{
				u8 *act = (u8*)padState[port][slot].actData;

				res += act[3+i];

				if(res >= 0x3D) actData[i] = 0;
			}
		}
	}

	return 0;
}

u32 padSetActDirect(u32 port, u32 slot, u8 *actData)
{
	if(padState[port][slot].currentTask == TASK_UPDATE_PAD)
	{
		if( SetActDirect2(port, slot, actData) != 0)
		{
			// Really kprintf
			M_PRINTF("Over consumpt Max\n");
		}
		else
		{
		
			u32 i;
	
			for(i=0; i < 6; i++)
				padState[port][slot].ee_actDirectData[i] = actData[i];
		
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
		u32 i;

		for(i=0; i < 6; i++)
			padState[port][slot].ee_actAlignData[i] = actData[i];

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
		u32 i;

		info = (info << 6) | 0x3F;

		padState[port][slot].buttonInfo[0] = (u8)(info);
		padState[port][slot].buttonInfo[1] = (u8)(info >> 8);
		padState[port][slot].buttonInfo[2] = (u8)(info >> 16);
		padState[port][slot].buttonInfo[3] = (u8)(info >> 24);

		for(i=0; i < 11; i++)
			padState[port][slot].vrefParam[i] = 0x2;

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
		u32 i;	

		for(i=0; i < 11; i++)
			padState[port][slot].vrefParam[i] = vparam[i];

		padState[port][slot].reqState = PAD_RSTAT_BUSY;
		padState[port][slot].runTask = TASK_SET_VREF_PARAM;
		padState[port][slot].taskTid = padState[port][slot].setvrefparamTid;


		return 1;
	}	

	return 0;
}

u32 padGetPortMax()
{
	return 2;
}

u32 padGetSlotMax(u32 port)
{
	return sio2_mtap_get_slot_max(port);
}

u32 padGetModVersion()
{
	return version;
}

u32 padGetInBuffer(u32 port, u32 slot, u8 *buf)
{
	u32 i;

	for(i=0; i < 32 ; i++)
		buf[i] = padState[port][slot].inbuffer[i];

	return 32;
}

u32 padGetModeConfig(u32 port, u32 slot)
{
	return padState[port][slot].modeConfig;
}
