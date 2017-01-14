#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__

typedef int	sys_prot_t;
typedef int	sys_sem_t;
typedef int	sys_mbox_t;
typedef int	sys_thread_t;

#define	SYS_MBOX_NULL	-1
#define	SYS_SEM_NULL	-1

#define SYS_MAX_MESSAGES	(MEMP_NUM_TCPIP_MSG_API+MEMP_NUM_TCPIP_MSG_INPKT)

typedef struct st_arch_message
{
    struct st_arch_message *next;
    void *sys_msg;
} arch_message;

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t n, size_t size);

#endif /* __SYS_ARCH_H__ */
