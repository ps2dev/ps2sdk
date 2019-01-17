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
#include "thevent.h"
#include "thbase.h"
#include "sio2Cmds.h"
#include "padCmds.h"
#include "padData.h"

// Global variables
extern padState_t padState[2][4];
extern u32 openSlots[2];

extern int thpri_lo;
extern int thpri_hi;

static void UpdatePadThread(void *arg)
{
	iop_thread_info_t tstatus;
	padState_t *pstate;
	u32 res = 0;

	ReferThreadStatus(TH_SELF, &tstatus);
	pstate = (padState_t*)tstatus.option;

	while(1)
	{
		WaitClearEvent(pstate->eventflag, EF_UPDATE_PAD, WEF_AND|WEF_CLEAR, NULL);

		// Check if able to read button data from pad
		if( ReadData(pstate) == 1)
		{
			res = 0;

			if( (pstate->outbuffer[2] != 0x5a) || ( pstate->outbuffer[1] != pstate->modeCurId) || (pstate->outbuffer[1] == 0xf3) )
			{
				pstate->buttonDataReady = 0;
				pstate->state = PAD_STATE_EXECCMD ;
				pstate->currentTask = TASK_QUERY_PAD;
				StartThread(pstate->querypadTid, NULL);
			}
			else
			{
				u32 i;

				for(i=0; i < 32; i++)
					pstate->buttonStatus[i] = pstate->outbuffer[i];

				pstate->buttonDataReady = 1;

				if(pstate->modeConfig == MODE_CONFIG_QUERY_PAD)
					pstate->state = PAD_STATE_FINDCTP1;
				else
					pstate->state = PAD_STATE_STABLE ;

				if( (pstate->reqState == PAD_RSTAT_BUSY) && (pstate->runTask == TASK_NONE))
					pstate->reqState = PAD_RSTAT_COMPLETE;
			}
		}
		else
		{
			pstate->buttonDataReady = 0;
			pstate->state = PAD_STATE_ERROR;
			pstate->findPadRetries++;

			res += pdGetError( pstate->port, pstate->slot );

			if(res >= 10)
			{
				pstate->currentTask = TASK_QUERY_PAD;
				StartThread(pstate->querypadTid, NULL);
			}
		}
	}
}

