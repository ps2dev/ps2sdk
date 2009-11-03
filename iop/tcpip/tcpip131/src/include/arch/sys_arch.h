#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__

typedef int						sys_prot_t;
typedef int						sys_sem_t;
typedef int	                    sys_mbox_t;
typedef int						sys_thread_t;

#define	SYS_MBOX_NULL			-1
#define	SYS_SEM_NULL			0

typedef struct st_arch_message
{
    struct st_arch_message *next;
    void *sys_msg;
} arch_message;

arch_message *alloc_msg(void);
void free_msg(arch_message *msg);

void PostInputMSG(sys_mbox_t mbid, arch_message *msg);

#if		defined(DEBUG)

struct pbuf;

void	DumpMBox(sys_mbox_t pMBox);
void	DumpSysStats(void);
void	DumpPBuf(struct pbuf* pBuf);

#endif	//defined(DEBUG)

#endif /* __SYS_ARCH_H__ */
