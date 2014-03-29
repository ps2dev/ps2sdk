#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__

#include <kernel.h>
#include <malloc.h>

#define	SYS_MBOX_NULL	NULL
#define	SYS_SEM_NULL	-1

typedef struct st_arch_message
{
	struct st_arch_message *next;
	void *sys_msg;
} arch_message;

struct MboxData{
	int SemaID, MessageCountSema;
	arch_message *FirstMessage;
	arch_message *LastMessage;
};

struct ProtData{
	int level;
	int LockingThreadID;
	int SemaID;
};

typedef struct ProtData	sys_prot_t;
typedef int	sys_sem_t;
typedef struct MboxData *sys_mbox_t;
typedef int	sys_thread_t;

#define SYS_ARCH_PROTECT(x)	\
	if(x.SemaID<0){	\
		ee_sema_t sema;	\
		sema.attr=sema.option=0;	\
		sema.init_count=sema.max_count=1;	\
		x.SemaID=CreateSema(&sema);	\
	}	\
	if(!(x.level>0 && x.LockingThreadID==GetThreadId())) WaitSema(x.SemaID);	\
	x.level++;

#define SYS_ARCH_UNPROTECT(x)	\
	x.level--;	\
	if(x.level<1) SignalSema(x.SemaID);

#define SYS_ARCH_DECL_PROTECT(x)	\
	static struct ProtData x={0,-1,-1};	\


#endif /* __SYS_ARCH_H__ */

