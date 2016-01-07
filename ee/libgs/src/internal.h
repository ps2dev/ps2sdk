int checkModelVersion(void);

//DMA management
void GsDmaInit(void);
void GsDmaSend(const void *addr, unsigned int qwords);
void GsDmaSend_tag(const void *addr, unsigned int qwords, const GS_GIF_DMACHAIN_TAG *tag);
void GsDmaWait(void);
