/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __PS2_OSAL_H__
#define __PS2_OSAL_H__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <kernel.h>
#include <kernel_util.h>

typedef int pte_osThreadHandle;
typedef int pte_osSemaphoreHandle;
typedef int pte_osMutexHandle;
#include <sys/pte_generic_osal.h>

pte_osResult pteTlsGlobalInit(int maxEntries);
void * pteTlsThreadInit(void);

pte_osResult __pteTlsAlloc(unsigned int *pKey);
void * pteTlsGetValue(void *pTlsThreadStruct, unsigned int index);
pte_osResult __pteTlsSetValue(void *pTlsThreadStruct, unsigned int index, void * value);
void *__getTlsStructFromThread(s32 thid);
pte_osResult pteTlsFree(unsigned int index);

void pteTlsThreadDestroy(void * pTlsThreadStruct);
void pteTlsGlobalDestroy(void);

struct OsalThreadInfo {
  uint32_t threadNumber;
  void *tlsPtr;
};

extern struct OsalThreadInfo __threadInfo[];

#endif /* __PS2_OSAL_H__ */
