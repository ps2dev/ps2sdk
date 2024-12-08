extern int NetManInitRPCClient(void);
extern void NetManDeinitRPCClient(void);
extern int NetManRPCRegisterNetworkStack(void);
extern int NetManRPCUnregisterNetworkStack(void);
extern int NetManRpcIoctl(unsigned int command, void *arg, unsigned int arg_len, void *output, unsigned int length);
extern void NetManRpcNetIFXmit(void);
