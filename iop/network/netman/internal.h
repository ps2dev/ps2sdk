/* #define SaveGP() \
	void *_ori_gp;	\
	__asm("sw $gp, 0(%1)\n"	\
	"move $gp, %0" :: "r"(_gp), "r"(&_ori_gp))

#define RestoreGP() \
	__asm("lw $gp, 0(%0)" :: "r"(&_ori_gp)) */

void NetManToggleGlobalNetIFLinkState(unsigned char state);
int DMATransferDataToEEAligned(const void *src, void *dest, unsigned int size);
int EE_memcpy(const void *src, void *dest, unsigned int size);

