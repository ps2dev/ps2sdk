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

static union {
	s32 mode;
	struct NetManIoctl IoctlArgs;
	char netifName[NETMAN_NETIF_NAME_MAX_LEN];
	struct NetManRegNetworkStack NetStack;
	u8 buffer[128];
}TransmitBuffer ALIGNED(64);

static union {
	s32 result;
	struct NetManRegNetworkStackResult NetStackResult;
	struct NetManIoctlResult IoctlResult;
	struct NetManQueryMainNetIFResult QueryMainNetIFResult;
	u8 buffer[128];
}ReceiveBuffer ALIGNED(64);

static int NetManIOSemaID = -1, NETMAN_Tx_threadID = -1;
static unsigned char NETMAN_Tx_ThreadStack[0x1000] ALIGNED(16);

static unsigned short int IOPFrameBufferWrPtr;
static u8 *IOPFrameBuffer = NULL;	/* On the IOP side. */
static struct NetManBD *IOPFrameBufferStatus = NULL;
static struct NetManBD *FrameBufferStatus = NULL;

static unsigned char IsInitialized=0, IsProcessingTx;

static void deinitCleanup(void)
{
	if(NetManIOSemaID >= 0)
	{
		DeleteSema(NetManIOSemaID);
		NetManIOSemaID = -1;
	}
	if(NETMAN_Tx_threadID >= 0)
	{
		TerminateThread(NETMAN_Tx_threadID);
		DeleteThread(NETMAN_Tx_threadID);
		NETMAN_Tx_threadID = -1;
	}
}

static void NETMAN_TxThread(void *arg);

int NetManInitRPCClient(void){
	static const char NetManID[]="NetMan";
	int result;
	ee_sema_t SemaData;
	ee_thread_t thread;

	if(!IsInitialized)
	{
		SemaData.max_count=1;
		SemaData.init_count=1;
		SemaData.option=(u32)NetManID;
		SemaData.attr=0;
		if((NetManIOSemaID=CreateSema(&SemaData)) < 0)
		{
			deinitCleanup();
			return NetManIOSemaID;
		}

		thread.func=&NETMAN_TxThread;
		thread.stack=NETMAN_Tx_ThreadStack;
		thread.stack_size=sizeof(NETMAN_Tx_ThreadStack);
		thread.gp_reg=&_gp;
		thread.initial_priority=0x56;	/* Should be given a higher priority than the protocol stack, so that it can dump frames in the EE and return. */
		thread.attr=thread.option=0;

		if((NETMAN_Tx_threadID=CreateThread(&thread)) >= 0)
		{
			IsProcessingTx = 0;
			StartThread(NETMAN_Tx_threadID, NULL);
		} else {
			deinitCleanup();
			return NETMAN_Tx_threadID;
		}

		while((SifBindRpc(&NETMAN_rpc_cd, NETMAN_RPC_NUMBER, 0)<0)||(NETMAN_rpc_cd.server==NULL))
			nopdelay();

		if((result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_INIT, 0, NULL, 0, &ReceiveBuffer, sizeof(s32), NULL, NULL))>=0)
		{
			if((result=ReceiveBuffer.result) == 0)
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

int NetManRPCRegisterNetworkStack(void)
{
	int result;

	WaitSema(NetManIOSemaID);

	if(FrameBufferStatus == NULL) FrameBufferStatus = memalign(64, NETMAN_RPC_BLOCK_SIZE * sizeof(struct NetManBD));

	if(FrameBufferStatus != NULL)
	{
		memset(UNCACHED_SEG(FrameBufferStatus), 0, NETMAN_RPC_BLOCK_SIZE * sizeof(struct NetManBD));
		TransmitBuffer.NetStack.FrameBufferStatus = FrameBufferStatus;
		IOPFrameBufferWrPtr = 0;

		if((result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_REG_NETWORK_STACK, 0, &TransmitBuffer, sizeof(struct NetManRegNetworkStack), &ReceiveBuffer, sizeof(struct NetManRegNetworkStackResult), NULL, NULL))>=0)
		{
			if((result=ReceiveBuffer.NetStackResult.result) == 0)
			{
				IOPFrameBuffer = ReceiveBuffer.NetStackResult.FrameBuffer;
				IOPFrameBufferStatus = ReceiveBuffer.NetStackResult.FrameBufferStatus;
			}
		}
	}
	else
	{
		result = -ENOMEM;
	}

	SignalSema(NetManIOSemaID);

	return result;
}

int NetManRPCUnregisterNetworkStack(void)
{
	int result;

	WaitSema(NetManIOSemaID);

	result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_UNREG_NETWORK_STACK, 0, NULL, 0, NULL, 0, NULL, NULL);
	IOPFrameBuffer = NULL;
	IOPFrameBufferWrPtr = 0;

	free(FrameBufferStatus);
	FrameBufferStatus = NULL;

	SignalSema(NetManIOSemaID);

	return result;
}

