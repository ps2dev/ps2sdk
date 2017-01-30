#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__

#include <kernel.h>
#include <malloc.h>

#define	SYS_MBOX_NULL	NULL
#define	SYS_SEM_NULL	-1

#define SYS_MAX_MESSAGES	(MEMP_NUM_TCPIP_MSG_API+MEMP_NUM_TCPIP_MSG_INPKT)

typedef struct st_arch_message
{
	struct st_arch_message *next;
	void *sys_msg;
} arch_message;

struct MboxData{
	int MessageCountSema;
	arch_message *FirstMessage;
	arch_message *LastMessage;
};

typedef int	sys_prot_t;
typedef int	sys_sem_t;
typedef struct MboxData *sys_mbox_t;
typedef int	sys_thread_t;

#endif /* __SYS_ARCH_H__ */