static void QueryPadThread(void *arg)
{
	iop_thread_info_t tinfo;
	padState_t *pstate;
	u32 i;
	u8 count;
	u32 res;
	u32 modeCurId = 0xFF;

	ReferThreadStatus(TH_SELF, &tinfo);

	pstate = (padState_t*)tinfo.option;

	pstate->modeConfig = 0;
	pstate->modeCurId = 0;
	pstate->model = 0;
	pstate->numModes = 0;
	pstate->numActuators = 0;
	pstate->numActComb = 0;

	for(i=0; i < 4; i++)
	{
		pstate->buttonInfo[i] = 0;
		pstate->buttonMask[i] = 0;
	}

	pstate->buttonDataReady = 0;
	pstate->ee_actDirectSize = 0;
	pstate->findPadRetries = 0;
	pstate->modeCurId = 0;


	D_PRINTF("QueryPadThread: Checking for pad (%i,%i)\n", pstate->port, pstate->slot);

	do
	{
		WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, WEF_AND|WEF_CLEAR, NULL);

		res = PadIsSupported(pstate);

		if(res == 1)
		{
			if(pstate->modeCurId != 0xFF)
			{
				modeCurId = pstate->modeCurId;
				pstate->modeCurId = 0;
				pstate->state = PAD_STATE_EXECCMD;
			}
			else
			{
				pstate->state = PAD_STATE_DISCONN;
				pstate->disconnected = 1;
			}
		}
		else
		{
			pstate->state = PAD_STATE_DISCONN;
			pstate->disconnected = 1;
		}
	}
	while( (res != 1) || (pstate->modeCurId == 0xFF) );


	if(pstate->disconnected != 0)
	{
		for(i=0; i < 6; i++)
			pstate->ee_actAlignData.data[i] = 0xFF;
	}

	D_PRINTF("Found pad (%i,%i) - modeCurId: 0x%x\n", (int)pstate->port, (int)pstate->slot, (int)modeCurId);

	res = 0;

	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, WEF_AND|WEF_CLEAR, NULL);

		res = EnterConfigMode(modeCurId, pstate);

		if(res == 1) pstate->modeConfig = MODE_CONFIG_READY;
	}

	if( res != 1 )
	{
		D_PRINTF("EnterConfigMode (%i, %i): Failed\n", (int)pstate->port, (int)pstate->slot);

		pstate->modeConfig = MODE_CONFIG_QUERY_PAD;
		pstate->modeCurId = modeCurId;
		pstate->currentTask = TASK_UPDATE_PAD;

		ExitThread();
	}

	D_PRINTF("EnterConfigMode (%i, %i): Success\n", (int)pstate->port, (int)pstate->slot);

	for(i=0,res=0; res != 1; i++)
	{
		WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, WEF_AND|WEF_CLEAR, NULL);
		res = QueryModel(pstate);

		if((res != 1) && (i >= 10))
		{
			D_PRINTF("QueryModel (%i, %i): Failed\n",(int)pstate->port, (int)pstate->slot);

			pstate->modeCurId = 0;
			pstate->currentTask = TASK_UPDATE_PAD;
			ExitThread();
		}
	}

	D_PRINTF("QueryModel (%i, %i): Success\n", (int)pstate->port, (int)pstate->slot);

	if( (pstate->disconnected == 1) && (pstate->modeCurOffs != 0))
	{
		pstate->mode = 0;
		pstate->lock = 0;

		for(i=0,res=0; res != 1; i++)
		{
			WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, WEF_AND|WEF_CLEAR, NULL);

			res = SetMainMode(pstate);

			if(res == 1)
			{
				D_PRINTF("SetMainMode (%i,%i): Success\n", (int)pstate->port, (int)pstate->slot);
				D_PRINTF("QueryPadThread: Done (%i,%i)\n", (int)pstate->port, (int)pstate->slot);

				pstate->modeCurId = 0;
				pstate->currentTask = TASK_UPDATE_PAD;
				pstate->disconnected = 0;
				ExitThread();
			}
			else
			{
				if(i >= 10)
				{
					D_PRINTF("SetMainMode (%i, %i): Failed\n", (int)pstate->port, (int)pstate->slot);

					pstate->reqState = PAD_RSTAT_FAILED;
					pstate->currentTask = TASK_UPDATE_PAD;
					ExitThread();
				}
			}
		}
	}

	for(count=0; count < pstate->numActuators; count++)
	{
		for(i=0,res=0; res != 1; i++)
		{
			WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, WEF_AND|WEF_CLEAR, NULL);

			res = QueryAct(count, pstate);

			if((res != 1) && (i >= 10))
			{
				D_PRINTF("QueryAct (%i,%i): Failed\n", (int)pstate->port, (int)pstate->slot);

				pstate->modeCurId = 0;
				pstate->currentTask = TASK_UPDATE_PAD;
				ExitThread();
			}
		}
	}

	D_PRINTF("QueryAct (%i,%i): Success\n", (int)pstate->port, (int)pstate->slot);

	for(count=0; count < pstate->numActComb; count++)
	{
		for(i=0,res=0; res != 1; i++)
		{
			WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, WEF_AND|WEF_CLEAR, NULL);

			res = QueryComb(count, pstate);

			if((res != 1) && (i >= 10))
			{
				D_PRINTF("QueryComb (%i,%i): Failed\n", (int)pstate->port, (int)pstate->slot);

				pstate->modeCurId = 0;
				pstate->currentTask = TASK_UPDATE_PAD;
				ExitThread();
			}
		}
	}

	D_PRINTF("QueryComb (%i,%i): Success\n", (int)pstate->port, (int)pstate->slot);

	for(count=0; count < pstate->numModes; count++)
	{
		for(i=0,res=0; res != 1; i++)
		{
			WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, WEF_AND|WEF_CLEAR, NULL);

			res = QueryMode(count, pstate);

			if((res != 1) && (i >= 10))
			{
				D_PRINTF("QueryMode (%i,%i): Failed\n", (int)pstate->port, (int)pstate->slot);

				pstate->modeCurId = 0;
				pstate->currentTask = TASK_UPDATE_PAD;
				ExitThread();
			}
		}
	}

	D_PRINTF("QueryMode (%i,%i): Success\n", (int)pstate->port, (int)pstate->slot);

	for(i=0,res=0; res != 1; i++)
	{
		WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, WEF_AND|WEF_CLEAR, NULL);

		res = SetActAlign(pstate);

		if((res != 1) && (i >= 10))
		{
			D_PRINTF("SetActAlign (%i,%i): Failed\n", (int)pstate->port, (int)pstate->slot);

			pstate->reqState = PAD_RSTAT_FAILED;
			pstate->currentTask = TASK_UPDATE_PAD;
			ExitThread();
		}
	}

	D_PRINTF("SetActAlign (%i,%i): Success\n", (int)pstate->port, (int)pstate->slot);

	if(((pstate->modeConfig & 0xFF) == 0x02) &&
		((pstate->model & 0x02) == 0x02))
	{
		for(i=0,res=0; res != 1; i++)
		{
			WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, WEF_AND|WEF_CLEAR, NULL);

			res = QueryButtonMask(pstate);

			if((res != 1) && (i >= 10))
			{
				D_PRINTF("QueryButtonMask (%i,%i): Failed\n", (int)pstate->port, (int)pstate->slot);

				pstate->modeCurId = 0;
				pstate->currentTask = TASK_UPDATE_PAD;
				ExitThread();
			}
		}

		D_PRINTF("QueryButtonMask (%i,%i): Success\n", (int)pstate->port, (int)pstate->slot);
	}

	for(i=0,res=0; res != 1; i++)
	{
		WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, WEF_AND|WEF_CLEAR, NULL);

		res = ExitConfigMode(pstate);

		if((res != 1) && (i >= 10))
		{
			D_PRINTF("ExitConfigMode (%i,%i): Failed\n", (int)pstate->port, (int)pstate->slot);

			pstate->modeCurId = 0;
			pstate->currentTask = TASK_UPDATE_PAD;
			ExitThread();
		}
	}

	D_PRINTF("ExitConfigMode (%i,%i): Success\n", (int)pstate->port, (int)pstate->slot);

	for(i=0,res=0; res != 1; i++)
	{
		WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, WEF_AND|WEF_CLEAR, NULL);

		res = PadIsSupported(pstate);

		if((res != 1) && (i >= 10))
		{
			D_PRINTF("PadIsSupported (%i,%i): Failed\n", (int)pstate->port, (int)pstate->slot);

			pstate->modeCurId = 0;
			pstate->currentTask = TASK_UPDATE_PAD;
			ExitThread();
		}
	}

	D_PRINTF("PadIsSupported (%i,%i): Success\n", (int)pstate->port, (int)pstate->slot);

	D_PRINTF("QueryPadThread: Done (%i,%i)\n", (int)pstate->port, (int)pstate->slot);

	modeCurId = pstate->modeCurId;
	pstate->state = PAD_STATE_EXECCMD;
	pstate->modeCurId = 0;
	pstate->currentTask = TASK_UPDATE_PAD;
	pstate->disconnected = 0;
	pstate->modeCurId = modeCurId;	//Weird, but this was how it was done.

	ExitThread();
}

