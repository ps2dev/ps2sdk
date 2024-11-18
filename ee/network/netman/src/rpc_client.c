#include <errno.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include <malloc.h>
#include <netman.h>
#include <netman_rpc.h>

#include "rpc_client.h"

typedef union {
	s32 mode;
	struct NetManIoctl IoctlArgs;
	char netifName[NETMAN_NETIF_NAME_MAX_LEN];
	struct NetManRegNetworkStack NetStack;
	u8 buffer[128];
} __attribute__((packed,aligned(64))) trasnmit_buffer_t;

typedef union {
	s32 result;
	struct NetManRegNetworkStackResult NetStackResult;
	struct NetManIoctlResult IoctlResult;
	struct NetManQueryMainNetIFResult QueryMainNetIFResult;
	u8 buffer[128];
} __attribute__((packed,aligned(64))) receive_buffer_t;

extern void *_gp;

#ifdef F___rpc_client_TransmitBuffer
trasnmit_buffer_t __rpc_client_TransmitBuffer;
#else
extern trasnmit_buffer_t __rpc_client_TransmitBuffer;
#endif

#ifdef F___rpc_client_ReceiveBuffer
receive_buffer_t __rpc_client_ReceiveBuffer;
#else
extern receive_buffer_t __rpc_client_ReceiveBuffer;
#endif

#ifdef F___NETMAN_rpc_cd
SifRpcClientData_t __NETMAN_rpc_cd;
#else
extern SifRpcClientData_t __NETMAN_rpc_cd;
#endif

#ifdef F___NetManIOSemaID
int __NetManIOSemaID = -1;
#else
extern int __NetManIOSemaID;
#endif

#ifdef F___NETMAN_Tx_threadID
int __NETMAN_Tx_threadID = -1;
#else
extern int __NETMAN_Tx_threadID;
#endif

#ifdef F___netman_IOPFrameBufferWrPtr
unsigned short int __netman_IOPFrameBufferWrPtr;
#else
extern unsigned short int __netman_IOPFrameBufferWrPtr;
#endif

#ifdef F___netman_IOPFrameBuffer
u8 *__netman_IOPFrameBuffer = NULL;	/* On the IOP side. */
#else
extern u8 *__netman_IOPFrameBuffer;
#endif

#ifdef F___netman_IOPFrameBufferStatus
struct NetManBD *__netman_IOPFrameBufferStatus = NULL;
#else
extern struct NetManBD *__netman_IOPFrameBufferStatus;
#endif

#ifdef F___netman_FrameBufferStatus
struct NetManBD *__netman_FrameBufferStatus = NULL;
#else
extern struct NetManBD *__netman_FrameBufferStatus;
#endif

#ifdef F___netman_rpc_client_IsInitialized
unsigned char __netman_rpc_client_IsInitialized=0;
#else
extern unsigned char __netman_rpc_client_IsInitialized;
#endif

#ifdef F___netman_rpc_client_IIsProcessingTx
unsigned char __netman_rpc_client_IIsProcessingTx;
#else
extern unsigned char __netman_rpc_client_IIsProcessingTx;
#endif

#ifdef F___rpc_client_deinitCleanup
void __rpc_client_deinitCleanup(void)
{
	if(__NetManIOSemaID >= 0)
	{
		DeleteSema(__NetManIOSemaID);
		__NetManIOSemaID = -1;
	}
	if(__NETMAN_Tx_threadID >= 0)
	{
		TerminateThread(__NETMAN_Tx_threadID);
		DeleteThread(__NETMAN_Tx_threadID);
		__NETMAN_Tx_threadID = -1;
	}
}
#else
extern void __rpc_client_deinitCleanup(void);
#endif

#ifdef F_NetManInitRPCClient
static unsigned char NETMAN_Tx_ThreadStack[0x1000] ALIGNED(16);