void NetManDeinitRPCClient(void)
{
	if(IsInitialized)
	{
		WaitSema(NetManIOSemaID);

		SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_DEINIT, 0, NULL, 0, NULL, 0, NULL, NULL);
		deinitCleanup();

		IsInitialized=0;
	}
}

int NetManRpcIoctl(unsigned int command, void *args, unsigned int args_len, void *output, unsigned int length)
{
	int result;
	struct NetManIoctl *IoctlArgs=&TransmitBuffer.IoctlArgs;

	WaitSema(NetManIOSemaID);

	IoctlArgs->command=command;
	memcpy(IoctlArgs->args, args, args_len);
	IoctlArgs->args_len=args_len;
	IoctlArgs->output=output;
	IoctlArgs->length=length;

	if((result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_IOCTL, 0, &TransmitBuffer, sizeof(struct NetManIoctl), &ReceiveBuffer, sizeof(struct NetManIoctlResult), NULL, NULL))>=0)
	{
		result=ReceiveBuffer.IoctlResult.result;
		memcpy(output, ReceiveBuffer.IoctlResult.output, length);
	}

	SignalSema(NetManIOSemaID);

	return result;
}

static void NETMAN_TxThread(void *arg)
{
	static SifCmdHeader_t cmd ALIGNED(64);
	SifDmaTransfer_t dmat[2];
	struct NetManPktCmd *npcmd;
	int dmat_id, length, unaligned, unalignedCache, NumTx;
	void *payload, *payloadAligned, *payloadCacheAligned;
	volatile struct NetManBD *bd, *bdNext;

	while(1)
	{
		SleepThread();

		NumTx = 0;
		while((length = NetManTxPacketNext(&payload)) > 0)
		{
			IsProcessingTx = 1;

			//Write back D-cache, before performing a DMA transfer.
			unaligned = (int)((u32)payload & 15);
			unalignedCache = (int)((u32)payload & 63);
			payloadAligned = (void*)((u32)payload & ~15);
			payloadCacheAligned = (void*)((u32)payload & ~63);
			SifWriteBackDCache(payloadCacheAligned, (length + unalignedCache + 63) & ~63);

			do {
				//Wait for a spot to be freed up.
				bd = UNCACHED_SEG(&FrameBufferStatus[IOPFrameBufferWrPtr]);
				while(bd->length != 0){}

				//Transfer to IOP RAM
				//Determine mode of transfer.
				bdNext = UNCACHED_SEG(&FrameBufferStatus[(IOPFrameBufferWrPtr + 1) % NETMAN_RPC_BLOCK_SIZE]);
				if((NumTx + 1) >= NETMAN_FRAME_GROUP_SIZE || bdNext->length == 0)
				{
					//Prepare SIFCMD packet
					//Record the frame length.
					npcmd = (struct NetManPktCmd*)&cmd.opt;
					npcmd->length = length;
					npcmd->offset = unaligned;
					npcmd->id = IOPFrameBufferWrPtr;

					while((dmat_id = SifSendCmd(NETMAN_SIFCMD_ID, &cmd, sizeof(SifCmdHeader_t),
									(void*)payloadAligned,
									(void*)&IOPFrameBuffer[IOPFrameBufferWrPtr * NETMAN_MAX_FRAME_SIZE],
									(length + unaligned + 15) & ~15)) == 0){ };
				} else {
					//Record the frame length.
					bd->length = length;
					bd->offset = unaligned;

					//Normal DMA transfer
					dmat[0].src = (void*)payloadAligned;
					dmat[0].dest = (void*)&IOPFrameBuffer[IOPFrameBufferWrPtr * NETMAN_MAX_FRAME_SIZE];
					dmat[0].size = (length + unaligned + 15) & ~15;
					dmat[0].attr = 0;
					dmat[1].src = (void*)&FrameBufferStatus[IOPFrameBufferWrPtr];
					dmat[1].dest = (void*)&IOPFrameBufferStatus[IOPFrameBufferWrPtr];
					dmat[1].size = sizeof(struct NetManBD);
					dmat[1].attr = 0;

					while((dmat_id = SifSetDma(dmat, 2)) == 0){ };
				}

				//Increase write pointer by one position.
				IOPFrameBufferWrPtr = (IOPFrameBufferWrPtr + 1) % NETMAN_RPC_BLOCK_SIZE;

				if((length = NetManTxPacketAfter(&payload)) > 0)
				{	//Write back the cache of the next packet, while waiting.
					unaligned = (int)((u32)payload & 15);
					unalignedCache = (int)((u32)payload & 63);
					payloadAligned = (void*)((u32)payload & ~15);
					payloadCacheAligned = (void*)((u32)payload & ~63);
					SifWriteBackDCache(payloadCacheAligned, (length + unalignedCache + 63) & ~63);
				}

				NumTx++;

				while(SifDmaStat(dmat_id) >= 0){ };
				NetManTxPacketDeQ();
			} while(length > 0);

			IsProcessingTx = 0;
		}
	}
}