static void SetMainModeThread(void *arg)
{
	u32 i, res;
	iop_thread_info_t tinfo;
	padState_t *pstate;

	ReferThreadStatus(TH_SELF, &tinfo);

	pstate = (padState_t*)tinfo.option;

	pstate->buttonDataReady = 0;
	pstate->state = PAD_STATE_EXECCMD;

	for(i=0,res=0; res != 1; i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_MAIN_MODE, WEF_AND|WEF_CLEAR, NULL);

		res = EnterConfigMode(pstate->modeCurId, pstate);

		if((res != 1) && (i >= 10))
		{
			pstate->reqState = PAD_RSTAT_FAILED;
			pstate->currentTask = TASK_UPDATE_PAD;
			ExitThread();
		}
	}

	for(i=0,res=0; res != 1; i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_MAIN_MODE, WEF_AND|WEF_CLEAR, NULL);

		res = SetMainMode( pstate);

		if((res != 1) && (i >= 10))
		{
			pstate->reqState = PAD_RSTAT_FAILED;
			pstate->currentTask = TASK_UPDATE_PAD;
			ExitThread();
		}
	}

	for(i=0,res=0; res != 1; i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_MAIN_MODE, WEF_AND|WEF_CLEAR, NULL);

		res = ExitConfigMode(pstate);

		if((res != 1) && (i >= 10))
		{
			pstate->reqState = PAD_RSTAT_FAILED;
			pstate->currentTask = TASK_UPDATE_PAD;
			ExitThread();
		}
	}

	pstate->currentTask = TASK_UPDATE_PAD;
	ExitThread();

}

