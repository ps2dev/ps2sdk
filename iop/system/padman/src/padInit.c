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
#include "intrman.h"
#include "vblank.h"
#include "xsio2man.h"
#include "sifman.h"
#include "sio2Cmds.h"
#include "sysmem.h"
#include "padData.h"

int pad_port;
int pad_slot;
u32 mainThreadCount2;
u32 pad_portdata[2];
/* It is very important that sif_buffer and padState are right after each other. */
u32 sif_buffer[4] __attribute__((aligned(4)));
padState_t padState[2][4];
u32 openSlots[2];
vblankData_t vblankData;
int padman_init;
void *pad_ee_addr;
int thpri_hi;
int thpri_lo;
SifDmaTransfer_t sifdma_td[9];	//Original was likely 16 descriptors.

int vblank_end = 0;
u32 frame_count = 0;
int sifdma_id = 0;
u32 vblankStartCount = 0;
s32 mainThreadCount = 0;

static void TransferThread(void *arg)
{
	while(1)
	{
		WaitClearEvent(vblankData.eventflag, EF_VB_TRANSFER, WEF_AND|WEF_CLEAR, NULL);
		pdTransfer();
		SetEventFlag(vblankData.eventflag, EF_VB_TRANSFER_DONE);
	}
}

u32 setupEEButtonData(u32 port, u32 slot, padState_t *pstate)
{
	if(padState[port][slot].buttonDataReady == 1)
	{
		int i;

		pstate->ee_pdata.data[0] = 0;
		pstate->ee_pdata.data[1] = padState[port][slot].modeCurId;
		pstate->ee_pdata.data[31] = padState[port][slot].buttonStatus[2];

		for(i=2; i < 31; i++)
			pstate->ee_pdata.data[i] = padState[port][slot].buttonStatus[i+1];

		if(padState[port][slot].modeCurId == 0x12)
		{
			pstate->ee_pdata.data[2] = 0xFF,
			pstate->ee_pdata.data[3] = 0xFF;
			pstate->ee_pdata.data[4] = 0;
			pstate->ee_pdata.data[5] = 0;
		}

		//New in v3.6
		if(padState[port][slot].modeCurId == 0x79)
		{
			int value;

			//Check button status & update pressure data
			value = ~((pstate->ee_pdata.data[2] << 8) | pstate->ee_pdata.data[3]);
			if(value & 0x2000)
			{
				if(pstate->ee_pdata.data[8] == 0)
					pstate->ee_pdata.data[8] = 1;
			}
			else
			{
				if(pstate->ee_pdata.data[8] != 0)
					pstate->ee_pdata.data[8] = 0;
			}

			if(value & 0x8000)
			{
				if(pstate->ee_pdata.data[9] == 0)
					pstate->ee_pdata.data[9] = 1;
			}
			else
			{
				if(pstate->ee_pdata.data[9] != 0)
					pstate->ee_pdata.data[9] = 0;
			}

			if(value & 0x1000)
			{
				if(pstate->ee_pdata.data[10] == 0)
					pstate->ee_pdata.data[10] = 1;
			}
			else
			{
				if(pstate->ee_pdata.data[10] != 0)
					pstate->ee_pdata.data[10] = 0;
			}

			if(value & 0x4000)
			{
				if(pstate->ee_pdata.data[11] == 0)
					pstate->ee_pdata.data[11] = 1;
			}
			else
			{
				if(pstate->ee_pdata.data[11] != 0)
					pstate->ee_pdata.data[11] = 0;
			}

			if(value & 0x0010)
			{
				if(pstate->ee_pdata.data[12] == 0)
					pstate->ee_pdata.data[12] = 1;
			}
			else
			{
				if(pstate->ee_pdata.data[12] != 0)
					pstate->ee_pdata.data[12] = 0;
			}

			if(value & 0x0020)
			{
				if(pstate->ee_pdata.data[13] == 0)
					pstate->ee_pdata.data[13] = 1;
			}
			else
			{
				if(pstate->ee_pdata.data[13] != 0)
					pstate->ee_pdata.data[13] = 0;
			}

			if(value & 0x0040)
			{
				if(pstate->ee_pdata.data[14] == 0)
					pstate->ee_pdata.data[14] = 1;
			}
			else
			{
				if(pstate->ee_pdata.data[14] != 0)
					pstate->ee_pdata.data[14] = 0;
			}

			if(value & 0x0080)
			{
				if(pstate->ee_pdata.data[15] == 0)
					pstate->ee_pdata.data[15] = 1;
			}
			else
			{
				if(pstate->ee_pdata.data[15] != 0)
					pstate->ee_pdata.data[15] = 0;
			}

			if(value & 0x0004)
			{
				if(pstate->ee_pdata.data[16] == 0)
					pstate->ee_pdata.data[16] = 1;
			}
			else
			{
				if(pstate->ee_pdata.data[16] != 0)
					pstate->ee_pdata.data[16] = 0;
			}

			if(value & 0x0008)
			{
				if(pstate->ee_pdata.data[17] == 0)
					pstate->ee_pdata.data[17] = 1;
			}
			else
			{
				if(pstate->ee_pdata.data[17] != 0)
					pstate->ee_pdata.data[17] = 0;
			}

			if(value & 0x0001)
			{
				if(pstate->ee_pdata.data[18] == 0)
					pstate->ee_pdata.data[18] = 1;
			}
			else
			{
				if(pstate->ee_pdata.data[18] != 0)
					pstate->ee_pdata.data[18] = 0;
			}

			if(value & 0x0002)
			{
				if(pstate->ee_pdata.data[19] == 0)
					pstate->ee_pdata.data[19] = 1;
			}
			else
			{
				if(pstate->ee_pdata.data[19] != 0)
					pstate->ee_pdata.data[19] = 0;
			}
		}

		return 32;
	}
	else
	{
		int i;

		pstate->ee_pdata.data[0] = 0xFF;

		for(i=1; i < 32; i++)
			pstate->ee_pdata.data[i] = 0;

		return 0;
	}
}


