/*
 * freepad - IOP pad driver
 * Copyright (c) 2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include "types.h"
#include "freepad.h"
#include "stdio.h"
#include "thevent.h"
#include "thbase.h"
#include "sio2Cmds.h"
#include "padCmds.h"
#include "padData.h"

void UpdatePadThread(void *arg)
{
	iop_thread_info_t tstatus;
	padState_t *pstate;
	u32 res = 0;

	ReferThreadStatus(0, &tstatus);
	pstate = (padState_t*)tstatus.info[IOP_THINFO_OPTION];

	while(1)
	{
		WaitClearEvent(pstate->eventflag, EF_UPDATE_PAD, 0x10, 0);

		// Check if able to read button data from pad
		if( ReadData(pstate) == 0)
		{
			pstate->state = PAD_STATE_ERROR;
			pstate->findPadRetries++;
			
			res += pdGetError( pstate->port, pstate->slot );
			
			if(res >= 10)
			{
				pstate->currentTask = TASK_QUERY_PAD;
				StartThread(pstate->querypadTid, 0);
			} 
		}
		else
		{
			res = 0;

			if( (pstate->outbuffer[2] != 0x5a) || ( pstate->outbuffer[1] != pstate->modeCurId) || (pstate->outbuffer[1] == 0xf3) )
			{
				pstate->buttonDataReady = 0;
				pstate->state = PAD_STATE_EXECCMD ;				
				pstate->currentTask = TASK_QUERY_PAD;
				StartThread(pstate->querypadTid, 0);			
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

				if( (pstate->reqState == PAD_RSTAT_BUSY) && (pstate->runTask == 0)) 
					pstate->reqState = PAD_RSTAT_COMPLETE;
			}
		}	
	}
}

void QueryPadThread(void *arg)
{
	iop_thread_info_t tinfo;
	padState_t *pstate;
	u32 i;
	u8 count;
	u32 res;
	u32 modeCurId = 0xFF;

	ReferThreadStatus(0, &tinfo);

	pstate = (padState_t*)tinfo.info[IOP_THINFO_OPTION];

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
		WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, 0x10, 0);

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
		for(i=0; i < 5; i++);
			pstate->ee_actAlignData[i] = 0xFF;
	}

	res = 0;	

	D_PRINTF("Found pad (%i,%i) - modeCurId: 0x%x\n", (int)pstate->port, (int)pstate->slot, (int)modeCurId);

	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, 0x10, 0);	
		
		res = EnterConfigMode(modeCurId, pstate);

		if(res == 1) pstate->modeConfig = MODE_CONFIG_READY;
	} 

	if( res != 1 )
	{
		D_PRINTF("EnterConfigMode (%i, %i): Failed\n", (int)pstate->port, (int)pstate->slot);

		pstate->modeConfig = MODE_CONFIG_QUERY_PAD;
		pstate->modeCurId = 0xFF;
		pstate->currentTask = TASK_UPDATE_PAD;

		ExitThread();
	}

	res = 0;

	D_PRINTF("EnterConfigMode (%i, %i): Success\n", (int)pstate->port, (int)pstate->slot);

	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, 0x10, 0);	
		res = QueryModel(pstate);
	}

	if( res != 1 )
	{
		D_PRINTF("QueryModel (%i, %i): Failed\n",(int)pstate->port, (int)pstate->slot);

		pstate->modeConfig = 0;
		pstate->currentTask = TASK_UPDATE_PAD; 
		ExitThread();
	}
		

	D_PRINTF("QueryModel (%i, %i): Success\n", (int)pstate->port, (int)pstate->slot);

	res = 0;
	

	if( (pstate->disconnected == 1) && (pstate->modeCurOffs != 0))
	{
		pstate->mode = 0;
		pstate->lock = 0;

		for(i=0; (i < 10)  || (res != 1); i++)
		{	
			WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, 0x10, 0);	

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

		}

		if(res != 1)
		{
			D_PRINTF("SetMainMode (%i, %i): Failed\n", (int)pstate->port, (int)pstate->slot);

			pstate->reqState = PAD_RSTAT_FAILED;
			pstate->currentTask = TASK_UPDATE_PAD;		
			ExitThread();
		}
	}

	res = 0;

	if( pstate->numActuators != 0)
	{
		count = 0;

		do
		{
			res = 0;

			for(i=0; (i < 10) && (res != 1); i++)
			{	
				WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, 0x10, 0);	

				res = QueryAct(count, pstate);
			}

			if(res != 1)
			{
				D_PRINTF("QueryAct (%i,%i): Failed\n", (int)pstate->port, (int)pstate->slot);

				pstate->modeCurId = 0;
				pstate->currentTask = TASK_UPDATE_PAD;
				ExitThread();
			}

			count++;
		}
		while( count < pstate->numActuators );

	}
	
	D_PRINTF("QueryAct (%i,%i): Success\n", (int)pstate->port, (int)pstate->slot);

	res = 0;

	if( pstate->numActComb != 0)
	{
		count = 0;

		do
		{
			res = 0;

			for(i=0; (i < 10)  && (res != 1); i++)
			{	
				WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, 0x10, 0);	

				res = QueryComb(count, pstate);
			}

			if(res != 1)
			{
				D_PRINTF("QueryComb (%i,%i): Failed\n", (int)pstate->port, (int)pstate->slot);

				pstate->modeCurId = 0;
				pstate->currentTask = TASK_UPDATE_PAD;
				ExitThread();
			}

			count++;
		}
		while( count < pstate->numActComb );


	}

	D_PRINTF("QueryComb (%i,%i): Success\n", (int)pstate->port, (int)pstate->slot);

	res = 0;

	if( pstate->numModes != 0)
	{
		count = 0;

		do
		{
			res = 0;

			for(i=0; (i < 10)  && (res != 1); i++)
			{	
				WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, 0x10, 0);	

				res = QueryMode(count, pstate);
			}

			if(res != 1)
			{
				D_PRINTF("QueryMode (%i,%i): Failed\n", (int)pstate->port, (int)pstate->slot);

				pstate->modeCurId = 0;
				pstate->currentTask = TASK_UPDATE_PAD;
				ExitThread();
			}

			count++;
		}
		while( count < pstate->numModes );
	}
	
	D_PRINTF("QueryMode (%i,%i): Success\n", (int)pstate->port, (int)pstate->slot);
	
	res = 0;

	for(i=0; (i < 10)  && (res != 1); i++)
	{	
		WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, 0x10, 0);	
		
		res = SetActAlign(pstate);
	}

	if(res != 1)
	{
		D_PRINTF("SetActAlign (%i,%i): Failed\n", (int)pstate->port, (int)pstate->slot);

		pstate->reqState = PAD_RSTAT_FAILED;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}	

	D_PRINTF("SetActAlign (%i,%i): Success\n", (int)pstate->port, (int)pstate->slot);

	res = 0;

	for(i=0; (i < 10) && (res != 1); i++)
	{	
		WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, 0x10, 0);	
		
		res = QueryButtonMask(pstate);
	}

	if(res != 1)
	{
		D_PRINTF("QueryButtonMask (%i,%i): Failed\n", (int)pstate->port, (int)pstate->slot);

		pstate->modeCurId = 0;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}	

	D_PRINTF("QueryButtonMask (%i,%i): Success\n", (int)pstate->port, (int)pstate->slot);

	res = 0;

	for(i=0; (i < 10)  && (res != 1); i++)
	{	
		WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, 0x10, 0);	
		
		res = ExitConfigMode(pstate);
	}

	if(res != 1)
	{
		D_PRINTF("ExitConfigMode (%i,%i): Failed\n", (int)pstate->port, (int)pstate->slot);

		pstate->modeCurId = 0;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}	

	D_PRINTF("ExitConfigMode (%i,%i): Success\n", (int)pstate->port, (int)pstate->slot);

	res = 0;	

	for(i=0; (i < 10) && (res != 1); i++)
	{	
		WaitClearEvent( pstate->eventflag, EF_QUERY_PAD, 0x10, 0);	
		
		res = PadIsSupported(pstate);
	}

	if(res != 1)
	{
		D_PRINTF("PadIsSupported (%i,%i): Failed\n", (int)pstate->port, (int)pstate->slot);

		pstate->modeCurId = 0;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}	

	D_PRINTF("PadIsSupported (%i,%i): Success\n", (int)pstate->port, (int)pstate->slot);

	D_PRINTF("QueryPadThread: Done (%i,%i)\n", (int)pstate->port, (int)pstate->slot);

	pstate->state = PAD_STATE_EXECCMD;
	pstate->currentTask = TASK_UPDATE_PAD;
	pstate->disconnected = 0;
	
	ExitThread();

}

void SetMainModeThread(void *arg)
{
	u32 i, res;	
	iop_thread_info_t tinfo;
	padState_t *pstate;
	
	ReferThreadStatus(0, &tinfo);

	pstate = (padState_t*)tinfo.info[IOP_THINFO_OPTION];

	pstate->buttonDataReady = 0;
	pstate->state = PAD_STATE_EXECCMD;

	res = 0;

	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_MAIN_MODE, 0x10, 0);	
		
		res = EnterConfigMode(pstate->modeCurId, pstate);
	}

	if(res != 1)
	{
		pstate->reqState = PAD_RSTAT_FAILED;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}

	res = 0;
	
	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_MAIN_MODE, 0x10, 0);	
		
		res = SetMainMode( pstate);
	}

	if(res != 1)
	{
		pstate->reqState = PAD_RSTAT_FAILED;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}

	res = 0;
	
	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_MAIN_MODE, 0x10, 0);	
		
		res = ExitConfigMode(pstate);
	}

	if(res != 1)
	{
		pstate->reqState = PAD_RSTAT_FAILED;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}

	pstate->currentTask = TASK_UPDATE_PAD;
	ExitThread();

}

void SetActAlignThread(void *arg)
{
	u32 i, res;	
	iop_thread_info_t tinfo;
	padState_t *pstate;
	
	ReferThreadStatus(0, &tinfo);

	pstate = (padState_t*)tinfo.info[IOP_THINFO_OPTION];

	pstate->buttonDataReady = 0;
	pstate->state = PAD_STATE_EXECCMD;

	res = 0;

	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_ACT_ALIGN, 0x10, 0);	
		res = EnterConfigMode(pstate->modeCurId, pstate);
	}

	if(res != 1)
	{
		pstate->reqState = PAD_RSTAT_FAILED;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}

	res = 0;

	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_ACT_ALIGN, 0x10, 0);	
		
		res = SetActAlign(pstate);
	}

	if(res != 1)
	{
		pstate->reqState = PAD_RSTAT_FAILED;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}

	res = 0;

	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_ACT_ALIGN, 0x10, 0);	
		
		res = ExitConfigMode(pstate);
	}

	if(res != 1)
	{
		pstate->reqState = PAD_RSTAT_FAILED;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}

	pstate->currentTask = TASK_UPDATE_PAD;

	ExitThread();
}

void SetButtonInfoThread(void *arg)
{
	u32 i, res;	
	iop_thread_info_t tinfo;
	padState_t *pstate;
	
	ReferThreadStatus(0, &tinfo);

	pstate = (padState_t*)tinfo.info[IOP_THINFO_OPTION];

	pstate->buttonDataReady = 0;
	pstate->state = PAD_STATE_EXECCMD;

	res = 0;

	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_SET_BUTTON_INFO, 0x10, 0);	
		
		res = EnterConfigMode(pstate->modeCurId, pstate);
	}

	if(res != 1)
	{
		pstate->reqState = PAD_RSTAT_FAILED;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}

	res = 0;

	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_SET_BUTTON_INFO, 0x10, 0);	
		
		res = SetButtonInfo(pstate);
	}

	if(res != 1)
	{
		pstate->reqState = PAD_RSTAT_FAILED;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}

	res = 0;

	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_SET_BUTTON_INFO, 0x10, 0);	
		
		res = VrefParam(0x10, pstate);
	}

	if(res != 1)
	{
		pstate->reqState = PAD_RSTAT_FAILED;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}

	res = 0;

	
	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_SET_BUTTON_INFO, 0x10, 0);	
		
		res = ExitConfigMode(pstate);
	}

	if(res != 1)
	{
		pstate->reqState = PAD_RSTAT_FAILED;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}
	

	pstate->currentTask = TASK_UPDATE_PAD;
	ExitThread();	
}

void SetVrefParamThread(void *arg)
{
	u32 i, res;	
	iop_thread_info_t tinfo;
	padState_t *pstate;
	
	ReferThreadStatus(0, &tinfo);

	pstate = (padState_t*)tinfo.info[IOP_THINFO_OPTION];

	pstate->buttonDataReady = 0;
	pstate->state = PAD_STATE_EXECCMD;

	res = 0;

	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_VREF_PARAM, 0x10, 0);	
		
		res = EnterConfigMode(pstate->modeCurId, pstate);
	}

	if(res != 1)
	{
		pstate->reqState = PAD_RSTAT_FAILED;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}

	res = 0;

	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_VREF_PARAM, 0x10, 0);	
		
		res = VrefParam(0, pstate);
	}

	if(res != 1)
	{
		pstate->reqState = PAD_RSTAT_FAILED;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
	}

	res = 0;

	for(i=0; (i < 10) && (res != 1); i++)
	{
		WaitClearEvent( pstate->eventflag, EF_SET_VREF_PARAM, 0x10, 0);	
		
		res = ExitConfigMode( pstate);
	}

	if(res != 1)
	{
		pstate->reqState = PAD_RSTAT_FAILED;
		pstate->currentTask = TASK_UPDATE_PAD;
		ExitThread();
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
	padState[port][slot].reqState = PAD_RSTAT_COMPLETE;
	padState[port][slot].frame = 0;
	padState[port][slot].padarea_ee_addr = pad_area_ee_addr;
	padState[port][slot].buttonDataReady = 0;
	padState[port][slot].ee_actDirectSize = 0;
	padState[port][slot].val_c6 = 0;
	padState[port][slot].currentTask = 0;
	padState[port][slot].runTask = 0;
	padState[port][slot].val_184 = 0;

	padState[port][slot].disconnected = 0;
	padState[port][slot].stat70bit = 0;

	event.attr = 2;
	event.bits = 0;

	if((padState[port][slot].eventflag = CreateEventFlag(&event)) == 0)
	{
		M_PRINTF("Port open failed (CreateEventFlag).\n");
		return 0;
	}


	thread.attr = TH_C;
	thread.option = (u32)&padState[port][slot]; // !!
	thread.stacksize = 0x600;
	thread.priority = 46;

	// Create UpdatePadThread
	thread.thread = UpdatePadThread;

	padState[port][slot].updatepadTid = CreateThread(&thread);

	if(padState[port][slot].updatepadTid == 0)
	{
		M_PRINTF("Port open failed (CreateThread UpdatePadThread).\n");
		return 0;
	}

	// Create QueryPadThread
	thread.thread = QueryPadThread;

	padState[port][slot].querypadTid = CreateThread(&thread);

	if(padState[port][slot].querypadTid == 0)
	{
		M_PRINTF("Port open failed (CreateThread QueryPadThread).\n");
		return 0;
	}
	
	// Create SetMainModeThread
	thread.thread = SetMainModeThread;

	padState[port][slot].setmainmodeTid = CreateThread(&thread);

	if(padState[port][slot].setmainmodeTid == 0)
	{
		M_PRINTF("Port open failed (CreateThread SetMainModeThread)\n.");
		return 0;
	}

	// Create SetActAlignThread
	thread.thread = SetActAlignThread;

	padState[port][slot].setactalignTid = CreateThread(&thread);

	if(padState[port][slot].setactalignTid == 0)
	{
		M_PRINTF("Port open failed (CreateThread SetActAlignThread).\n");
		return 0;
	}

	// Create SetButtonInfoThread
	thread.thread = SetButtonInfoThread;

	padState[port][slot].setbuttoninfoTid = CreateThread(&thread);

	if(padState[port][slot].setbuttoninfoTid == 0)
	{
		M_PRINTF("Port open failed (CreateThread SetButtonInfoThread).\n");
		return 0;
	}

	// Create SetVrefParamThread
	thread.thread = SetVrefParamThread;

	padState[port][slot].setvrefparamTid = CreateThread(&thread);

	if(padState[port][slot].setvrefparamTid == 0)
	{
		M_PRINTF("Port open failed (CreateThread SetVrefParamThread).\n");
		return 0;
	}

	if(padState[port][slot].currentTask > 1)
	{
		M_PRINTF("Port open failed, busy.\n");
		return 0;
	}
	else
	{
		StartThread(padState[port][slot].updatepadTid, 0);
		padState[port][slot].currentTask = 1;
	}

	return 1;
}