void SetActAlignThread(void *arg)
{
	u32 i, res;
	iop_thread_info_t tinfo;
	padState_t *pstate;

	ReferThreadStatus(TH_SELF, &tinfo);

	pstate = (padState_t*)tinfo.option;

	pstate->buttonDataReady = 0;
	pstate->state = PAD_STATE_EXECCMD;

	for(i=0,res=0; res != 1; i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_ACT_ALIGN, WEF_AND|WEF_CLEAR, NULL);

		res = EnterConfigMode(pstate->modeCurId, pstate);

		if((res != 1) && (i >= 10))
		{
			pstate->reqState = PAD_RSTAT_FAILED;
			pstate->currentTask = TASK_UPDATE_PAD;
			ExitThread();
		}
	}

	for(i=0,res=0; res != 1; i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_ACT_ALIGN, WEF_AND|WEF_CLEAR, NULL);

		res = SetActAlign(pstate);

		if((res != 1) && (i >= 10))
		{
			pstate->reqState = PAD_RSTAT_FAILED;
			pstate->currentTask = TASK_UPDATE_PAD;
			ExitThread();
		}
	}

	for(i=0,res=0; res != 1; i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_ACT_ALIGN, WEF_AND|WEF_CLEAR, NULL);

		res = ExitConfigMode(pstate);

		if((res != 1) && (i >= 10))
		{
			pstate->reqState = PAD_RSTAT_FAILED;
			pstate->currentTask = TASK_UPDATE_PAD;
			ExitThread();
		}
	}

	pstate->currentTask = TASK_UPDATE_PAD;

	ExitThread();
}

