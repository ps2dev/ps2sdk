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
#include "intrman.h"
#include "vblank.h"
#include "xsio2man.h"
#include "sifman.h"
#include "sio2Cmds.h"
#include "padData.h"

u32 vblank_end;
u32 frame_count = 0;
u32 vblankStartCount = 0;
void *pad_ee_addr;
s32 mainThreadCount = 0;
u32 mainThreadCount2 = 0;
u32 pad_port = 0;
u32 pad_slot = 0;
u32 freepad_init = 0;
vblankData_t vblankData;
u32 pad_portdata[2];
u32 openSlots[2];
s32 sifdma_id;
SifDmaTransfer_t sifdma_td[9];
/* It is very important that sif_buffer and padState are right after each other. */
u32 sif_buffer[4] __attribute__((aligned(4)));
padState_t padState[2][4];

void TransferThread(void *arg)
{
	while(1)
	{
		WaitClearEvent(vblankData.eventflag, EF_VB_TRANSFER, 0x10, 0);
		pdTransfer();
		SetEventFlag(vblankData.eventflag, EF_VB_TRANSFER_DONE);
	}
}

u32 setupEEButtonData(u32 port, u32 slot, padState_t *pstate)
{
	if(padState[port][slot].buttonDataReady == 1)
	{
		u32 i;

		pstate->ee_pdata.data[0] = 0;
		pstate->ee_pdata.data[1] = padState[port][slot].modeCurId;
		pstate->ee_pdata.data[31] = padState[port][slot].buttonStatus[2];

		for(i=2; i < 31; i++)
			pstate->ee_pdata.data[i] = padState[port][slot].buttonStatus[i+1];

		if(padState[port][slot].modeCurId == 18)
		{
			pstate->ee_pdata.data[2] = 0xFF,
			pstate->ee_pdata.data[3] = 0xFF;
			pstate->ee_pdata.data[4] = 0;
			pstate->ee_pdata.data[5] = 0;
		}

		return 32;
	}
	else
	{
		u32 i;

		pstate->ee_pdata.data[0] = 0xFF;

		for(i=1; i < 32; i++)
			pstate->ee_pdata.data[i] = 0;

		return 0;
	}
}


void DmaSendEE()
{
	s32 dma_stat;

	dma_stat = sceSifDmaStat(sifdma_id);

	if(dma_stat >= 0)
	{
		if( (frame_count % 30) == 0)
		{
			// These are actually kprintfs
			M_PRINTF("DMA Busy, ID = 0x%08X, dma_stat = %i\n", (int)sifdma_id, (int)dma_stat);
			M_PRINTF("        SB_STAT = 0x%08X\n", (int)SB_STAT);
		}
	}
	else
	{
		u32 port, slot;
		u32 sifdma_count = 1;

		/* This is where the 128*2 bytes of 'garbage' gets sent to EE.
		   I believe that only 16 bytes should have been sent, used for checking
           which ports/slots are open on the EE (padGetConnection). However 
		   someone made a mistake and sent 128 bytes instead :-) Its the first 
		   112 (128-16) bytes of padState[0][0] which gets sent along with the 
		   sif_buffer, which I think clearly indicates that sending 128 bytes 
		   is an error. - Lukasz
		*/

		sif_buffer[0]++;
		sif_buffer[1] = pad_portdata[0];
		sif_buffer[2] = pad_portdata[1];
		
		if( (sif_buffer[0] % 30) == 0)
			sifdma_td[0].dest = pad_ee_addr + 128; 
		else
			sifdma_td[0].dest = pad_ee_addr;

		sifdma_td[0].src = sif_buffer;
		sifdma_td[0].size = 128;
		sifdma_td[0].attr = 0;

		for(port=0; port < 2; port++)
		{
			for(slot=0; slot < 4; slot++)
			{
				if( (openSlots[port] >> slot) & 1 )
				{
					padState_t *p = &padState[port][slot];
	
					// Setup EE pad data
					p->ee_pdata.frame = p->frame;
					p->ee_pdata.findPadRetries = p->findPadRetries;
					p->ee_pdata.modeConfig = p->modeConfig;
					p->ee_pdata.modeCurId = p->modeCurId;
					p->ee_pdata.model = p->model;
					p->ee_pdata.buttonDataReady = p->buttonDataReady;
					p->ee_pdata.nrOfModes = p->numModes;
					p->ee_pdata.modeCurOffs = p->modeCurOffs;
					p->ee_pdata.nrOfActuators = p->numActuators;
					p->ee_pdata.numActComb = p->numActComb;
					p->ee_pdata.val_c6 = p->val_c6;
					p->ee_pdata.mode = p->mode;
					p->ee_pdata.lock = p->lock;
					p->ee_pdata.actDirSize = p->ee_actDirectSize;
					p->ee_pdata.state = p->state;
					p->ee_pdata.reqState = p->reqState;
					p->ee_pdata.currentTask = p->currentTask;

					p->frame++;

					p->ee_pdata.runTask = p->runTask;

					p->ee_pdata.actDirData[0] = *(u32*)&p->ee_actDirectData[0];
					p->ee_pdata.actDirData[1] = *(u32*)&p->ee_actDirectData[4];
					p->ee_pdata.actAlignData[0] = *(u32*)&p->ee_actAlignData[0];
					p->ee_pdata.actAlignData[1] = *(u32*)&p->ee_actAlignData[4];

					p->ee_pdata.actData[0] = p->actData[0];
					p->ee_pdata.actData[1] = p->actData[1];
					p->ee_pdata.actData[2] = p->actData[2];
					p->ee_pdata.actData[3] = p->actData[3];
					p->ee_pdata.actData[4] = p->actData[4];
					p->ee_pdata.actData[5] = p->actData[5];
					p->ee_pdata.actData[6] = p->actData[6];
					p->ee_pdata.actData[7] = p->actData[7];

					p->ee_pdata.modeTable[0] = p->modeTable[0];
					p->ee_pdata.modeTable[1] = p->modeTable[1];

					p->ee_pdata.stat70bit = p->stat70bit;

					if( p->buttonDataReady == 1)
					{
						p->ee_pdata.length = setupEEButtonData(port, slot, p);
					}
					else
					{
						p->ee_pdata.length = 0;
					}

					if( (p->frame & 1) == 0)
						sifdma_td[sifdma_count].dest = (void*)p->padarea_ee_addr;
					else
						sifdma_td[sifdma_count].dest = (void*)(p->padarea_ee_addr + 128);

					sifdma_td[sifdma_count].src =  &p->ee_pdata;	
					sifdma_td[sifdma_count].size = 128;
					sifdma_td[sifdma_count].attr = 0;
				
					sifdma_count++;
				}
			}
		}
	
		if(sifdma_count != 0)
		{
			int intr_state;

			CpuSuspendIntr(&intr_state);			
			
			sifdma_id = sceSifSetDma( sifdma_td, sifdma_count);

			CpuResumeIntr(intr_state);
			
		}
	}
}

