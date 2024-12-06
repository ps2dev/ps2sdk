extern int NetManInitRPCClient(void);
extern void NetManDeinitRPCClient(void);
extern void NetManRpcToggleGlobalNetIFLinkState(int state);
extern void *NetManRpcNetProtStackAllocRxPacket(unsigned int length, void **payload);
extern void NetManRpcNetProtStackFreeRxPacket(void *packet);
extern void NetManRpcProtStackEnQRxPacket(void *packet);