static void SetButtonInfoThread(void *arg)
{
	int i, res;
	u32 val;
	iop_thread_info_t tinfo;
	padState_t *pstate;

	ReferThreadStatus(TH_SELF, &tinfo);

	pstate = (padState_t*)tinfo.option;

	pstate->buttonDataReady = 0;
	pstate->state = PAD_STATE_EXECCMD;

	for(i=0,res=0; res != 1; i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_SET_BUTTON_INFO, WEF_AND|WEF_CLEAR, NULL);

		res = EnterConfigMode(pstate->modeCurId, pstate);

		if((res != 1) && (i >= 10))
		{
			pstate->reqState = PAD_RSTAT_FAILED;
			pstate->currentTask = TASK_UPDATE_PAD;
			ExitThread();
		}
	}

	for(i=0,res=0; res != 1; i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_SET_BUTTON_INFO, WEF_AND|WEF_CLEAR, NULL);

		res = SetButtonInfo(pstate);

		if((res != 1) && (i >= 10))
		{
			pstate->reqState = PAD_RSTAT_FAILED;
			pstate->currentTask = TASK_UPDATE_PAD;
			ExitThread();
		}
	}

	for(val = 0; val < 12; val++)
	{
		for(i=0,res=0; res != 1; i++)
		{
			WaitClearEvent( pstate->eventflag, EF_SET_SET_BUTTON_INFO, WEF_AND|WEF_CLEAR, NULL);

			res = VrefParam(val, pstate);

			if((res != 1) && (i >= 10))
			{
				pstate->reqState = PAD_RSTAT_FAILED;
				pstate->currentTask = TASK_UPDATE_PAD;
				ExitThread();
			}
		}
	}

	for(i=0,res=0; res != 1; i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_SET_BUTTON_INFO, WEF_AND|WEF_CLEAR, NULL);

		res = ExitConfigMode(pstate);

		if((res != 1) && (i >= 10))
		{
			pstate->reqState = PAD_RSTAT_FAILED;
			pstate->currentTask = TASK_UPDATE_PAD;
			ExitThread();
		}
	}

	pstate->val_c6 = 1;
	pstate->currentTask = TASK_UPDATE_PAD;
	ExitThread();
}

static void SetVrefParamThread(void *arg)
{
	int i, res;
	u32 val;
	iop_thread_info_t tinfo;
	padState_t *pstate;

	ReferThreadStatus(TH_SELF, &tinfo);

	pstate = (padState_t*)tinfo.option;

	pstate->buttonDataReady = 0;
	pstate->state = PAD_STATE_EXECCMD;

	for(i=0,res=0; res != 1; i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_VREF_PARAM, WEF_AND|WEF_CLEAR, NULL);

		res = EnterConfigMode(pstate->modeCurId, pstate);

		if((res != 1) && (i >= 10))
		{
			pstate->reqState = PAD_RSTAT_FAILED;
			pstate->currentTask = TASK_UPDATE_PAD;
			ExitThread();
		}
	}

	for(val = 0; val < 12; val++)
	{
		for(i=0,res=0; res != 1; i++)
		{
			WaitClearEvent( pstate->eventflag, EF_SET_VREF_PARAM, WEF_AND|WEF_CLEAR, NULL);

			res = VrefParam(val, pstate);

			if((res != 1) && (i >= 10))
			{
				pstate->reqState = PAD_RSTAT_FAILED;
				pstate->currentTask = TASK_UPDATE_PAD;
				ExitThread();
			}
		}
	}

	for(i=0,res=0; res != 1; i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_VREF_PARAM, WEF_AND|WEF_CLEAR, NULL);

		res = ExitConfigMode( pstate);

		if((res != 1) && (i >= 10))
		{
			pstate->reqState = PAD_RSTAT_FAILED;
			pstate->currentTask = TASK_UPDATE_PAD;
			ExitThread();
		}
	}

	pstate->currentTask = TASK_UPDATE_PAD;

	ExitThread();
}