u32 GetThreadsStatus(padState_t *state)
{
	u32 count = 0;
	iop_thread_info_t tinfo;

	if( ReferThreadStatus(state->updatepadTid, &tinfo) == 0)
	{
		if( (tinfo.info[2] & 0x10) == 0) count++;
	}

	if( ReferThreadStatus(state->querypadTid, &tinfo) == 0)
	{
		if( (tinfo.info[2] & 0x10) == 0) count++;
	}

	if( ReferThreadStatus(state->setmainmodeTid, &tinfo) == 0)
	{
		if( (tinfo.info[2] & 0x10) == 0) count++;
	}

	if( ReferThreadStatus(state->setactalignTid, &tinfo) == 0)
	{
		if( (tinfo.info[2] & 0x10) == 0) count++;
	}

	if( ReferThreadStatus(state->setbuttoninfoTid, &tinfo) == 0)
	{
		if( (tinfo.info[2] & 0x10) == 0) count++;
	}

	if( ReferThreadStatus(state->setvrefparamTid, &tinfo) == 0)
	{
		if( (tinfo.info[2] & 0x10) == 0) count++;
	}

	if(count == 0)
		return 1;
	else
		return 0;
}

void DeleteThreads(padState_t *state)
{
	DeleteThread(state->updatepadTid);
	DeleteThread(state->querypadTid);
	DeleteThread(state->setmainmodeTid);
	DeleteThread(state->setactalignTid);
	DeleteThread(state->setbuttoninfoTid);
	DeleteThread(state->setvrefparamTid);
}

