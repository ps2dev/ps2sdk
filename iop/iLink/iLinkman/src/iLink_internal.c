/*	iLink_internal.c
 *	Purpose:	Contains the functions that are used internally by the iLinkman driver.
 *
 *	Last Updated:	2012/02/28
 *	Programmer:	SP193
 */

#include <dmacman.h>
#include <intrman.h>
#include <stdio.h>
#include <sysclib.h>
#include <sysmem.h>
#include <thbase.h>
#include <thevent.h>
#include <cdvdman.h>

#include "iLinkman.h"
#include "iLink_internal.h"

extern struct ILINKMemMap *ILINKRegisterBase;
extern struct DMAChannelRegBlock *iLinkDMACRegs;
extern struct DMARRegBlock *iLinkDMARRegs;

extern void (*CallBackFunction)(int reason, unsigned int offset, unsigned int size);

int GenerationNumber;
int nNodes;
int NodeCapabilities;
struct TransactionContextData TransactionContexts[10];
extern int IntrEventFlag;

extern const void *RequestHandlers[];

void UBUFThread(void *arg){
	void (*RequestResponseHandler)(unsigned int header, volatile unsigned int *buffer, unsigned int nQuads);
	unsigned int data, DataBufferLevel;
	u32 ef_bits;

	while(1){
		WaitEventFlag(IntrEventFlag, iLinkEventBusReset|iLinkEventURx, WEF_OR, &ef_bits);
		ClearEventFlag(IntrEventFlag, ~(iLinkEventBusReset|iLinkEventURx));

		while((DataBufferLevel=ILINKRegisterBase->ubufReceiveLevel)>0){
			data=ILINKRegisterBase->ubufReceive;
			iDEBUG_PRINTF("Handling UBUF data... handler: 0x%02lx.\n", (data>>4)&0xF);
			RequestResponseHandler=RequestHandlers[(data>>4)&0xF];
			RequestResponseHandler(data, (volatile unsigned int *)&ILINKRegisterBase->ubufReceive, DataBufferLevel-1);
		}
	}
}

int GetConsoleIDs(u64 *guid, char *ModelName){
	u64 ilink_id;
	u32 stat;
	int result;

	if(sceCdRI((u8*)&ilink_id, &stat)<0 || stat!=0){
		result=-1;
		DEBUG_PRINTF("Error reading console/ILINK ID.\n");
	}
	else{
		*guid=(ilink_id&0xFFFFFFFF00000000) | ((*(u8*)&ilink_id)|0x08004600);
		if(sceCdRM(ModelName, &stat)<0 || stat!=0){
			result=-1;
			DEBUG_PRINTF("Error reading console model name.\n");
		}

		result=0;
	}

	return result;
}

void iLinkDisableIntr(void){
	ILINKRegisterBase->intr0Mask=0;
	ILINKRegisterBase->intr1Mask=0;
	ILINKRegisterBase->intr2Mask=0;

	/* Set all interrupt bits, to acknowledge any pending interrupts that there may be. */
	ILINKRegisterBase->intr0=0xFFFFFFFF;
	ILINKRegisterBase->intr1=0xFFFFFFFF;
	ILINKRegisterBase->intr2=0xFFFFFFFF;
}

void iLinkEnableIntr(void){
	/* Enable the interrupt events */
	ILINKRegisterBase->intr0Mask=iLink_INTR0_CmdRst|iLink_INTR0_PhyRst|iLink_INTR0_RetEx|iLink_INTR0_STO|iLink_INTR0_PBCntR|iLink_INTR0_UResp|iLink_INTR0_URx;
#ifdef REQ_CHECK_DMAC_STAT
	ILINKRegisterBase->intr0Mask|=iLink_INTR0_DRFR;
#endif

#ifdef REQ_CHECK_ERRORS
	ILINKRegisterBase->intr0Mask|=(iLink_INTR0_HdrErr|iLink_INTR0_SntBsyAck); /* For debugging purposes */
#endif
	ILINKRegisterBase->intr1Mask=iLink_INTR1_UTD;
}