s32 padPortOpen(s32 port, s32 slot, s32 pad_area_ee_addr, u32 *buf)
{
	iop_event_t event;
	iop_thread_t thread;

	if(port > 2)
	{
		M_PRINTF("Invalid port number: %d\n", (int)port);
		return 0;
	}

	if(slot > 4)
	{
		M_PRINTF("Invalid slot number: %d\n", (int)port);
		return 0;
	}

	if( (openSlots[port] >> slot) & 0x1)
	{
		M_PRINTF("This slot is already open: (%d, %d)\n", (int)port, (int)slot);
		return 0;
	}

	padState[port][slot].port = port;
	padState[port][slot].slot = slot;

	openSlots[port] |= 1 << slot;

	padState[port][slot].state = PAD_STATE_EXECCMD;
	padState[port][slot].modeCurId = 0;
	padState[port][slot].reqState = PAD_RSTAT_COMPLETE;
	padState[port][slot].frame = 0;
	padState[port][slot].padarea_ee_addr = pad_area_ee_addr;
	padState[port][slot].buttonDataReady = 0;
	padState[port][slot].ee_actDirectSize = 0;
	padState[port][slot].val_c6 = 0;
	padState[port][slot].currentTask = TASK_NONE;
	padState[port][slot].runTask = TASK_NONE;
	padState[port][slot].val_184 = 0;

	padState[port][slot].disconnected = 1;
	padState[port][slot].stat70bit = 0;

	event.attr = EA_MULTI;
	event.bits = 0;

	if((padState[port][slot].eventflag = CreateEventFlag(&event)) == 0)
	{
		M_PRINTF("Port open failed (CreateEventFlag).\n");
		return 0;
	}


	// Create UpdatePadThread
	thread.attr = TH_C;
	thread.option = (u32)&padState[port][slot];
	thread.thread = &UpdatePadThread;
	thread.stacksize = 0x600;
	thread.priority = thpri_lo;

	padState[port][slot].updatepadTid = CreateThread(&thread);

	if(padState[port][slot].updatepadTid == 0)
	{
		M_PRINTF("Port open failed (CreateThread UpdatePadThread).\n");
		return 0;
	}

	// Create QueryPadThread
	thread.attr = TH_C;
	thread.option = (u32)&padState[port][slot];
	thread.thread = &QueryPadThread;
	thread.stacksize = 0x600;
	thread.priority = thpri_lo;

	padState[port][slot].querypadTid = CreateThread(&thread);

	if(padState[port][slot].querypadTid == 0)
	{
		M_PRINTF("Port open failed (CreateThread QueryPadThread).\n");
		return 0;
	}

	// Create SetMainModeThread
	thread.attr = TH_C;
	thread.option = (u32)&padState[port][slot];
	thread.thread = &SetMainModeThread;
	thread.stacksize = 0x400;
	thread.priority = thpri_lo;

	padState[port][slot].setmainmodeTid = CreateThread(&thread);

	if(padState[port][slot].setmainmodeTid == 0)
	{
		M_PRINTF("Port open failed (CreateThread SetMainModeThread)\n.");
		return 0;
	}

	// Create SetActAlignThread
	thread.attr = TH_C;
	thread.option = (u32)&padState[port][slot];
	thread.thread = &SetActAlignThread;
	thread.stacksize = 0x400;
	thread.priority = thpri_lo;

	padState[port][slot].setactalignTid = CreateThread(&thread);

	if(padState[port][slot].setactalignTid == 0)
	{
		M_PRINTF("Port open failed (CreateThread SetActAlignThread).\n");
		return 0;
	}

	// Create SetButtonInfoThread
	thread.attr = TH_C;
	thread.option = (u32)&padState[port][slot];
	thread.thread = &SetButtonInfoThread;
	thread.stacksize = 0x400;
	thread.priority = thpri_lo;

	padState[port][slot].setbuttoninfoTid = CreateThread(&thread);

	if(padState[port][slot].setbuttoninfoTid == 0)
	{
		M_PRINTF("Port open failed (CreateThread SetButtonInfoThread).\n");
		return 0;
	}

	// Create SetVrefParamThread
	thread.attr = TH_C;
	thread.option = (u32)&padState[port][slot];
	thread.thread = &SetVrefParamThread;
	thread.stacksize = 0x400;
	thread.priority = thpri_lo;

	padState[port][slot].setvrefparamTid = CreateThread(&thread);

	if(padState[port][slot].setvrefparamTid == 0)
	{
		M_PRINTF("Port open failed (CreateThread SetVrefParamThread).\n");
		return 0;
	}

	if(padState[port][slot].currentTask < TASK_QUERY_PAD)
	{
		StartThread(padState[port][slot].updatepadTid, NULL);
		padState[port][slot].currentTask = TASK_UPDATE_PAD;
	}
	else
	{
		M_PRINTF("Port open failed, busy.\n");
		return 0;
	}

	return 1;
}
