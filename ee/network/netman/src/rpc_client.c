#include <errno.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include <netman_rpc.h>

#include "rpc_client.h"

static SifRpcClientData_t NETMAN_rpc_cd;
extern void *_gp;

static unsigned char TransmitBuffer[128] ALIGNED(64);
static unsigned char ReceiveBuffer[128] ALIGNED(64);
static int NetManIOSemaID, NetManTxSemaID;
static void *TxFrameTagBuffer;	/* On the IOP side. */
//static struct AlignmentData AlignmentData ALIGNED(64);

struct TxFIFOData{
	unsigned char FrameBuffer[NETMAN_RPC_BLOCK_SIZE*MAX_FRAME_SIZE] ALIGNED(64);
	struct PacketReqs PacketReqs;
};

static struct TxFIFOData TxFIFOData1 ALIGNED(64);
static struct TxFIFOData TxFIFOData2 ALIGNED(64);
static struct TxFIFOData *CurrentTxFIFOData;

static unsigned char TxActiveBankID;
static int TxBankAccessSema;
static int TxThreadID;

static unsigned char TxThreadStack[0x1000] ALIGNED(128);
static void TxThread(void *arg);

static unsigned char IsInitialized=0;

/* static void RpcEndXferAlignmentFunc(void *AlignmentData){
	memcpy(((struct AlignmentData *)UNCACHED_SEG(AlignmentData))->buffer1Address, ((struct AlignmentData *)UNCACHED_SEG(AlignmentData))->buffer1, ((struct AlignmentData *)UNCACHED_SEG(AlignmentData))->buffer1_len);
	memcpy(((struct AlignmentData *)UNCACHED_SEG(AlignmentData))->buffer2Address, ((struct AlignmentData *)UNCACHED_SEG(AlignmentData))->buffer2, ((struct AlignmentData *)UNCACHED_SEG(AlignmentData))->buffer2_len);
} */

int NetManInitRPCClient(void){
	int result;
	ee_sema_t SemaData;
	ee_thread_t ThreadData;

	if(!IsInitialized){
		SemaData.max_count=1;
		SemaData.init_count=1;
		SemaData.option=SemaData.attr=0;
		NetManIOSemaID=CreateSema(&SemaData);

		TxActiveBankID=0;
		memset(UNCACHED_SEG(&TxFIFOData1), 0, sizeof(TxFIFOData1));
		memset(UNCACHED_SEG(&TxFIFOData2), 0, sizeof(TxFIFOData2));
		CurrentTxFIFOData=UNCACHED_SEG(&TxFIFOData1);

		SemaData.max_count=1;
		SemaData.init_count=1;
		SemaData.option=SemaData.attr=0;
		TxBankAccessSema=CreateSema(&SemaData);

		SemaData.max_count=1;
		SemaData.init_count=1;
		SemaData.option=SemaData.attr=0;
		NetManTxSemaID=CreateSema(&SemaData);

		ThreadData.func=&TxThread;
		ThreadData.stack=TxThreadStack;
		ThreadData.stack_size=sizeof(TxThreadStack);
		ThreadData.gp_reg=&_gp;
		ThreadData.initial_priority=0x18;
		ThreadData.attr=ThreadData.option=0;

		StartThread(TxThreadID=CreateThread(&ThreadData), NULL);

		while((SifBindRpc(&NETMAN_rpc_cd, NETMAN_RPC_NUMBER, 0)<0)||(NETMAN_rpc_cd.server==NULL)){
			nopdelay();
			nopdelay();
			nopdelay();
			nopdelay();
		}

//		((struct NetManInit*)TransmitBuffer)->AlignmentData=&AlignmentData;
		if((result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_INIT, 0, TransmitBuffer, sizeof(struct NetManInit), ReceiveBuffer, sizeof(struct NetManInitResult), NULL, NULL))>=0){
			result=((struct NetManInitResult*)ReceiveBuffer)->result;
			TxFrameTagBuffer=((struct NetManInitResult*)ReceiveBuffer)->FrameTagBuffer;
		}

		IsInitialized=1;
	}
	else result=0;

	return result;
}

void NetManDeinitRPCClient(void){
	WaitSema(NetManIOSemaID);

	SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_DEINIT, 0, NULL, 0, ReceiveBuffer, sizeof(struct NetManInitResult), NULL, NULL);
	TerminateThread(TxThreadID);
	DeleteThread(TxThreadID);
	DeleteSema(NetManIOSemaID);
	DeleteSema(NetManTxSemaID);

	IsInitialized=0;
}

