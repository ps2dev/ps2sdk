
#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__

typedef int						sys_prot_t;
typedef int						sys_sem_t;
typedef struct sys_mbox*	sys_mbox_t;
typedef int						sys_thread_t;

#define	SYS_MBOX_NULL			NULL
#define	SYS_SEM_NULL			0
#define	LWIP_PLATFORM_DIAG	printf

int	IsMessageBoxFull(sys_mbox_t pMBox);
int	IsMessageBoxEmpty(sys_mbox_t pMBox);
void	PostInputMSG(sys_mbox_t pMBox,void* pvMSG);

#if		defined(DEBUG)

struct pbuf;

void	DumpMBox(sys_mbox_t pMBox);
void	DumpSysStats(void);
void	DumpPBuf(struct pbuf* pBuf);

#endif	//defined(DEBUG)

#endif /* __SYS_ARCH_H__ */
