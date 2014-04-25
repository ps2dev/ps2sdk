int checkModelVersion(void);
void _GetGsDxDyOffset(int mode, int *arg1, int *arg2, int *arg3, int *arg4);

//DMA management
void GsDmaInit(void);
void GsDmaSend(const void *addr, unsigned int qwords);
void GsDmaSend_tag(const void *addr, unsigned int qwords, const GS_GIF_DMACHAIN_TAG *tag);
void GsDmaWait(void);