static void DmaSendEE(void)
{
	int dma_stat;

	dma_stat = sceSifDmaStat(sifdma_id);

	if(dma_stat >= 0)
	{
		if( (frame_count % 240) == 0)
		{
			M_KPRINTF("DMA Busy ID = %08x ret = %d\n", sifdma_id, dma_stat);
			M_KPRINTF("        SB_STAT = %08x\n", SB_STAT);
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

		if( (sif_buffer[0] % 2) == 0)
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

					p->ee_pdata.actDirData[0] = p->ee_actDirectData.data32[0];
					p->ee_pdata.actDirData[1] = p->ee_actDirectData.data32[1];
					p->ee_pdata.actAlignData[0] = p->ee_actAlignData.data32[0];
					p->ee_pdata.actAlignData[1] = p->ee_actAlignData.data32[1];

					p->ee_pdata.actData[0] = p->actData.data32[0];
					p->ee_pdata.actData[1] = p->actData.data32[1];
					p->ee_pdata.actData[2] = p->actData.data32[2];
					p->ee_pdata.actData[3] = p->actData.data32[3];
					p->ee_pdata.combData[0] = p->combData.data32[0];
					p->ee_pdata.combData[1] = p->combData.data32[1];
					p->ee_pdata.combData[2] = p->combData.data32[2];
					p->ee_pdata.combData[3] = p->combData.data32[3];

					p->ee_pdata.modeTable[0] = p->modeTable.data32[0];
					p->ee_pdata.modeTable[1] = p->modeTable.data32[1];

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

			if(sifdma_id == 0)
				M_KPRINTF("sceSifSetDma failed\n");
		}
	}
}

