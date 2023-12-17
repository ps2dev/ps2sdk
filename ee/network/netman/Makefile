# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

CORE_OBJS = erl-support.o

NETMAN_OBJS = \
	__netman_MainNetProtStack.o __netman_IsInitialized.o __netman_IsNetStackInitialized.o __netman_NIFLinkState.o \
	NetManToggleGlobalNetIFLinkState.o NetManGetGlobalNetIFLinkState.o NetManInit.o NetManDeinit.o \
	NetManRegisterNetworkStack.o NetManUnregisterNetworkStack.o NetManNetIFXmit.o NetManIoctl.o \
	NetManNetProtStackAllocRxPacket.o NetManNetProtStackFreeRxPacket.o NetManNetProtStackEnQRxPacket.o \
	NetManTxPacketNext.o NetManTxPacketDeQ.o NetManTxPacketAfter.o NetManNetProtStackReallocRxPacket.o

RPC_SERVER_OBJS = \
	__NETMAN_RpcSvr_threadID.o __NETMAN_Rx_threadID.o __NETMAN_SifHandlerID.o __rpc_server_IsInitialized.o \
	__rpc_server_IsProcessingRx.o __rpc_server_FrameBufferStatus.o __rpc_server_RxIOPFrameBufferStatus.o \
	__rpc_server_cb_queue.o __rpc_server_cb_srv.o \
	__rpc_server_ClearBufferLen.o _NetManRPCAllocRxBuffers.o _NetManInitRPCServer.o _NetManDeinitRPCServer.o

RPC_CLIENT_OBJS = \
	__rpc_client_TransmitBuffer.o __rpc_client_ReceiveBuffer.o __NETMAN_rpc_cd.o __NetManIOSemaID.o \
	__NETMAN_Tx_threadID.o __netman_IOPFrameBufferWrPtr.o __netman_IOPFrameBuffer.o \
	__netman_IOPFrameBufferStatus.o __netman_FrameBufferStatus.o __netman_rpc_client_IsInitialized.o \
	__netman_rpc_client_IIsProcessingTx.o __rpc_client_deinitCleanup.o NetManInitRPCClient.o \
	NetManRPCRegisterNetworkStack.o NetManRPCUnregisterNetworkStack.o NetManDeinitRPCClient.o NetManRpcIoctl.o \
	NetManRpcNetIFXmit.o NetManSetMainIF.o NetManQueryMainIF.o NetManSetLinkMode.o

EE_OBJS = $(CORE_OBJS) $(NETMAN_OBJS) $(RPC_SERVER_OBJS) $(RPC_CLIENT_OBJS)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(NETMAN_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)netman.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(RPC_SERVER_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)rpc_server.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(RPC_CLIENT_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)rpc_client.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@