void MainThread(void *arg)
{
	vblankData.stopTransfer = 0; 
	
	while(1)
	{
		u32 port, slot;

		mainThreadCount = (mainThreadCount+1) % 30;
	
		if( mainThreadCount == 0) sio2_mtap_update_slots();
	
		for(port=0; port < 2; port++)
		{
			for(slot=0; slot < 4; slot++)
			{
				if( ((openSlots[port] >> slot) & 0x1) == 1)
				{
					pdSetActive(port, slot, 0);

					padState[port][slot].stat70bit = pdGetStat70bit(port, slot);
					
					if(padState[port][slot].runTask != 0)
					{
						if(padState[port][slot].runTask == TASK_PORT_CLOSE)
						{
							padState[port][slot].currentTask = 3;
							padState[port][slot].runTask = 0;
							padState[port][slot].reqState = PAD_RSTAT_BUSY;
			
							SetEventFlag(padState[port][slot].eventflag, EF_EXIT_THREAD);
							
						}
						else
						{
							if(padState[port][slot].currentTask == TASK_UPDATE_PAD)
							{
								// Start Task
								StartThread(padState[port][slot].taskTid, 0);
								padState[port][slot].currentTask = padState[port][slot].runTask;
								padState[port][slot].runTask = 0;
								padState[port][slot].reqState = PAD_RSTAT_BUSY;
							}
							else
							{
								padState[port][slot].runTask = 0;
								padState[port][slot].reqState = PAD_RSTAT_FAILED;
							}
						}
					}
				}

				if( padState[port][slot].currentTask < 8 )
				{
					switch(padState[port][slot].currentTask)
					{
						case TASK_UPDATE_PAD:
						{
							SetEventFlag(padState[port][slot].eventflag, EF_UPDATE_PAD);
							WaitEventFlag(padState[port][slot].eventflag, EF_PAD_TRANSFER_START, 0x10, 0);
							pdSetActive(port, slot, 1);
						} break; 

						case TASK_QUERY_PAD:
						{
							SetEventFlag(padState[port][slot].eventflag, EF_QUERY_PAD);
							WaitEventFlag(padState[port][slot].eventflag, EF_PAD_TRANSFER_START, 0x10, 0);
							pdSetActive(port, slot, 1);
						} break; 	
					
						case TASK_PORT_CLOSE: 
						{
							padState[port][slot].buttonDataReady = 0;
							
							if(GetThreadsStatus( &padState[port][slot] ) == 1)
							{	
								padState[port][slot].currentTask = 0;
								padState[port][slot].reqState = PAD_RSTAT_COMPLETE ;
								openSlots[port] ^= (1 << slot);

								DeleteThreads( &padState[port][slot] );
								SetEventFlag(padState[port][slot].eventflag, EF_PORT_CLOSE);
							}

						} break;

						case TASK_SET_MAIN_MODE: 
						{
							SetEventFlag(padState[port][slot].eventflag, EF_SET_MAIN_MODE);
							WaitEventFlag(padState[port][slot].eventflag, EF_PAD_TRANSFER_START, 0x10, 0);
							pdSetActive(port, slot, 1);
						} break; 	

						case TASK_SET_ACT_ALIGN: 
						{
							SetEventFlag(padState[port][slot].eventflag, EF_SET_ACT_ALIGN);
							WaitEventFlag(padState[port][slot].eventflag, EF_PAD_TRANSFER_START, 0x10, 0);
							pdSetActive(port, slot, 1);
						} break; 	

						case TASK_SET_BUTTON_INFO:
						{
							SetEventFlag(padState[port][slot].eventflag, EF_SET_SET_BUTTON_INFO);
							WaitEventFlag(padState[port][slot].eventflag, EF_PAD_TRANSFER_START, 0x10, 0);
							pdSetActive(port, slot, 1);
						} break; 	

						case TASK_SET_VREF_PARAM: 
						{
							SetEventFlag(padState[port][slot].eventflag, EF_SET_VREF_PARAM);
							WaitEventFlag(padState[port][slot].eventflag, EF_PAD_TRANSFER_START, 0x10, 0);
							pdSetActive(port, slot, 1);
						} break; 	

					}		
				}
			}
		}
	
		// Transfer is started in VblankStart
		vblankData.stopTransfer = 0;
		WaitClearEvent(vblankData.eventflag, EF_VB_TRANSFER_DONE, 0x10, 0);
		vblankData.stopTransfer = 1;

		if( (openSlots[0] != 0) || (openSlots[1] != 0))
		{
			for(port=0; port < 2; port++)
			{
				for(slot=0; slot < 4; slot++)
				{
					if(pdIsActive(port, slot) == 1)
					{
						/* Signal transfer done and wait to task (reading 
						   sio2 data) to be done. */
						SetEventFlag(padState[port][slot].eventflag, EF_PAD_TRANSFER_DONE);
						WaitEventFlag(padState[port][slot].eventflag, EF_TASK_DONE, 0x10, 0);
	
					}
				}
			}
		}

		// Send pad data to EE
		DmaSendEE();

		mainThreadCount2++; // s7

		// Check for disconnected controllers
		if(mainThreadCount2 >= 8)
		{
			if(mainThreadCount == 0)
			{
				if( pdIsActive(pad_port, pad_slot) == 1)
				{
					if( padState[pad_port][pad_slot].state == PAD_STATE_DISCONN)
						pad_portdata[pad_port] &= ~(1 << pad_slot); // clear slot	
					else
						pad_portdata[pad_port] |= (1 << pad_slot);	// set slot
				}
				else
				{
					if( pdCheckConnection(pad_port, pad_slot) == 1)
						pad_portdata[pad_port] |= (1 << pad_slot);	// set slot
					else
						pad_portdata[pad_port] &= ~(1 << pad_slot); // clear slot	
				}

				
				pad_slot++;

				if(pad_slot >= 4) 
				{
					pad_slot = 0;

					pad_port++;
	
					if(pad_port >= 2) pad_port = 0;
				}
				

				mainThreadCount2 = 0;

			}	
		}

	}
}

