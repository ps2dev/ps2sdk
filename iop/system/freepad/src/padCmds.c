/*
 * freepad - IOP pad driver
 * Copyright (c) 2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include "types.h"
#include "thevent.h"
#include "freepad.h"
#include "sio2Cmds.h"
#include "padData.h"

void shiftarray(u8 *buf)
{
	u8 temp[32];
	u32 i;

	for(i=0; i < 32; i++)
		temp[i] = buf[i];

	buf[0] = 0xFF;

	for(i=0; i < 31; i++)
		buf[i+1] = temp[i];
}

u32 PadIsSupported(padState_t *pstate)
{
	u32 res;
	u32 ret = 0;

	res = sio2CmdGetPortCtrl1(0, pstate->stat70bit, pstate->model);

	pdSetCtrl1(pstate->port, pstate->slot, res);

	res = sio2CmdGetPortCtrl2(0, pstate->stat70bit);

	pdSetCtrl2(pstate->port, pstate->slot, res);

	res = pdGetRegData(0);

	pdSetRegData(pstate->port, pstate->slot, res);

	res = pdGetInSize(0);

	pdSetInSize(pstate->port, pstate->slot, res);
	
	res = pdGetOutSize(0);

	pdSetOutSize(pstate->port, pstate->slot, res);
	
	sio2CmdSetReadData(0, pstate->inbuffer);

	pdSetInBuffer(pstate->port, pstate->slot, 0, pstate->inbuffer);

	SetEventFlag(pstate->eventflag, EF_PAD_TRANSFER_START);

	WaitClearEvent(pstate->eventflag, EF_PAD_TRANSFER_DONE, 0x10, 0);

	res = pdGetError(pstate->port, pstate->slot);

	if(res == 0)
	{
		pdGetOutBuffer(pstate->port, pstate->slot, 0, pstate->outbuffer);
		
		if(pstate->stat70bit == 1) shiftarray(pstate->outbuffer);

		if(pstate->outbuffer[1] != 0)
		{
			if(sio2cmdCheckId(pstate->outbuffer[1]) == 1)
			{
				
				pstate->modeCurId = pstate->outbuffer[1];
				ret = 1;
			}
		}
	}

	SetEventFlag(pstate->eventflag, EF_TASK_DONE);

	return ret;
}

u32 VrefParam(u32 val, padState_t *pstate)
{
	u32 res;
	u32 ret = 0;

	res = sio2CmdGetPortCtrl1(PAD_ID_CONFIG, pstate->stat70bit, pstate->model);
	
	pdSetCtrl1(pstate->port, pstate->slot, res);

	res = sio2CmdGetPortCtrl2(PAD_ID_CONFIG, pstate->stat70bit);

	pdSetCtrl2(pstate->port, pstate->slot, res),

	res = pdGetRegData(PAD_ID_CONFIG);

	pdSetRegData(pstate->port, pstate->slot, res);

	res = pdGetInSize(PAD_ID_CONFIG);

	pdSetInSize(pstate->port, pstate->slot, res);

	res = pdGetOutSize(PAD_ID_CONFIG);

	pdSetOutSize(pstate->port, pstate->slot, res);

	sio2CmdSetSetVrefParam(PAD_ID_CONFIG, pstate->inbuffer);
	
	pstate->inbuffer[3] = val;
	pstate->inbuffer[4] = pstate->vrefParam[val];
	
	pdSetInBuffer(pstate->port, pstate->slot, 0, pstate->inbuffer);

	SetEventFlag(pstate->eventflag, EF_PAD_TRANSFER_START);

	WaitClearEvent(pstate->eventflag, EF_PAD_TRANSFER_DONE, 0x10, 0);

	if( pdGetError(pstate->port, pstate->slot) == 0)
	{
		pdGetOutBuffer(pstate->port, pstate->slot, 0, pstate->outbuffer);
		
		if(pstate->stat70bit == 1) shiftarray(pstate->outbuffer);

		if(pstate->outbuffer[1] == PAD_ID_CONFIG) ret = 1;
	}	

	SetEventFlag(pstate->eventflag, EF_TASK_DONE);	

	return ret;
}

u32 ReadData(padState_t *pstate)
{
	u32 res, i;
	u32 ret = 0;

	res = sio2CmdGetPortCtrl1(pstate->modeCurId, pstate->stat70bit, pstate->model);
	
	pdSetCtrl1(pstate->port, pstate->slot, res);

	res = sio2CmdGetPortCtrl2(pstate->modeCurId, pstate->stat70bit);

	pdSetCtrl2(pstate->port, pstate->slot, res);

	res = pdGetRegData( pstate->modeCurId );

	pdSetRegData( pstate->port, pstate->slot, res);
	
	res = pdGetInSize( pstate->modeCurId );

	pdSetInSize( pstate->port, pstate->slot, res);

	res = pdGetOutSize( pstate->modeCurId );

	pdSetOutSize(pstate->port, pstate->slot, res);

	for(i=0; i < 32; i++)
	{
		pstate->inbuffer[i] = 0;
		pstate->outbuffer[i] = 0;
	}
	
	sio2CmdSetReadData(pstate->modeCurId, pstate->inbuffer);

	if( pstate->ee_actDirectSize > 0)
	{
		for(i=0; i < pstate->ee_actDirectSize; i++)
		{
			if(pstate->ee_actAlignData[i] == 0xFF)
				pstate->inbuffer[i+3] = pstate->ee_actAlignData[i];
			else
				pstate->inbuffer[i+3] = pstate->ee_actDirectData[i];
		}
	}	

	pdSetInBuffer(pstate->port, pstate->slot, 0, pstate->inbuffer);

	SetEventFlag( pstate->eventflag, EF_PAD_TRANSFER_START);

	WaitClearEvent( pstate->eventflag, EF_PAD_TRANSFER_DONE, 0x10, 0);

	res = pdGetError(pstate->port, pstate->slot);

	if(res == 0)
	{
		pdGetOutBuffer(pstate->port, pstate->slot, 0, pstate->outbuffer);

		ret = 1;

		if(pstate->stat70bit == 1) shiftarray( pstate->outbuffer );
	}

	SetEventFlag( pstate->eventflag, EF_TASK_DONE);

	return ret;
}

u32 EnterConfigMode(u8 val, padState_t *pstate)
{
	u32 res;
	u32 ret = 0;

	res = sio2CmdGetPortCtrl1(val, pstate->stat70bit, pstate->model);
	
	pdSetCtrl1(pstate->port, pstate->slot, res);

	res = sio2CmdGetPortCtrl2(val, pstate->stat70bit);

	pdSetCtrl2(pstate->port, pstate->slot, res);

	res = pdGetRegData(val);

	pdSetRegData(pstate->port, pstate->slot, res);

	res = pdGetInSize(val);

	pdSetInSize(pstate->port, pstate->slot, res);

	res = pdGetOutSize(val);

	pdSetOutSize(pstate->port, pstate->slot, res);
	
	sio2CmdSetEnterConfigMode(val, pstate->inbuffer);

	pdSetInBuffer(pstate->port, pstate->slot, 0, pstate->inbuffer);

	SetEventFlag(pstate->eventflag, EF_PAD_TRANSFER_START);

	WaitClearEvent(pstate->eventflag, EF_PAD_TRANSFER_DONE, 0x10, 0);

	if( pdGetError(pstate->port, pstate->slot) == 0)
	{
		pdGetOutBuffer(pstate->port, pstate->slot, 0, pstate->outbuffer);

		if( pstate->stat70bit == 1)
		{
			shiftarray(pstate->outbuffer);
		}
		
		ret = 1;	
	}	

	SetEventFlag(pstate->eventflag, EF_TASK_DONE);


	return ret;
}

u32 QueryModel(padState_t *pstate)
{
	u32 res;
	u32 ret = 0;

	res = sio2CmdGetPortCtrl1(PAD_ID_CONFIG, pstate->stat70bit, pstate->model);

	pdSetCtrl1(pstate->port, pstate->slot, res);

	res = sio2CmdGetPortCtrl2(PAD_ID_CONFIG, pstate->stat70bit);

	pdSetCtrl2(pstate->port, pstate->slot, res);

	res = pdGetRegData(PAD_ID_CONFIG);
	
	pdSetRegData(pstate->port, pstate->slot, res);

	res = pdGetInSize(PAD_ID_CONFIG);

	pdSetInSize(pstate->port, pstate->slot, res);

	res = pdGetOutSize(PAD_ID_CONFIG);

	pdSetOutSize(pstate->port, pstate->slot, res);

	sio2CmdSetQueryModel(PAD_ID_CONFIG, pstate->inbuffer);
	
	pdSetInBuffer(pstate->port, pstate->slot, 0, pstate->inbuffer);

	SetEventFlag(pstate->eventflag, EF_PAD_TRANSFER_START);

	WaitClearEvent(pstate->eventflag, EF_PAD_TRANSFER_DONE, 0x10, 0);

	if( pdGetError(pstate->port, pstate->slot) == 0)
	{
		pdGetOutBuffer(pstate->port, pstate->slot, 0, pstate->outbuffer);

		if( pstate->stat70bit == 1) shiftarray( pstate->outbuffer );

		if( pstate->outbuffer[1] == PAD_ID_CONFIG)
		{
			pstate->numActuators = pstate->outbuffer[6];
			pstate->model = pstate->outbuffer[3];
			pstate->numModes = pstate->outbuffer[4];
			pstate->modeCurOffs = pstate->outbuffer[5];
			pstate->numActComb = pstate->outbuffer[7];

			if( pstate->numActuators > 4) pstate->numActuators = 4;
			if( pstate->numActComb > 4 ) pstate->numActComb = 4;
			if( pstate->numModes > 4) pstate->numModes = 4;
	
			ret = 1;
		}
	}	

	SetEventFlag(pstate->eventflag, EF_TASK_DONE);

	return ret;
}

u32 SetMainMode(padState_t *pstate)
{
	u32 ret = 0;
	u32 res;

	res = sio2CmdGetPortCtrl1(PAD_ID_CONFIG, pstate->stat70bit, pstate->model);

	pdSetCtrl1(pstate->port, pstate->slot, res);

	res = sio2CmdGetPortCtrl2(PAD_ID_CONFIG, pstate->stat70bit);

	pdSetCtrl2(pstate->port, pstate->slot, res);

	res = pdGetRegData(PAD_ID_CONFIG);

	pdSetRegData(pstate->port, pstate->slot, res);

	res = pdGetInSize(PAD_ID_CONFIG);
	
	pdSetInSize(pstate->port, pstate->slot, res);

	res = pdGetOutSize(PAD_ID_CONFIG);

	pdSetOutSize(pstate->port, pstate->slot, res);

	sio2CmdSetSetMainMode(PAD_ID_CONFIG, pstate->inbuffer);

	pstate->inbuffer[3] = pstate->mode;
	pstate->inbuffer[4] = pstate->lock;

	pdSetInBuffer(pstate->port, pstate->slot, 0, pstate->inbuffer);

	SetEventFlag(pstate->eventflag, EF_PAD_TRANSFER_START);

	WaitClearEvent(pstate->eventflag, EF_PAD_TRANSFER_DONE, 0x10, 0);

	if( pdGetError(pstate->port, pstate->slot) == 0)
	{
		pdGetOutBuffer(pstate->port, pstate->slot, 0, pstate->outbuffer);

		if(pstate->stat70bit == 1) shiftarray( pstate->outbuffer);

		if(pstate->outbuffer[1] == PAD_ID_CONFIG) ret = 1;

	}

	SetEventFlag(pstate->eventflag, EF_TASK_DONE);

	return ret;
}

u32 QueryAct(u32 actuator, padState_t *pstate)
{
	u32 ret = 0;
	u32 res;

	res = sio2CmdGetPortCtrl1(PAD_ID_CONFIG, pstate->stat70bit, pstate->model);

	pdSetCtrl1(pstate->port, pstate->slot, res);

	res = sio2CmdGetPortCtrl2(PAD_ID_CONFIG, pstate->stat70bit);

	pdSetCtrl2(pstate->port, pstate->slot, res);

	res = pdGetRegData(PAD_ID_CONFIG);

	pdSetRegData(pstate->port, pstate->slot, res);

	res = pdGetInSize(PAD_ID_CONFIG);

	pdSetInSize(pstate->port, pstate->slot, res);

	res = pdGetOutSize(PAD_ID_CONFIG);

	pdSetOutSize(pstate->port, pstate->slot, res);

	sio2CmdSetQueryAct(PAD_ID_CONFIG, pstate->inbuffer);

	pstate->inbuffer[3] = actuator;

	pdSetInBuffer(pstate->port, pstate->slot, 0, pstate->inbuffer);

	SetEventFlag(pstate->eventflag, EF_PAD_TRANSFER_START);

	WaitClearEvent(pstate->eventflag, EF_PAD_TRANSFER_DONE, 0x10, 0);

	if( pdGetError(pstate->port, pstate->slot) == 0)
	{
		pdGetOutBuffer(pstate->port, pstate->slot, 0, pstate->outbuffer);

		if(pstate->stat70bit == 1) shiftarray( pstate->outbuffer );

		if(pstate->outbuffer[1] == PAD_ID_CONFIG)
		{
			u8 *actData = (u8*)&pstate->actData[actuator];

			actData[0] = pstate->outbuffer[5];
			actData[1] = pstate->outbuffer[6];
			actData[2] = pstate->outbuffer[7];
			actData[3] = pstate->outbuffer[8];
		
			ret = 1;
		}
	}

	SetEventFlag(pstate->eventflag, EF_TASK_DONE);	

	return ret;	
}

u32 QueryComb(u32 val, padState_t *pstate)
{
	u32 ret = 0;
	u32 res;

	res = sio2CmdGetPortCtrl1(PAD_ID_CONFIG, pstate->stat70bit, pstate->model);
	
	pdSetCtrl1(pstate->port, pstate->slot, res);

	res = sio2CmdGetPortCtrl2(PAD_ID_CONFIG, pstate->stat70bit);

	pdSetCtrl2(pstate->port, pstate->slot, res),

	res = pdGetRegData(PAD_ID_CONFIG);

	pdSetRegData(pstate->port, pstate->slot, res);

	res = pdGetInSize(PAD_ID_CONFIG);

	pdSetInSize(pstate->port, pstate->slot, res);

	res = pdGetOutSize(PAD_ID_CONFIG);

	pdSetOutSize(pstate->port, pstate->slot, res);

	sio2CmdSetQueryComb(PAD_ID_CONFIG, pstate->inbuffer);

	pstate->inbuffer[3] = val;

	pdSetInBuffer(pstate->port, pstate->slot, 0, pstate->inbuffer);

	SetEventFlag(pstate->eventflag, EF_PAD_TRANSFER_START);

	WaitClearEvent(pstate->eventflag, EF_PAD_TRANSFER_DONE, 0x10, 0);

	if( pdGetError(pstate->port, pstate->slot) == 0)
	{
		pdGetOutBuffer(pstate->port, pstate->slot, 0, pstate->outbuffer);
		
		if(pstate->stat70bit == 1) shiftarray(pstate->outbuffer);

		if(pstate->outbuffer[1] == PAD_ID_CONFIG)
		{
			u8 *data = (u8*)&pstate->combData[val];
			
			data[0] = pstate->outbuffer[5];
			data[1] = pstate->outbuffer[6];
			data[2] = pstate->outbuffer[7];
			data[3] = pstate->outbuffer[8];

			ret = 1;
		}
	}	
	
	SetEventFlag(pstate->eventflag, EF_TASK_DONE);	


	return ret;
}

u32 QueryMode(u32 val, padState_t *pstate)
{
	u32 res;
	u32 ret = 0;

	res = sio2CmdGetPortCtrl1(PAD_ID_CONFIG, pstate->stat70bit, pstate->model);
	
	pdSetCtrl1(pstate->port, pstate->slot, res);

	res = sio2CmdGetPortCtrl2(PAD_ID_CONFIG, pstate->stat70bit);

	pdSetCtrl2(pstate->port, pstate->slot, res),

	res = pdGetRegData(PAD_ID_CONFIG);

	pdSetRegData(pstate->port, pstate->slot, res);

	res = pdGetInSize(PAD_ID_CONFIG);

	pdSetInSize(pstate->port, pstate->slot, res);

	res = pdGetOutSize(PAD_ID_CONFIG);

	pdSetOutSize(pstate->port, pstate->slot, res);

	sio2CmdSetQueryMode(PAD_ID_CONFIG, pstate->inbuffer);
	
	pstate->inbuffer[3] = val;

	pdSetInBuffer(pstate->port, pstate->slot, 0, pstate->inbuffer);

	SetEventFlag(pstate->eventflag, EF_PAD_TRANSFER_START);

	WaitClearEvent(pstate->eventflag, EF_PAD_TRANSFER_DONE, 0x10, 0);

	if( pdGetError(pstate->port, pstate->slot) == 0)
	{
		pdGetOutBuffer(pstate->port, pstate->slot, 0, pstate->outbuffer);
		
		if(pstate->stat70bit == 1) shiftarray(pstate->outbuffer);

		if(pstate->outbuffer[1] == PAD_ID_CONFIG)
		{
			u16 *modeTable = (u16*)&pstate->modeTable[0];

			modeTable[val] = (pstate->outbuffer[5] << 8) | pstate->outbuffer[6];

			ret = 1;
		}
	}	

	SetEventFlag(pstate->eventflag, EF_TASK_DONE);	

	return ret;
}

u32 ExitConfigMode(padState_t *pstate)
{
	u32 res;
	u32 ret = 0;

	res = sio2CmdGetPortCtrl1(PAD_ID_CONFIG, pstate->stat70bit, pstate->model);
	
	pdSetCtrl1(pstate->port, pstate->slot, res);

	res = sio2CmdGetPortCtrl2(PAD_ID_CONFIG, pstate->stat70bit);

	pdSetCtrl2(pstate->port, pstate->slot, res),

	res = pdGetRegData(PAD_ID_CONFIG);

	pdSetRegData(pstate->port, pstate->slot, res);

	res = pdGetInSize(PAD_ID_CONFIG);

	pdSetInSize(pstate->port, pstate->slot, res);

	res = pdGetOutSize(PAD_ID_CONFIG);

	pdSetOutSize(pstate->port, pstate->slot, res);

	sio2CmdSetExitConfigMode(PAD_ID_CONFIG, pstate->inbuffer);
	
	pdSetInBuffer(pstate->port, pstate->slot, 0, pstate->inbuffer);

	SetEventFlag(pstate->eventflag, EF_PAD_TRANSFER_START);

	WaitClearEvent(pstate->eventflag, EF_PAD_TRANSFER_DONE, 0x10, 0);

	if( pdGetError(pstate->port, pstate->slot) == 0)
	{
		pdGetOutBuffer(pstate->port, pstate->slot, 0, pstate->outbuffer);
		
		if(pstate->stat70bit == 1) shiftarray(pstate->outbuffer);

		ret = 1;
	}	

	SetEventFlag(pstate->eventflag, EF_TASK_DONE);	

	return ret;
}

u32 SetActAlign(padState_t *pstate)
{
	u32 res, i;
	u32 ret = 0;

	res = sio2CmdGetPortCtrl1(PAD_ID_CONFIG, pstate->stat70bit, pstate->model);
	
	pdSetCtrl1(pstate->port, pstate->slot, res);

	res = sio2CmdGetPortCtrl2(PAD_ID_CONFIG, pstate->stat70bit);

	pdSetCtrl2(pstate->port, pstate->slot, res),

	res = pdGetRegData(PAD_ID_CONFIG);

	pdSetRegData(pstate->port, pstate->slot, res);

	res = pdGetInSize(PAD_ID_CONFIG);

	pdSetInSize(pstate->port, pstate->slot, res);

	res = pdGetOutSize(PAD_ID_CONFIG);

	pdSetOutSize(pstate->port, pstate->slot, res);

	sio2CmdSetSetActAlign(PAD_ID_CONFIG, pstate->inbuffer);

	for(i=0; i < 6; i++) 
		pstate->inbuffer[3+i] = pstate->ee_actAlignData[i];

	pdSetInBuffer(pstate->port, pstate->slot, 0, pstate->inbuffer);

	SetEventFlag(pstate->eventflag, EF_PAD_TRANSFER_START);

	WaitClearEvent(pstate->eventflag, EF_PAD_TRANSFER_DONE, 0x10, 0);

	if( pdGetError(pstate->port, pstate->slot) == 0)
	{
		pdGetOutBuffer(pstate->port, pstate->slot, 0, pstate->outbuffer);
		
		if(pstate->stat70bit == 1) shiftarray(pstate->outbuffer);

		if(pstate->outbuffer[1] == PAD_ID_CONFIG) ret = 1;
	}			


	SetEventFlag(pstate->eventflag, EF_TASK_DONE);	

	return ret;
}

u32 QueryButtonMask(padState_t *pstate)
{
	u32 res;
	u32 ret = 0;

	res = sio2CmdGetPortCtrl1(PAD_ID_CONFIG, pstate->stat70bit, pstate->model);
	
	pdSetCtrl1(pstate->port, pstate->slot, res);

	res = sio2CmdGetPortCtrl2(PAD_ID_CONFIG, pstate->stat70bit);

	pdSetCtrl2(pstate->port, pstate->slot, res),

	res = pdGetRegData(PAD_ID_CONFIG);

	pdSetRegData(pstate->port, pstate->slot, res);

	res = pdGetInSize(PAD_ID_CONFIG);

	pdSetInSize(pstate->port, pstate->slot, res);

	res = pdGetOutSize(PAD_ID_CONFIG);

	pdSetOutSize(pstate->port, pstate->slot, res);

	sio2CmdSetQueryButtonMask(PAD_ID_CONFIG, pstate->inbuffer);

	pdSetInBuffer(pstate->port, pstate->slot, 0, pstate->inbuffer);

	SetEventFlag(pstate->eventflag, EF_PAD_TRANSFER_START);

	WaitClearEvent(pstate->eventflag, EF_PAD_TRANSFER_DONE, 0x10, 0);

	if( pdGetError(pstate->port, pstate->slot) == 0)
	{
		pdGetOutBuffer(pstate->port, pstate->slot, 0, pstate->outbuffer);
		
		if(pstate->stat70bit == 1) shiftarray(pstate->outbuffer);

		if(pstate->outbuffer[8] == 0x5A)
		{
			pstate->buttonMask[0] = pstate->outbuffer[3];	
			pstate->buttonMask[1] = pstate->outbuffer[4];
			pstate->buttonMask[2] = pstate->outbuffer[5];
			pstate->buttonMask[3] = pstate->outbuffer[6];
		}
		else
		{
			pstate->buttonMask[0] = 0;
			pstate->buttonMask[1] = 0;
			pstate->buttonMask[2] = 0;
			pstate->buttonMask[3] = 0;
		}

		ret = 1;		
	}			

	SetEventFlag(pstate->eventflag, EF_TASK_DONE);	

	return ret;
}

u32 SetButtonInfo(padState_t *pstate)
{
	u32 res, i;
	u32 ret = 0;

	res = sio2CmdGetPortCtrl1(PAD_ID_CONFIG, pstate->stat70bit, pstate->model);
	
	pdSetCtrl1(pstate->port, pstate->slot, res);

	res = sio2CmdGetPortCtrl2(PAD_ID_CONFIG, pstate->stat70bit);

	pdSetCtrl2(pstate->port, pstate->slot, res),

	res = pdGetRegData(PAD_ID_CONFIG);

	pdSetRegData(pstate->port, pstate->slot, res);

	res = pdGetInSize(PAD_ID_CONFIG);

	pdSetInSize(pstate->port, pstate->slot, res);

	res = pdGetOutSize(PAD_ID_CONFIG);

	pdSetOutSize(pstate->port, pstate->slot, res);

	sio2CmdSetSetButtonInfo(PAD_ID_CONFIG, pstate->inbuffer);
	
	for(i=0; i < 4; i++)
		pstate->inbuffer[3+i] = pstate->buttonInfo[i];

	pdSetInBuffer(pstate->port, pstate->slot, 0, pstate->inbuffer);

	SetEventFlag(pstate->eventflag, EF_PAD_TRANSFER_START);

	WaitClearEvent(pstate->eventflag, EF_PAD_TRANSFER_DONE, 0x10, 0);

	if( pdGetError(pstate->port, pstate->slot) == 0)
	{
		pdGetOutBuffer(pstate->port, pstate->slot, 0, pstate->outbuffer);
		
		if(pstate->stat70bit == 1) shiftarray(pstate->outbuffer);

		if(pstate->outbuffer[1] == PAD_ID_CONFIG) ret = 1;
	}	

	SetEventFlag(pstate->eventflag, EF_TASK_DONE);	

	return ret;
}

