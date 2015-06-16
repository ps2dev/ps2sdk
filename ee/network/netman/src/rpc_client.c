#include <errno.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include <malloc.h>
#include <netman.h>
#include <netman_rpc.h>

#include "rpc_client.h"

static SifRpcClientData_t NETMAN_rpc_cd;
extern void *_gp;

static unsigned char TransmitBuffer[128] ALIGNED(64);
static unsigned char ReceiveBuffer[128] ALIGNED(64);
static int NetManIOSemaID = -1, NetManTxSemaID = -1;
static void *IOPFrameBuffer = NULL;	/* On the IOP side. */

struct TxFIFOData{
	struct PacketReqs PacketReqs ALIGNED(64);
	unsigned char *FrameBuffer ALIGNED(64);
};

static struct TxFIFOData TxFIFOData1 ALIGNED(64);
static struct TxFIFOData TxFIFOData2 ALIGNED(64);
static struct TxFIFOData *CurrentTxFIFOData;

static unsigned char TxActiveBankID;
static int TxBankAccessSema = -1;
static int TxThreadID = -1;

static unsigned char TxThreadStack[0x1000] ALIGNED(128);
static void TxThread(void *arg);

static unsigned char IsInitialized=0;

static void deinitCleanup(void){
	if(TxThreadID >= 0){
		TerminateThread(TxThreadID);
		DeleteThread(TxThreadID);
		TxThreadID = -1;
	}
	if(NetManIOSemaID >= 0){
		DeleteSema(NetManIOSemaID);
		NetManIOSemaID = -1;
	}
	if(NetManTxSemaID >= 0){
		DeleteSema(NetManTxSemaID);
		NetManTxSemaID = -1;
	}
	if(TxFIFOData1.FrameBuffer != NULL){
		free(TxFIFOData1.FrameBuffer);
		TxFIFOData1.FrameBuffer = NULL;
	}
	if(TxFIFOData2.FrameBuffer != NULL){
		free(TxFIFOData2.FrameBuffer);
		TxFIFOData2.FrameBuffer = NULL;
	}
}

int NetManInitRPCClient(void){
	static const char NetManID[]="NetMan";
	int result;
	ee_sema_t SemaData;
	ee_thread_t ThreadData;

	if(!IsInitialized){
		memset(&TxFIFOData1, 0, sizeof(TxFIFOData1));
		memset(&TxFIFOData2, 0, sizeof(TxFIFOData2));

		TxFIFOData1.FrameBuffer = memalign(64, (MAX_FRAME_SIZE*NETMAN_RPC_BLOCK_SIZE+0x3F)&~0x3F);
		TxFIFOData2.FrameBuffer = memalign(64, (MAX_FRAME_SIZE*NETMAN_RPC_BLOCK_SIZE+0x3F)&~0x3F);

		if(TxFIFOData1.FrameBuffer == NULL) return -ENOMEM;
		if(TxFIFOData2.FrameBuffer == NULL){
			deinitCleanup();
			return -ENOMEM;
		}

		SemaData.max_count=1;
		SemaData.init_count=1;
		SemaData.option=(unsigned int)NetManID;
		SemaData.attr=0;
		if((NetManIOSemaID=CreateSema(&SemaData)) < 0){
			deinitCleanup();
			return NetManIOSemaID;
		}

		TxActiveBankID=0;
		CurrentTxFIFOData=UNCACHED_SEG(&TxFIFOData1);

		SemaData.max_count=1;
		SemaData.init_count=1;
		SemaData.option=(unsigned int)NetManID;
		SemaData.attr=0;
		if((TxBankAccessSema=CreateSema(&SemaData)) < 0){
			deinitCleanup();
			return TxBankAccessSema;
		}

		SemaData.max_count=1;
		SemaData.init_count=1;
		SemaData.option=(unsigned int)NetManID;
		SemaData.attr=0;
		if((NetManTxSemaID=CreateSema(&SemaData)) < 0){
			deinitCleanup();
			return NetManTxSemaID;
		}

		ThreadData.func=&TxThread;
		ThreadData.stack=TxThreadStack;
		ThreadData.stack_size=sizeof(TxThreadStack);
		ThreadData.gp_reg=&_gp;
		ThreadData.initial_priority=0x58;
		ThreadData.attr=ThreadData.option=0;

		if((TxThreadID=CreateThread(&ThreadData)) < 0){
			deinitCleanup();
			return TxThreadID;
		}

		if((result = StartThread(TxThreadID, NULL)) < 0){
			deinitCleanup();
			return result;
		}

		while((SifBindRpc(&NETMAN_rpc_cd, NETMAN_RPC_NUMBER, 0)<0)||(NETMAN_rpc_cd.server==NULL)){
			nopdelay();
			nopdelay();
			nopdelay();
			nopdelay();
		}

		if((result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_INIT, 0, NULL, 0, ReceiveBuffer, sizeof(int), NULL, NULL))>=0){
			if((result=*(int*)ReceiveBuffer) == 0)
				IsInitialized=1;
			else
				deinitCleanup();
		}else{
			deinitCleanup();
		}
	}
	else result=0;

	return result;
}