static u32 GetThreadsStatus(padState_t *state)
{
	u32 count = 0;
	iop_thread_info_t tinfo;

	if( ReferThreadStatus(state->updatepadTid, &tinfo) == 0)
	{
		if( (tinfo.status & THS_DORMANT) == 0) count++;
	}

	if( ReferThreadStatus(state->querypadTid, &tinfo) == 0)
	{
		if( (tinfo.status & THS_DORMANT) == 0) count++;
	}

	if( ReferThreadStatus(state->setmainmodeTid, &tinfo) == 0)
	{
		if( (tinfo.status & THS_DORMANT) == 0) count++;
	}

	if( ReferThreadStatus(state->setactalignTid, &tinfo) == 0)
	{
		if( (tinfo.status & THS_DORMANT) == 0) count++;
	}

	if( ReferThreadStatus(state->setbuttoninfoTid, &tinfo) == 0)
	{
		if( (tinfo.status & THS_DORMANT) == 0) count++;
	}

	if( ReferThreadStatus(state->setvrefparamTid, &tinfo) == 0)
	{
		if( (tinfo.status & THS_DORMANT) == 0) count++;
	}

	if(count == 0)
		return 1;
	else
		return 0;
}

static void DeleteThreads(padState_t *state)
{
	DeleteThread(state->updatepadTid);
	DeleteThread(state->querypadTid);
	DeleteThread(state->setmainmodeTid);
	DeleteThread(state->setactalignTid);
	DeleteThread(state->setbuttoninfoTid);
	DeleteThread(state->setvrefparamTid);
}

static void MainThread(void *arg)
{
	while(1)
	{
		u32 port, slot;

		mainThreadCount++;

		if( mainThreadCount % 30 == 0 ) sio2_mtap_update_slots();

		for(port=0; port < 2; port++)
		{
			for(slot=0; slot < 4; slot++)
			{
				if( ((openSlots[port] >> slot) & 0x1) == 1)
				{
					pdSetActive(port, slot, 0);

					padState[port][slot].stat70bit = pdGetStat70bit(port, slot);

					if(padState[port][slot].runTask != TASK_NONE)
					{
						if(padState[port][slot].runTask == TASK_PORT_CLOSE)
						{
							padState[port][slot].currentTask = padState[port][slot].runTask;
							padState[port][slot].runTask = TASK_NONE;
							padState[port][slot].reqState = PAD_RSTAT_BUSY;

							SetEventFlag(padState[port][slot].eventflag, EF_EXIT_THREAD);
						}
						else
						{
							if(padState[port][slot].currentTask == TASK_UPDATE_PAD)
							{
								// Start Task
								StartThread(padState[port][slot].taskTid, NULL);
								padState[port][slot].currentTask = padState[port][slot].runTask;
								padState[port][slot].runTask = TASK_NONE;
								padState[port][slot].reqState = PAD_RSTAT_BUSY;
							}
							else
							{
								padState[port][slot].runTask = TASK_NONE;
								padState[port][slot].reqState = PAD_RSTAT_FAILED;
							}
						}
					}
				}

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
						padState[port][slot].buttonDataReady = 0;

						SetEventFlag(padState[port][slot].eventflag, EF_QUERY_PAD);
						WaitEventFlag(padState[port][slot].eventflag, EF_PAD_TRANSFER_START, 0x10, 0);
						pdSetActive(port, slot, 1);
					} break;

					case TASK_PORT_CLOSE:
					{
						if(GetThreadsStatus( &padState[port][slot] ) == 1)
						{
							padState[port][slot].currentTask = TASK_NONE;
							padState[port][slot].reqState = PAD_RSTAT_COMPLETE;
							openSlots[port] ^= (1 << slot);

							DeleteThreads( &padState[port][slot] );
							SetEventFlag(padState[port][slot].eventflag, EF_PORT_CLOSE);
						}

					} break;

					case TASK_SET_MAIN_MODE:
					{
						padState[port][slot].buttonDataReady = 0;

						SetEventFlag(padState[port][slot].eventflag, EF_SET_MAIN_MODE);
						WaitEventFlag(padState[port][slot].eventflag, EF_PAD_TRANSFER_START, 0x10, 0);
						pdSetActive(port, slot, 1);
					} break;

					case TASK_SET_ACT_ALIGN:
					{
						padState[port][slot].buttonDataReady = 0;

						SetEventFlag(padState[port][slot].eventflag, EF_SET_ACT_ALIGN);
						WaitEventFlag(padState[port][slot].eventflag, EF_PAD_TRANSFER_START, 0x10, 0);
						pdSetActive(port, slot, 1);
					} break;

					case TASK_SET_BUTTON_INFO:
					{
						padState[port][slot].buttonDataReady = 0;

						SetEventFlag(padState[port][slot].eventflag, EF_SET_SET_BUTTON_INFO);
						WaitEventFlag(padState[port][slot].eventflag, EF_PAD_TRANSFER_START, 0x10, 0);
						pdSetActive(port, slot, 1);
					} break;

					case TASK_SET_VREF_PARAM:
					{
						padState[port][slot].buttonDataReady = 0;

						SetEventFlag(padState[port][slot].eventflag, EF_SET_VREF_PARAM);
						WaitEventFlag(padState[port][slot].eventflag, EF_PAD_TRANSFER_START, 0x10, 0);
						pdSetActive(port, slot, 1);
					} break;

				}
			}
		}

		// Transfer is started in VblankStart
		vblankData.stopTransfer = 0;
		WaitClearEvent(vblankData.eventflag, EF_VB_TRANSFER_DONE, WEF_AND|WEF_CLEAR, NULL);

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
			if(mainThreadCount % 30 != 0)
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

				//Move onto the next slot
				pad_slot++;
				if(pad_slot >= 4)
				{
					//Move onto the next port
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
		if( (tinfo.status & THS_DORMANT) == 0 )
			ret++;
	}

	if( iReferThreadStatus(vData->tid_2, &tinfo) == 0)
	{
		if( (tinfo.status & THS_DORMANT) == 0 )
			ret++;
	}

	return ret;
}