s32 VbReferThreadStatus(vblankData_t *vData)
{
	iop_thread_info_t tinfo; 
	s32 ret = 0;

	if( iReferThreadStatus(vData->tid_1, &tinfo) == 0)
	{
		if( (tinfo.info[2] & 0x10) == 0 )
			ret++;
	}

	if( iReferThreadStatus(vData->tid_2, &tinfo) == 0)
	{
		if( (tinfo.info[2] & 0x10) == 0 )
			ret++;
	}

	return ret;
}

int VblankStart(vblankData_t *vData)
{
	vblank_end = 0;

	frame_count++;
	vblankStartCount++;

	if((vData->init == 1) && (vData->stopTransfer == 0))
		iSetEventFlag(vData->eventflag, EF_VB_TRANSFER);

	/* Wait for threads to exit and signal event to padEnd */
	if(vData->padEnd == 1)
	{
		if( VbReferThreadStatus(vData) == 0 ) 
			iSetEventFlag(vData->eventflag, EF_VB_WAIT_THREAD_EXIT);
	}

	return 1;
}


int VblankEnd(void *arg)
{
	vblank_end = 1;
	return 1;
}

s32 padInit(void * ee_addr)
{
	iop_event_t event;

	if(freepad_init == 1)
	{
		M_PRINTF("Refresh request from EE\n.");
		padEnd();
	}

	vblankData.padEnd = 0;	
	vblankData.init = 0;	
	vblankData.stopTransfer = 0;
	
	pad_ee_addr = ee_addr;
	pad_port = 0;
	pad_slot = 0;	
	mainThreadCount2 = 0;	
	pad_portdata[0] = 0;	
	pad_portdata[1] = 0;	
	sif_buffer[0] = 0;

	sio2cmdReset();
	sio2cmdInitFindPads();
	sio2cmdInitMouse();
	sio2cmdInitNegicon();
	sio2cmdInitKonamiGun();
	sio2cmdInitDigital();
	sio2cmdInitJoystick();	
	sio2cmdInitNamcoGun();
	sio2cmdInitAnalog();
	sio2cmdInitJogcon();
	sio2cmdInitConfig();

	pdReset();

	openSlots[0] = 0;
	openSlots[1] = 0;

	event.attr = 2;
	event.bits = 0;
	
	vblankData.eventflag = CreateEventFlag(&event);

	if( vblankData.eventflag == 0)
		M_PRINTF("padInit: CreateEventFlag failed (%i).\n", (int)vblankData.eventflag);

	if(vblankData.eventflag != 0)
	{
		iop_thread_t thread;
		int intr_state;

		thread.attr = TH_C;
		thread.thread = TransferThread;
		thread.stacksize = 0x800;
		thread.priority = 20;

		vblankData.tid_2 = CreateThread(&thread);

		if(vblankData.tid_2 == 0)
		{
			M_PRINTF("padInit: CreateThread TransferThread failed (%i)\n.", (int)vblankData.tid_2);
			return 0;
		}

		StartThread(vblankData.tid_2, 0);

		thread.attr = TH_C;
		thread.thread = MainThread;
		thread.stacksize = 0x1000;
		thread.priority = 46;

		vblankData.tid_1 = CreateThread(&thread);

		if(vblankData.tid_1 == 0)
		{
			M_PRINTF("padInit: CreateThread MainThread failed (%i)\n.", (int)vblankData.tid_1);
			return 0;
		}

		StartThread(vblankData.tid_1, 0);

		CpuSuspendIntr(&intr_state);

		RegisterVblankHandler(0, 16, (void*)VblankStart, (void*)&vblankData);
		RegisterVblankHandler(1, 16, (void*)VblankEnd, (void*)&vblankData);

		CpuResumeIntr(intr_state);

		vblankData.init = 1;
		freepad_init = 1;

		D_PRINTF("padInit: Success\n");

		return 1;
	}
	
	D_PRINTF("padInit: Failed\n");

	return 0;
}
