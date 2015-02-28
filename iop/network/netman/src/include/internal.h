void NetManUpdateStackNIFLinkState(void);
void NetManToggleGlobalNetIFLinkState(unsigned char state);
int DMATransferDataToEEAligned(const void *src, void *dest, unsigned int size);
int EE_memcpy(const void *src, void *dest, unsigned int size);
void *malloc(int size);
void free(void *buffer);
