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

static inline void CopyFromFIFO(volatile u8 *smap_regbase, void *buffer, unsigned int length, unsigned short int RxBdPtr){
	int i, result;

	SMAP_REG16(SMAP_R_RXFIFO_RD_PTR)=RxBdPtr;

	if((result=SmapDmaTransfer(smap_regbase, buffer, length, DMAC_TO_MEM))<0){
		result=0;
	}

	if(result<length){
		for(i=result; i<length; i+=4){
			((unsigned int *)buffer)[i/4]=SMAP_REG32(SMAP_R_RXFIFO_DATA);
		}
	}
}

inline int HandleRxIntr(struct SmapDriverData *SmapDrivPrivData){
	USE_SMAP_RX_BD;
	int NumPacketsReceived;
	volatile smap_bd_t *PktBdPtr;
	volatile u8 *smap_regbase;
	struct NetManPacketBuffer *pbuf;
	unsigned short int ctrl_stat;

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
				if((pbuf=NetManNetProtStackAllocRxPacket(PktBdPtr->length))==NULL){
					NetManNetProtStackFlushInputQueue();
					if((pbuf=NetManNetProtStackAllocRxPacket(PktBdPtr->length))==NULL) break;	// Cannot continue. Stop.
				}

				CopyFromFIFO(SmapDrivPrivData->smap_regbase, pbuf->payload, pbuf->length, PktBdPtr->pointer);
				NetManNetProtStackEnQRxPacket(pbuf);
				NumPacketsReceived++;
			}

			SMAP_REG8(SMAP_R_RXFIFO_FRAME_DEC)=0;
			PktBdPtr->ctrl_stat=SMAP_BD_RX_EMPTY;
			SmapDrivPrivData->RxBDIndex++;
		}
		else break;
	}

	NetManNetProtStackFlushInputQueue();

	return NumPacketsReceived;
}

int SMAPSendPacket(const void *data, unsigned int length){
	int result, i, OldState;
	USE_SMAP_TX_BD;
	volatile u8 *smap_regbase;
	volatile u8 *emac3_regbase;
	volatile smap_bd_t *BD_ptr;
	u16 BD_data_ptr;
	unsigned int SizeRounded;

	SaveGP();

	if(SmapDriverData.SmapIsInitialized){
		ClearEventFlag(SmapDriverData.TxEndEventFlag, ~1);

		SizeRounded=(length+3)&~3;
		while((SmapDriverData.NumPacketsInTx>=SMAP_BD_MAX_ENTRY) || (SmapDriverData.TxBufferSpaceAvailable<SizeRounded)){
			WaitEventFlag(SmapDriverData.TxEndEventFlag, 1, WEF_AND|WEF_CLEAR, NULL);
		}

		smap_regbase=SmapDriverData.smap_regbase;
		BD_data_ptr=SMAP_REG16(SMAP_R_TXFIFO_WR_PTR);
		BD_ptr=&tx_bd[SmapDriverData.TxBDIndex&0x3F];

		if((i=SmapDmaTransfer(SmapDriverData.smap_regbase, (void*)data, length, DMAC_FROM_MEM))<0){
			i=0;
		}

		for(; i<length; i+=4){
			SMAP_REG32(SMAP_R_TXFIFO_DATA)=((unsigned int *)data)[i/4];
		}

		BD_ptr->length=length;
		BD_ptr->pointer=BD_data_ptr;
		SMAP_REG8(SMAP_R_TXFIFO_FRAME_INC)=0;
		BD_ptr->ctrl_stat=SMAP_BD_TX_READY|SMAP_BD_TX_GENFCS|SMAP_BD_TX_GENPAD;	/* 0x8300 */
		SmapDriverData.TxBDIndex++;

		CpuSuspendIntr(&OldState);
		SmapDriverData.NumPacketsInTx++;
		SmapDriverData.TxBufferSpaceAvailable-=SizeRounded;
		CpuResumeIntr(OldState);

		emac3_regbase=SmapDriverData.emac3_regbase;
		SMAP_EMAC3_SET(SMAP_R_EMAC3_TxMODE0, SMAP_E3_TX_GNP_0);

		result=1;
	}
	else result=-1;

	RestoreGP();

	return result;
}

