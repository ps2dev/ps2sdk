int NetManInitRPCClient(void);
void NetManDeinitRPCClient(void);
int NetManRPCRegisterNetworkStack(void);
int NetManRPCUnregisterNetworkStack(void);
int NetManRpcIoctl(unsigned int command, void *arg, unsigned int arg_len, void *output, unsigned int length);
int NetManRpcNetIFSendPacket(const void *packet, unsigned int length);