static void NETMAN_TxThread(void *arg)
{
	static SifCmdHeader_t cmd ALIGNED(64);
	SifDmaTransfer_t dmat[2];
	struct NetManPktCmd *npcmd;
	int dmat_id, unaligned, unalignedCache;
	void *payload, *payloadAligned, *payloadCacheAligned;
	volatile struct NetManBD *bd, *bdNext;

	(void)arg;

	while(1)
	{
		int length, NumTx;
		SleepThread();

		NumTx = 0;
		while((length = NetManTxPacketNext(&payload)) > 0)
		{
			__netman_rpc_client_IIsProcessingTx = 1;

			//Write back D-cache, before performing a DMA transfer.
			unaligned = (int)((u32)payload & 15);
			unalignedCache = (int)((u32)payload & 63);
			payloadAligned = (void*)((u32)payload & ~15);
			payloadCacheAligned = (void*)((u32)payload & ~63);
			SifWriteBackDCache(payloadCacheAligned, (length + unalignedCache + 63) & ~63);

			do {
				//Wait for a spot to be freed up.
				bd = UNCACHED_SEG(&__netman_FrameBufferStatus[__netman_IOPFrameBufferWrPtr]);
				while(bd->length != 0){}

				//Transfer to IOP RAM
				//Determine mode of transfer.
				bdNext = UNCACHED_SEG(&__netman_FrameBufferStatus[(__netman_IOPFrameBufferWrPtr + 1) % NETMAN_RPC_BLOCK_SIZE]);
				if((NumTx + 1) >= NETMAN_FRAME_GROUP_SIZE || bdNext->length == 0)
				{
					//Prepare SIFCMD packet
					//Record the frame length.
					npcmd = (struct NetManPktCmd*)&cmd.opt;
					npcmd->length = length;
					npcmd->offset = unaligned;
					npcmd->id = __netman_IOPFrameBufferWrPtr;

					while((dmat_id = SifSendCmd(NETMAN_SIFCMD_ID, &cmd, sizeof(SifCmdHeader_t),
									(void*)payloadAligned,
									(void*)&__netman_IOPFrameBuffer[__netman_IOPFrameBufferWrPtr * NETMAN_MAX_FRAME_SIZE],
									(length + unaligned + 15) & ~15)) == 0){ };
				} else {
					//Record the frame length.
					bd->length = length;
					bd->offset = unaligned;

					//Normal DMA transfer
					dmat[0].src = (void*)payloadAligned;
					dmat[0].dest = (void*)&__netman_IOPFrameBuffer[__netman_IOPFrameBufferWrPtr * NETMAN_MAX_FRAME_SIZE];
					dmat[0].size = (length + unaligned + 15) & ~15;
					dmat[0].attr = 0;
					dmat[1].src = (void*)&__netman_FrameBufferStatus[__netman_IOPFrameBufferWrPtr];
					dmat[1].dest = (void*)&__netman_IOPFrameBufferStatus[__netman_IOPFrameBufferWrPtr];
					dmat[1].size = sizeof(struct NetManBD);
					dmat[1].attr = 0;

					while((dmat_id = SifSetDma(dmat, 2)) == 0){ };
				}

				//Increase write pointer by one position.
				__netman_IOPFrameBufferWrPtr = (__netman_IOPFrameBufferWrPtr + 1) % NETMAN_RPC_BLOCK_SIZE;

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

			__netman_rpc_client_IIsProcessingTx = 0;
		}
	}
}

int NetManInitRPCClient(void){
	static const char NetManID[]="NetMan";
	int result;
	ee_sema_t SemaData;
	ee_thread_t thread;

	if(!__netman_rpc_client_IsInitialized)
	{
		SemaData.max_count=1;
		SemaData.init_count=1;
		SemaData.option=(u32)NetManID;
		SemaData.attr=0;
		if((__NetManIOSemaID=CreateSema(&SemaData)) < 0)
		{
			__rpc_client_deinitCleanup();
			return __NetManIOSemaID;
		}

		thread.func=&NETMAN_TxThread;
		thread.stack=NETMAN_Tx_ThreadStack;
		thread.stack_size=sizeof(NETMAN_Tx_ThreadStack);
		thread.gp_reg=&_gp;
		thread.initial_priority=0x56;	/* Should be given a higher priority than the protocol stack, so that it can dump frames in the EE and return. */
		thread.attr=thread.option=0;

		if((__NETMAN_Tx_threadID=CreateThread(&thread)) >= 0)
		{
			__netman_rpc_client_IIsProcessingTx = 0;
			StartThread(__NETMAN_Tx_threadID, NULL);
		} else {
			__rpc_client_deinitCleanup();
			return __NETMAN_Tx_threadID;
		}

		while((SifBindRpc(&__NETMAN_rpc_cd, NETMAN_RPC_NUMBER, 0)<0)||(__NETMAN_rpc_cd.server==NULL))
			nopdelay();

		if((result=SifCallRpc(&__NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_INIT, 0, NULL, 0, &__rpc_client_ReceiveBuffer, sizeof(s32), NULL, NULL))>=0)
		{
			if((result=__rpc_client_ReceiveBuffer.result) == 0)
				__netman_rpc_client_IsInitialized=1;
			else
				__rpc_client_deinitCleanup();
		}else{
			__rpc_client_deinitCleanup();
		}
	}
	else result=0;

	return result;
}
#endif

#ifdef F_NetManRPCRegisterNetworkStack
int NetManRPCRegisterNetworkStack(void)
{
	int result;

	WaitSema(__NetManIOSemaID);

	if(__netman_FrameBufferStatus == NULL) __netman_FrameBufferStatus = memalign(64, NETMAN_RPC_BLOCK_SIZE * sizeof(struct NetManBD));

	if(__netman_FrameBufferStatus != NULL)
	{
		memset(UNCACHED_SEG(__netman_FrameBufferStatus), 0, NETMAN_RPC_BLOCK_SIZE * sizeof(struct NetManBD));
		__rpc_client_TransmitBuffer.NetStack.FrameBufferStatus = __netman_FrameBufferStatus;
		__netman_IOPFrameBufferWrPtr = 0;

		if((result=SifCallRpc(&__NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_REG_NETWORK_STACK, 0, &__rpc_client_TransmitBuffer, sizeof(struct NetManRegNetworkStack), &__rpc_client_ReceiveBuffer, sizeof(struct NetManRegNetworkStackResult), NULL, NULL))>=0)
		{
			if((result=__rpc_client_ReceiveBuffer.NetStackResult.result) == 0)
			{
				__netman_IOPFrameBuffer = __rpc_client_ReceiveBuffer.NetStackResult.FrameBuffer;
				__netman_IOPFrameBufferStatus = __rpc_client_ReceiveBuffer.NetStackResult.FrameBufferStatus;
			}
		}
	}
	else
	{
		result = -ENOMEM;
	}

	SignalSema(__NetManIOSemaID);

	return result;
}
#endif

#ifdef F_NetManRPCUnregisterNetworkStack
int NetManRPCUnregisterNetworkStack(void)
{
	int result;

	WaitSema(__NetManIOSemaID);

	result=SifCallRpc(&__NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_UNREG_NETWORK_STACK, 0, NULL, 0, NULL, 0, NULL, NULL);
	__netman_IOPFrameBuffer = NULL;
	__netman_IOPFrameBufferWrPtr = 0;

	free(__netman_FrameBufferStatus);
	__netman_FrameBufferStatus = NULL;

	SignalSema(__NetManIOSemaID);

	return result;
}
#endif

#ifdef F_NetManDeinitRPCClient
void NetManDeinitRPCClient(void)
{
	if(__netman_rpc_client_IsInitialized)
	{
		WaitSema(__NetManIOSemaID);

		SifCallRpc(&__NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_DEINIT, 0, NULL, 0, NULL, 0, NULL, NULL);
		__rpc_client_deinitCleanup();

		__netman_rpc_client_IsInitialized=0;
	}
}
#endif

#ifdef F_NetManRpcIoctl
int NetManRpcIoctl(unsigned int command, void *args, unsigned int args_len, void *output, unsigned int length)
{
	int result;
	struct NetManIoctl *IoctlArgs=&__rpc_client_TransmitBuffer.IoctlArgs;

	WaitSema(__NetManIOSemaID);

	IoctlArgs->command=command;
	memcpy(IoctlArgs->args, args, args_len);
	IoctlArgs->args_len=args_len;
	IoctlArgs->output=output;
	IoctlArgs->length=length;

	if((result=SifCallRpc(&__NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_IOCTL, 0, &__rpc_client_TransmitBuffer, sizeof(struct NetManIoctl), &__rpc_client_ReceiveBuffer, sizeof(struct NetManIoctlResult), NULL, NULL))>=0)
	{
		result=__rpc_client_ReceiveBuffer.IoctlResult.result;
		memcpy(output, __rpc_client_ReceiveBuffer.IoctlResult.output, length);
	}

	SignalSema(__NetManIOSemaID);

	return result;
}
#endif

#ifdef F_NetManRpcNetIFXmit
void NetManRpcNetIFXmit(void)
{
	if(!__netman_rpc_client_IIsProcessingTx)
		WakeupThread(__NETMAN_Tx_threadID);
}
#endif

#ifdef F_NetManSetMainIF
int NetManSetMainIF(const char *name)
{
	int result;

	if (!__netman_rpc_client_IsInitialized)
		return -1;

	WaitSema(__NetManIOSemaID);

	strncpy(__rpc_client_TransmitBuffer.netifName, name, NETMAN_NETIF_NAME_MAX_LEN);
	__rpc_client_TransmitBuffer.netifName[NETMAN_NETIF_NAME_MAX_LEN-1] = '\0';
	if((result=SifCallRpc(&__NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_SET_MAIN_NETIF, 0, &__rpc_client_TransmitBuffer, NETMAN_NETIF_NAME_MAX_LEN, &__rpc_client_ReceiveBuffer, sizeof(s32), NULL, NULL))>=0)
		result=__rpc_client_ReceiveBuffer.result;

	SignalSema(__NetManIOSemaID);

	return result;
}
#endif

#ifdef F_NetManQueryMainIF
int NetManQueryMainIF(char *name)
{
	int result;
	
	if (!__netman_rpc_client_IsInitialized)
		return -1;

	WaitSema(__NetManIOSemaID);

	if((result=SifCallRpc(&__NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_QUERY_MAIN_NETIF, 0, NULL, 0, &__rpc_client_ReceiveBuffer, sizeof(struct NetManQueryMainNetIFResult), NULL, NULL))>=0)
	{
		if((result=__rpc_client_ReceiveBuffer.QueryMainNetIFResult.result) == 0)
		{
			strncpy(name, __rpc_client_ReceiveBuffer.QueryMainNetIFResult.name, NETMAN_NETIF_NAME_MAX_LEN);
			name[NETMAN_NETIF_NAME_MAX_LEN-1] = '\0';
		}
	}

	SignalSema(__NetManIOSemaID);

	return result;
}
#endif

#ifdef F_NetManSetLinkMode
int NetManSetLinkMode(int mode)
{
	int result;
	
	if (!__netman_rpc_client_IsInitialized)
		return -1;

	WaitSema(__NetManIOSemaID);

	__rpc_client_TransmitBuffer.mode = mode;
	if((result=SifCallRpc(&__NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_SET_LINK_MODE, 0, &__rpc_client_TransmitBuffer, sizeof(s32), &__rpc_client_ReceiveBuffer, sizeof(s32), NULL, NULL))>=0)
		result=__rpc_client_ReceiveBuffer.result;

	SignalSema(__NetManIOSemaID);

	return result;
}
#endif
