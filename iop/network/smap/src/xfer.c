#include <errno.h>
#include <stdio.h>
#include <dmacman.h>
#include <dev9.h>
#include <intrman.h>
#include <loadcore.h>
#include <modload.h>
#include <stdio.h>
#include <sysclib.h>
#include <thbase.h>
#include <thevent.h>
#include <thsemap.h>
#include <irx.h>

#include <smapregs.h>
#include <speedregs.h>

#include <netman.h>

#include "main.h"

#include "xfer.h"

extern void *_gp;
extern struct SmapDriverData SmapDriverData;

static int SmapDmaTransfer(volatile u8 *smap_regbase, void *buffer, unsigned int size, int direction){
	unsigned int NumBlocks;
	int result;

	if((NumBlocks=size>>7)>0){
		if(dev9DmaTransfer(1, buffer, NumBlocks<<16|0x20, direction)>=0){
			result=NumBlocks<<7;
		}
		else result=0;
	}
	else result=0;

	return result;
}

static inline void CopyFromFIFO(volatile u8 *smap_regbase, void *buffer, unsigned int length, u16 RxBdPtr){
	int i, result;

	SMAP_REG16(SMAP_R_RXFIFO_RD_PTR)=RxBdPtr;

	if((result=SmapDmaTransfer(smap_regbase, buffer, length, DMAC_TO_MEM))<0){
		result=0;
	}

	if(result<length){
		for(i=result; i<length; i+=4){
			((u32*)buffer)[i/4]=SMAP_REG32(SMAP_R_RXFIFO_DATA);
		}
	}
}

int HandleRxIntr(struct SmapDriverData *SmapDrivPrivData){
	USE_SMAP_RX_BD;
	int NumPacketsReceived;
	volatile smap_bd_t *PktBdPtr;
	volatile u8 *smap_regbase;
	void *pbuf, *payload;
	u16 ctrl_stat, length;

	smap_regbase=SmapDrivPrivData->smap_regbase;

	NumPacketsReceived=0;

	while(1){
		PktBdPtr=&rx_bd[SmapDrivPrivData->RxBDIndex&(SMAP_BD_MAX_ENTRY-1)];
		if(!((ctrl_stat=PktBdPtr->ctrl_stat)&SMAP_BD_RX_EMPTY)){
			if(ctrl_stat&(SMAP_BD_RX_INRANGE|SMAP_BD_RX_OUTRANGE|SMAP_BD_RX_FRMTOOLONG|SMAP_BD_RX_BADFCS|SMAP_BD_RX_ALIGNERR|SMAP_BD_RX_SHORTEVNT|SMAP_BD_RX_RUNTFRM|SMAP_BD_RX_OVERRUN) || PktBdPtr->length>MAX_FRAME_SIZE){
				SmapDrivPrivData->RuntimeStats.RxDroppedFrameCount++;

				if(ctrl_stat&SMAP_BD_RX_OVERRUN) SmapDrivPrivData->RuntimeStats.RxFrameOverrunCount++;
				if(ctrl_stat&(SMAP_BD_RX_INRANGE|SMAP_BD_RX_OUTRANGE|SMAP_BD_RX_FRMTOOLONG|SMAP_BD_RX_SHORTEVNT|SMAP_BD_RX_RUNTFRM)) SmapDrivPrivData->RuntimeStats.RxFrameBadLengthCount++;
				if(ctrl_stat&SMAP_BD_RX_BADFCS) SmapDrivPrivData->RuntimeStats.RxFrameBadFCSCount++;
				if(ctrl_stat&SMAP_BD_RX_ALIGNERR) SmapDrivPrivData->RuntimeStats.RxFrameBadAlignmentCount++;
			}
			else{
				length = PktBdPtr->length;

				if((pbuf=NetManNetProtStackAllocRxPacket(length, &payload))==NULL){
					break;	// Cannot continue. Stop.
				}

				CopyFromFIFO(SmapDrivPrivData->smap_regbase, payload, length, PktBdPtr->pointer);
				NetManNetProtStackEnQRxPacket(pbuf);
				NumPacketsReceived++;
			}

			SMAP_REG8(SMAP_R_RXFIFO_FRAME_DEC)=0;
			PktBdPtr->ctrl_stat=SMAP_BD_RX_EMPTY;
			SmapDrivPrivData->RxBDIndex++;
		}
		else break;
	}

	return NumPacketsReceived;
}

int SMAPSendPacket(const void *data, unsigned int length){
	int result, i, OldState;
	USE_SMAP_TX_BD;
	volatile u8 *smap_regbase;
	volatile smap_bd_t *BD_ptr;
	u16 BD_data_ptr;
	unsigned int SizeRounded;

	SaveGP();

	if(SmapDriverData.SmapIsInitialized){
		SizeRounded=(length+3)&~3;
		/*	Unlike the SONY implementation, LWIP expects packet transmission to either
			always succeed or to fail due to an unrecoverable error. This means that the driver
			should wait for transmissions to complete, if the Tx buffer is full. */
		while((SmapDriverData.NumPacketsInTx>=SMAP_BD_MAX_ENTRY) || (SmapDriverData.TxBufferSpaceAvailable<SizeRounded)){
			SetEventFlag(SmapDriverData.Dev9IntrEventFlag, SMAP_EVENT_XMIT);
			WaitEventFlag(SmapDriverData.TxEndEventFlag, 1, WEF_AND|WEF_CLEAR, NULL);
		}

		smap_regbase=SmapDriverData.smap_regbase;
		BD_data_ptr=SMAP_REG16(SMAP_R_TXFIFO_WR_PTR);
		BD_ptr=&tx_bd[SmapDriverData.TxBDIndex&0x3F];

		if((i=SmapDmaTransfer(SmapDriverData.smap_regbase, (void*)data, length, DMAC_FROM_MEM))<0){
			i=0;
		}

		for(; i<length; i+=4){
			SMAP_REG32(SMAP_R_TXFIFO_DATA)=((u32*)data)[i/4];
		}

		BD_ptr->length=length;
		BD_ptr->pointer=BD_data_ptr;
		SMAP_REG8(SMAP_R_TXFIFO_FRAME_INC)=0;
		BD_ptr->ctrl_stat=SMAP_BD_TX_READY|SMAP_BD_TX_GENFCS|SMAP_BD_TX_GENPAD;
		SmapDriverData.TxBDIndex++;

		CpuSuspendIntr(&OldState);
		SmapDriverData.NumPacketsInTx++;
		SmapDriverData.TxBufferSpaceAvailable-=SizeRounded;
		CpuResumeIntr(OldState);

		SetEventFlag(SmapDriverData.Dev9IntrEventFlag, SMAP_EVENT_XMIT);

		result=1;
	}
	else result=-1;

	RestoreGP();

	return result;
}