int NetManRpcIoctl(unsigned int command, void *args, unsigned int args_len, void *output, unsigned int length){
	int result;
/*	void *AlignedOutput;
	int AlignedSize; */

	WaitSema(NetManIOSemaID);

	((struct NetManIoctl*)UNCACHED_SEG(TransmitBuffer))->command=command;
	memcpy(((struct NetManIoctl*)UNCACHED_SEG(TransmitBuffer))->args, args, args_len);
	((struct NetManIoctl*)UNCACHED_SEG(TransmitBuffer))->args_len=args_len;
	((struct NetManIoctl*)UNCACHED_SEG(TransmitBuffer))->output=output;
	((struct NetManIoctl*)UNCACHED_SEG(TransmitBuffer))->length=length;

/*	!!! This entire system is not working. And I don't know why.
	AlignedOutput=(void*)(((unsigned int)output+0x3F)&~0x3F);
	AlignedSize=length-((unsigned int)AlignedOutput-(unsigned int)output);
	AlignedSize-=(AlignedSize&0x3F);

	if(AlignedSize>0){
		SifWriteBackDCache(AlignedOutput, AlignedSize);
	} */

	if((result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_IOCTL, SIF_RPC_M_NOWBDC, TransmitBuffer, sizeof(struct NetManIoctl), ReceiveBuffer, sizeof(struct NetManIoctlResult), NULL, NULL))>=0){
		result=((struct NetManIoctlResult*)UNCACHED_SEG(ReceiveBuffer))->result;
		memcpy(output, ((struct NetManIoctlResult*)UNCACHED_SEG(ReceiveBuffer))->output, length);
	}

	SignalSema(NetManIOSemaID);

	return result;
}

static int NetmanTxWaitingThread=-1;

static void TxThread(void *arg){
	struct TxFIFOData *TxFIFODataToTransmit;
	SifDmaTransfer_t dmat;
	int dmat_id;

	while(1){
		SleepThread();

		WaitSema(TxBankAccessSema);

		if(CurrentTxFIFOData->PacketReqs.NumPackets>0){
			// Switch banks
			TxFIFODataToTransmit=CurrentTxFIFOData;
			if(TxActiveBankID==0){
				CurrentTxFIFOData=UNCACHED_SEG(&TxFIFOData2);
				TxActiveBankID=1;
			}
			else{
				CurrentTxFIFOData=UNCACHED_SEG(&TxFIFOData1);
				TxActiveBankID=0;
			}

			SignalSema(TxBankAccessSema);

			dmat.src=(void*)((unsigned int)&TxFIFODataToTransmit->PacketReqs&0x0FFFFFC0);
			dmat.dest=TxFrameTagBuffer;
			dmat.size=8+sizeof(struct PacketTag)*TxFIFODataToTransmit->PacketReqs.NumPackets;
			dmat.attr=0;

			while((dmat_id=SifSetDma(&dmat, 1))==0){};

			WaitSema(NetManIOSemaID);
			SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_SEND_PACKETS, SIF_RPC_M_NOWBDC, TxFIFODataToTransmit->FrameBuffer, TxFIFODataToTransmit->PacketReqs.TotalLength, NULL, 0, NULL, NULL);
			SignalSema(NetManIOSemaID);

			TxFIFODataToTransmit->PacketReqs.NumPackets=0;
			TxFIFODataToTransmit->PacketReqs.TotalLength=0;
		}
		else SignalSema(TxBankAccessSema);

		if(NetmanTxWaitingThread>0) WakeupThread(NetmanTxWaitingThread);
	}
}

int NetManRpcNetIFSendPacket(const void *packet, unsigned int length){
	struct PacketTag *PacketTag;

	WaitSema(NetManTxSemaID);

	NetmanTxWaitingThread=GetThreadId();

	WaitSema(TxBankAccessSema);

	//Check is there is space in the current Tx FIFO. If not, wait for the Tx thread to empty out the other FIFO. */
	while(CurrentTxFIFOData->PacketReqs.NumPackets>=NETMAN_RPC_BLOCK_SIZE){
		SignalSema(TxBankAccessSema);
		WakeupThread(TxThreadID);
		SleepThread();
		WaitSema(TxBankAccessSema);
	}

	memcpy(&CurrentTxFIFOData->FrameBuffer[CurrentTxFIFOData->PacketReqs.TotalLength], packet, length);
	PacketTag=&CurrentTxFIFOData->PacketReqs.tags[CurrentTxFIFOData->PacketReqs.NumPackets];
	PacketTag->offset=CurrentTxFIFOData->PacketReqs.TotalLength;
	PacketTag->length=length;

	CurrentTxFIFOData->PacketReqs.TotalLength+=(length+3)&~3;
	CurrentTxFIFOData->PacketReqs.NumPackets++;

	WakeupThread(TxThreadID);

	SignalSema(TxBankAccessSema);

	NetmanTxWaitingThread=-1;

	SignalSema(NetManTxSemaID);

	return 0;
}