int NetManRPCRegisterNetworkStack(void){
	int result;

	WaitSema(NetManIOSemaID);

	if((result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_REG_NETWORK_STACK, 0, NULL, 0, ReceiveBuffer, sizeof(struct NetManRegNetworkStackResult), NULL, NULL))>=0){
		if((result=((struct NetManRegNetworkStackResult*)ReceiveBuffer)->result) == 0){
			IOPFrameBuffer=((struct NetManRegNetworkStackResult*)ReceiveBuffer)->FrameBuffer;
		}
	}

	SignalSema(NetManIOSemaID);

	return result;
}

int NetManRPCUnregisterNetworkStack(void){
	int result;

	WaitSema(NetManIOSemaID);

	result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_UNREG_NETWORK_STACK, 0, NULL, 0, NULL, 0, NULL, NULL);
	IOPFrameBuffer = NULL;

	SignalSema(NetManIOSemaID);

	return result;
}

void NetManDeinitRPCClient(void){
	if(IsInitialized){
		WaitSema(NetManIOSemaID);

		SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_DEINIT, 0, NULL, 0, NULL, 0, NULL, NULL);
		deinitCleanup();

		IsInitialized=0;
	}
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
		result=((struct NetManIoctlResult*)ReceiveBuffer)->result;
		memcpy(output, ((struct NetManIoctlResult*)ReceiveBuffer)->output, length);
	}

	SignalSema(NetManIOSemaID);

	return result;
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
				CurrentTxFIFOData=UNCACHED_SEG(&TxFIFOData2);
				TxActiveBankID=1;
			}
			else{
				CurrentTxFIFOData=UNCACHED_SEG(&TxFIFOData1);
				TxActiveBankID=0;
			}

			SignalSema(TxBankAccessSema);

			dmat.src=TxFIFODataToTransmit->FrameBuffer;
			dmat.dest=IOPFrameBuffer;
			dmat.size=TxFIFODataToTransmit->PacketReqs.TotalLength;
			dmat.attr=0;

			while((dmat_id=SifSetDma(&dmat, 1))==0){};

			WaitSema(NetManIOSemaID);
			SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_SEND_PACKETS, SIF_RPC_M_NOWBDC, &TxFIFODataToTransmit->PacketReqs, 8+sizeof(struct PacketTag)*TxFIFODataToTransmit->PacketReqs.NumPackets, NULL, 0, NULL, NULL);
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

	if(IOPFrameBuffer != NULL){
		WaitSema(NetManTxSemaID);

		NetmanTxWaitingThread=GetThreadId();

		WaitSema(TxBankAccessSema);

		//Check is there is space in the current Tx FIFO. If not, wait for the Tx thread to empty out the other FIFO. */
		while(CurrentTxFIFOData->PacketReqs.NumPackets+1>NETMAN_RPC_BLOCK_SIZE || (CurrentTxFIFOData->PacketReqs.TotalLength + length > MAX_FRAME_SIZE*NETMAN_RPC_BLOCK_SIZE)){
			SignalSema(TxBankAccessSema);
			WakeupThread(TxThreadID);
			SleepThread();
			WaitSema(TxBankAccessSema);
		}

		memcpy((void*)((unsigned int)&CurrentTxFIFOData->FrameBuffer[CurrentTxFIFOData->PacketReqs.TotalLength] | 0x30000000), packet, length);
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
	}else{
		return -1;
	}
}

int NetManSetMainIF(const char *name){
	int result;

	WaitSema(NetManIOSemaID);

	strncpy(TransmitBuffer, name, NETMAN_NETIF_NAME_MAX_LEN);
	TransmitBuffer[NETMAN_NETIF_NAME_MAX_LEN-1] = '\0';
	if((result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_SET_MAIN_NETIF, 0, TransmitBuffer, NETMAN_NETIF_NAME_MAX_LEN, ReceiveBuffer, sizeof(int), NULL, NULL))>=0){
		result=*(int*)ReceiveBuffer;
	}

	SignalSema(NetManIOSemaID);

	return result;
}

int NetManQueryMainIF(char *name){
	int result;

	WaitSema(NetManIOSemaID);

	if((result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_QUERY_MAIN_NETIF, 0, NULL, 0, ReceiveBuffer, sizeof(struct NetManQueryMainNetIFResult), NULL, NULL))>=0){
		if((result=((struct NetManQueryMainNetIFResult*)ReceiveBuffer)->result) == 0){
			strncpy(name, ((struct NetManQueryMainNetIFResult*)ReceiveBuffer)->name, NETMAN_NETIF_NAME_MAX_LEN);
			name[NETMAN_NETIF_NAME_MAX_LEN-1] = '\0';
		}
	}

	SignalSema(NetManIOSemaID);

	return result;
}

int NetManSetLinkMode(int mode){
	int result;

	WaitSema(NetManIOSemaID);

	*(int*)TransmitBuffer = mode;
	if((result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_SET_LINK_MODE, 0, TransmitBuffer, sizeof(int), ReceiveBuffer, sizeof(int), NULL, NULL))>=0){
		result=*(int*)ReceiveBuffer;
	}

	SignalSema(NetManIOSemaID);

	return result;
}