static void ShutDownDMAChannels(void){
	/* Stop all DMA-related activities */
	ILINKRegisterBase->dmaCtrlSR0=ILINKRegisterBase->dmaCtrlSR1=0;

	/* Stop all DMA-related activities */
	iLinkDMACRegs[0].chcr=iLinkDMACRegs[1].chcr=iLinkDMACRegs[2].chcr=0;

	/* Disable all DMAC channels connected to the i.Link hardware.
		(Disable DMA channels 13, 14 and 15) */
	dmac_disable(IOP_DMAC_FDMA0);
	dmac_disable(IOP_DMAC_FDMA1);
	dmac_disable(IOP_DMAC_FDMA2);
}

int iLinkResetHW(void){
	ShutDownDMAChannels();

    	/* Turn off, and then turn back on the LINK and PHY. If this is not done, the iLink hardware might not function correctly on some consoles. :( */
	ILINKRegisterBase->UnknownRegister7C=0x40;	/* Shut down. */
	DelayThread(400000);
	ILINKRegisterBase->UnknownRegister7C=0;

	/* If the LINK is off, switch it on. */
	if(!(ILINKRegisterBase->ctrl2&iLink_CTRL2_LPSEn)) ILINKRegisterBase->ctrl2=iLink_CTRL2_LPSEn;

	/* Wait for the clock to stabilize. */
	while(!(ILINKRegisterBase->ctrl2&iLink_CTRL2_SOK)) DelayThread(50);

	return 0;
}

void iLinkShutdownHW(void){
	ShutDownDMAChannels();
	ILINKRegisterBase->ctrl2&=(~iLink_CTRL2_LPSEn);	/* Set the LINK to enter standby mode. */
}

void iLinkHWInitialize(void){
	iLinkPHY_SetLCTRL(0);

	/* Configure the Control 0 register, resetting the transmitter and receiver at the same time. */
//	ILINKRegisterBase->ctrl0=iLink_CTRL0_TxRst|iLink_CTRL0_RxRst|iLink_CTRL0_RcvSelfID|iLink_CTRL0_BusIDRst|iLink_CTRL0_CycTmrEn|iLink_CTRL0_RetLim(0x0F)|iLink_CTRL0_RSP0|iLink_CTRL0_DELim(3);
	ILINKRegisterBase->ctrl0=iLink_CTRL0_TxRst|iLink_CTRL0_RxRst|iLink_CTRL0_RcvSelfID|iLink_CTRL0_BusIDRst|iLink_CTRL0_CycTmrEn|iLink_CTRL0_RetLim(0x0F)|iLink_CTRL0_DELim(3);
	while(ILINKRegisterBase->ctrl0&(iLink_CTRL0_RxRst|iLink_CTRL0_TxRst)){};

	ILINKRegisterBase->ubufTransmitClear=0;	/* Clear UBUF Tx FIFO. */
	ILINKRegisterBase->ubufReceiveClear=0;	/* Clear UBUF Rx FIFO. */

	ILINKRegisterBase->dbufFIFO_lvlR0=DBUF_FIFO_RESET_TX|DBUF_FIFO_RESET_RX;	/* Reset DBUF FIFO 0 */
	ILINKRegisterBase->dbufFIFO_lvlR1=DBUF_FIFO_RESET_TX|DBUF_FIFO_RESET_RX;	/* Reset DBUF FIFO 1 */

	iLinkInitPHT();
	iLinkEnableIntr();

	iLinkWritePhy(5, iLinkReadPhy(5)|REG05_EN_ACCL|REG05_EN_MULTI);	/* Enable supported P1394A-2000 enhancements. */

	/* Enable DMA channels 13 and 15. */
	dmac_enable(IOP_DMAC_FDMA0);
	dmac_enable(IOP_DMAC_FDMA2);

	GenerationNumber=0;
	NodeCapabilities=0;
}

void iLinkEnableCMaster(void){
	ILINKRegisterBase->ctrl0|=iLink_CTRL0_CMstr;
}

void iLinkBusEnable(void){
	ILINKRegisterBase->ctrl0|=(iLink_CTRL0_RxEn|iLink_CTRL0_TxEn);	/* Enable transmitter and receiver. */
}

void *malloc(unsigned int nBytes){
	int OldState;
	void *result;

	CpuSuspendIntr(&OldState);
	result=AllocSysMemory(ALLOC_FIRST, nBytes, NULL);
	CpuResumeIntr(OldState);

	return result;
}

void free(void *buffer){
	int OldState;

	CpuSuspendIntr(&OldState);
	FreeSysMemory(buffer);
	CpuResumeIntr(OldState);
}
