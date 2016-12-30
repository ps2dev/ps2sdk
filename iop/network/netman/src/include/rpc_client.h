int NetManInitRPCClient(void);
void NetManDeinitRPCClient(void);
void NetManRpcToggleGlobalNetIFLinkState(int state);
void *NetManRpcNetProtStackAllocRxPacket(unsigned int length, void **payload);
void NetManRpcNetProtStackFreeRxPacket(void *packet);
void NetManRpcProtStackEnQRxPacket(void *packet);
