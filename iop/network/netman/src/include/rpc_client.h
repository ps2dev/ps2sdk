int NetManInitRPCClient(void);
void NetManDeinitRPCClient(void);
void NetManRpcToggleGlobalNetIFLinkState(unsigned int state);
struct NetManPacketBuffer *NetManRpcNetProtStackAllocRxPacket(unsigned int length);
void NetManRpcNetProtStackFreeRxPacket(struct NetManPacketBuffer *packet);
int NetManRpcProtStackEnQRxPacket(struct NetManPacketBuffer *packet);
int NetmanRpcFlushInputQueue(void);
