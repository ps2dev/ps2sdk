#include <errno.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include <netman.h>
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

int NetManInitRPCClient(void){
	static const char NetManID[]="NetMan";
	int result;
	ee_sema_t SemaData;
	ee_thread_t ThreadData;

	if(!IsInitialized){
		SemaData.max_count=1;
		SemaData.init_count=1;
		SemaData.option=(unsigned int)NetManID;
		SemaData.attr=0;
		NetManIOSemaID=CreateSema(&SemaData);

		TxActiveBankID=0;
		memset(&TxFIFOData1, 0, sizeof(TxFIFOData1));
		memset(&TxFIFOData2, 0, sizeof(TxFIFOData2));
		CurrentTxFIFOData=UNCACHED_SEG(&TxFIFOData1);

		SemaData.max_count=1;
		SemaData.init_count=1;
		SemaData.option=(unsigned int)NetManID;
		SemaData.attr=0;
		TxBankAccessSema=CreateSema(&SemaData);

		SemaData.max_count=1;
		SemaData.init_count=1;
		SemaData.option=(unsigned int)NetManID;
		SemaData.attr=0;
		NetManTxSemaID=CreateSema(&SemaData);

		ThreadData.func=&TxThread;
		ThreadData.stack=TxThreadStack;
		ThreadData.stack_size=sizeof(TxThreadStack);
		ThreadData.gp_reg=&_gp;
		ThreadData.initial_priority=0x58;
		ThreadData.attr=ThreadData.option=0;

		StartThread(TxThreadID=CreateThread(&ThreadData), NULL);

		while((SifBindRpc(&NETMAN_rpc_cd, NETMAN_RPC_NUMBER, 0)<0)||(NETMAN_rpc_cd.server==NULL)){
			nopdelay();
			nopdelay();
			nopdelay();
			nopdelay();
		}

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
	struct NetManIoctl *IoctlArgs=(struct NetManIoctl*)TransmitBuffer;

	WaitSema(NetManIOSemaID);

	IoctlArgs->command=command;
	memcpy(IoctlArgs->args, args, args_len);
	IoctlArgs->args_len=args_len;
	IoctlArgs->output=output;
	IoctlArgs->length=length;

	if((result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_IOCTL, 0, TransmitBuffer, sizeof(struct NetManIoctl), ReceiveBuffer, sizeof(struct NetManIoctlResult), NULL, NULL))>=0){
		result=((struct NetManIoctlResult*)UNCACHED_SEG(ReceiveBuffer))->result;
		memcpy(output, ((struct NetManIoctlResult*)UNCACHED_SEG(ReceiveBuffer))->output, length);
	}

	SignalSema(NetManIOSemaID);

	return result;
}

int NetManNetIFSetLinkMode(int mode){
	return NetManRpcIoctl(NETMAN_NETIF_IOCTL_ETH_SET_LINK_MODE, &mode, sizeof(mode), NULL, 0);
}

static volatile int NetmanTxWaitingThread=-1;

static void TxThread(void *arg){
	struct TxFIFOData *TxFIFODataToTransmit;
	SifDmaTransfer_t dmat;
	int dmat_id, ThreadToWakeUp;

	while(1){
		SleepThread();

		WaitSema(TxBankAccessSema);

		if(CurrentTxFIFOData->PacketReqs.NumPackets>0){
			// Switch banks
			TxFIFODataToTransmit=CurrentTxFIFOData;
			if(TxActiveBankID==0){
				CurrentTxFIFOData=&TxFIFOData2;
				TxActiveBankID=1;
			}
			else{
				CurrentTxFIFOData=&TxFIFOData1;
				TxActiveBankID=0;
			}

			SignalSema(TxBankAccessSema);

			SifWriteBackDCache(&TxFIFODataToTransmit->PacketReqs, sizeof(TxFIFODataToTransmit->PacketReqs));

			dmat.src=&TxFIFODataToTransmit->PacketReqs;
			dmat.dest=TxFrameTagBuffer;
			dmat.size=8+sizeof(struct PacketTag)*TxFIFODataToTransmit->PacketReqs.NumPackets;
			dmat.attr=0;

			while((dmat_id=SifSetDma(&dmat, 1))==0){};

			WaitSema(NetManIOSemaID);
			SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_SEND_PACKETS, 0, TxFIFODataToTransmit->FrameBuffer, TxFIFODataToTransmit->PacketReqs.TotalLength, NULL, 0, NULL, NULL);
			SignalSema(NetManIOSemaID);

			TxFIFODataToTransmit->PacketReqs.NumPackets=0;
			TxFIFODataToTransmit->PacketReqs.TotalLength=0;
		}
		else SignalSema(TxBankAccessSema);

		if(NetmanTxWaitingThread>=0){
			ThreadToWakeUp=NetmanTxWaitingThread;
			NetmanTxWaitingThread=-1;	//To prevent a race condition from occurring, invalidate NetmanTxWaitingThread before invoking WakeupThread.
			WakeupThread(ThreadToWakeUp);
		}
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