int VblankStart(void *arg)
{
	vblankData_t *vData = arg;

	vblank_end = 0;
	vblankStartCount++;
	frame_count++;

	if((vData->init == 1) && (vData->stopTransfer == 0))
	{
		vData->stopTransfer = 1;
		iSetEventFlag(vData->eventflag, EF_VB_TRANSFER);
	}

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
	iop_thread_t thread;
	int intr_state;
	iop_event_t event;

	if(padman_init == 1)
	{
		M_PRINTF("Refresh request from EE\n.");
		padEnd();
	}

	vblankData.padEnd = 0;
	vblankData.init = 0;
	vblankData.stopTransfer = 1;

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

	event.attr = EA_MULTI;
	event.bits = 0;

	vblankData.eventflag = CreateEventFlag(&event);

	if( vblankData.eventflag == 0)
	{
		M_PRINTF("padInit: CreateEventFlag failed (%d).\n", vblankData.eventflag);
		return 0;
	}

	thread.attr = TH_C;
	thread.thread = &TransferThread;
	thread.stacksize = 0x800;
	thread.priority = thpri_hi;

	vblankData.tid_2 = CreateThread(&thread);

	if(vblankData.tid_2 == 0)
	{
		M_PRINTF("padInit: CreateThread TransferThread failed (%d)\n.", vblankData.tid_2);
		return 0;
	}

	StartThread(vblankData.tid_2, NULL);

	thread.attr = TH_C;
	thread.thread = MainThread;
	thread.stacksize = 0x1000;
	thread.priority = thpri_lo;

	vblankData.tid_1 = CreateThread(&thread);

	if(vblankData.tid_1 == 0)
	{
		M_PRINTF("padInit: CreateThread MainThread failed (%d)\n.", vblankData.tid_1);
		return 0;
	}

	StartThread(vblankData.tid_1, NULL);

	CpuSuspendIntr(&intr_state);

	RegisterVblankHandler(0, 16, &VblankStart, (void*)&vblankData);
	RegisterVblankHandler(1, 16, &VblankEnd, (void*)&vblankData);

	CpuResumeIntr(intr_state);	//Original BUG: was originally a call to CpuEnableIntr with intr_state as an argument

	vblankData.init = 1;
	padman_init = 1;

	return 1;
}