void NetManRpcNetIFXmit(void)
{
	if(!IsProcessingTx)
		WakeupThread(NETMAN_Tx_threadID);
}

int NetManSetMainIF(const char *name)
{
	int result;

	if (!IsInitialized)
		return -1;

	WaitSema(NetManIOSemaID);

	strncpy(TransmitBuffer.netifName, name, NETMAN_NETIF_NAME_MAX_LEN);
	TransmitBuffer.netifName[NETMAN_NETIF_NAME_MAX_LEN-1] = '\0';
	if((result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_SET_MAIN_NETIF, 0, &TransmitBuffer, NETMAN_NETIF_NAME_MAX_LEN, &ReceiveBuffer, sizeof(s32), NULL, NULL))>=0)
		result=ReceiveBuffer.result;

	SignalSema(NetManIOSemaID);

	return result;
}

int NetManQueryMainIF(char *name)
{
	int result;
	
	if (!IsInitialized)
		return -1;

	WaitSema(NetManIOSemaID);

	if((result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_QUERY_MAIN_NETIF, 0, NULL, 0, &ReceiveBuffer, sizeof(struct NetManQueryMainNetIFResult), NULL, NULL))>=0)
	{
		if((result=ReceiveBuffer.QueryMainNetIFResult.result) == 0)
		{
			strncpy(name, ReceiveBuffer.QueryMainNetIFResult.name, NETMAN_NETIF_NAME_MAX_LEN);
			name[NETMAN_NETIF_NAME_MAX_LEN-1] = '\0';
		}
	}

	SignalSema(NetManIOSemaID);

	return result;
}

int NetManSetLinkMode(int mode)
{
	int result;
	
	if (!IsInitialized)
		return -1;

	WaitSema(NetManIOSemaID);

	TransmitBuffer.mode = mode;
	if((result=SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_SET_LINK_MODE, 0, &TransmitBuffer, sizeof(s32), &ReceiveBuffer, sizeof(s32), NULL, NULL))>=0)
		result=ReceiveBuffer.result;

	SignalSema(NetManIOSemaID);

	return result;
}
