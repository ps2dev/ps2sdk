extern int checkModelVersion(void);

//DMA management
extern void GsDmaInit(void);
extern void GsDmaSend(const void *addr, u32 qwords);
extern void GsDmaSend_tag(const void *addr, u32 qwords, const GS_GIF_DMACHAIN_TAG *tag);
extern void GsDmaWait(void);
