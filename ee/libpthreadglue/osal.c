/*
 * PSP Software Development Kit - https://github.com/pspdev
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * osal.c - Pthread compatible system calls.
 *
 * Copyright (c) 2021 Francisco J Trujillo <fjtrujy@gmail.com>
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pspkerror.h>
#include <pspthreadman.h>
#include <pspsdk.h>


typedef int pte_osThreadHandle;
typedef int pte_osSemaphoreHandle;
typedef int pte_osMutexHandle;
#include <sys/pte_generic_osal.h>

#define MAX_PSP_UID 2048 // SWAG
#define DEFAULT_STACK_SIZE_BYTES 4096
#define PSP_MAX_TLS 32

pte_osResult pteTlsGlobalInit(int maxEntries);
void * pteTlsThreadInit(void);

pte_osResult __pteTlsAlloc(unsigned int *pKey);
void * pteTlsGetValue(void *pTlsThreadStruct, unsigned int index);
pte_osResult __pteTlsSetValue(void *pTlsThreadStruct, unsigned int index, void * value);
void *__getTlsStructFromThread(SceUID thid);
pte_osResult pteTlsFree(unsigned int index);

void pteTlsThreadDestroy(void * pTlsThreadStruct);
void pteTlsGlobalDestroy(void);

#if 0
#define PSP_DEBUG(x) printf(x)
#else
#define PSP_DEBUG(x)
#endif

#define POLLING_DELAY_IN_us	100

/* TLS key used to access pspThreadData struct for reach thread. */
#ifdef F___threadDataKey
unsigned int __threadDataKey;
#else
extern unsigned int __threadDataKey;
#endif

extern void *__globalTls;

/*
 * Data stored on a per-thread basis - allocated in pte_osThreadCreate
 * and freed in pte_osThreadDelete.
 */
typedef struct pspThreadData
  {
    /* Entry point and parameters to thread's main function */
    pte_osThreadEntryPoint entryPoint;
    void * argv;

    /* Semaphore used for cancellation.  Posted to by pte_osThreadCancel, 
       polled in pte_osSemaphoreCancellablePend */
    SceUID cancelSem;

  } pspThreadData;


/****************************************************************************
 *
 * Helper functions
 *
 ***************************************************************************/
#ifdef F___getThreadData
pspThreadData *__getThreadData(SceUID threadHandle)
{
  pspThreadData *pThreadData;
  void *pTls;

  pTls = __getTlsStructFromThread(threadHandle);
  pThreadData = (pspThreadData *) pteTlsGetValue(pTls, __threadDataKey);

  return pThreadData;
}
#else
pspThreadData *__getThreadData(SceUID threadHandle);
#endif

/* A new thread's stub entry point.  It retrieves the real entry point from the per thread control
 * data as well as any parameters to this function, and then calls the entry point.
 */
#ifdef F___pspStubThreadEntry
int __pspStubThreadEntry(unsigned int argc, void *argv)
{
  int result;
  pspThreadData *pThreadData;

  pThreadData = __getThreadData(sceKernelGetThreadId());
  result = (*(pThreadData->entryPoint))(pThreadData->argv);

  return result;
}
#else 
extern int __pspStubThreadEntry(unsigned int argc, void *argv);
#endif

/****************************************************************************
 *
 * Initialization
 *
 ***************************************************************************/
#ifdef F_pte_osInit
pte_osResult pte_osInit(void)
{
  pte_osResult result;
  pspThreadData *pThreadData;
  char cancelSemName[64];

  /* Allocate and initialize TLS support */
  result = pteTlsGlobalInit(PSP_MAX_TLS);

  if (result == PTE_OS_OK) {
    /* Allocate a key that we use to store control information (e.g. cancellation semaphore) per thread */
    result = __pteTlsAlloc(&__threadDataKey);

    if (result == PTE_OS_OK) {
      /* Initialize the structure used to emulate TLS for 
        * non-POSIX threads 
        */
      __globalTls = pteTlsThreadInit();

      /* Also create a "thread data" structure for a single non-POSIX thread. */

      /* Allocate some memory for our per-thread control data.  We use this for:
        * 1. Entry point and parameters for the user thread's main function.
        * 2. Semaphore used for thread cancellation.
        */
      pThreadData = (pspThreadData *) malloc(sizeof(pspThreadData));

      if (pThreadData == NULL) {
        result = PTE_OS_NO_RESOURCES;
      } else {
        /* Save a pointer to our per-thread control data as a TLS value */
        __pteTlsSetValue(__globalTls, __threadDataKey, pThreadData);

        /* Create a semaphore used to cancel threads */
        snprintf(cancelSemName, sizeof(cancelSemName), "pthread_cancelSemGlobal");

        pThreadData->cancelSem = sceKernelCreateSema(cancelSemName,
                              0,          /* attributes (default) */
                              0,          /* initial value        */
                              255,        /* maximum value        */
                              0);         /* options (default)    */
        result = PTE_OS_OK;
      }
    }
  }

  return result;
}
#endif

#ifdef F_pte_osTerminate
pte_osResult pte_osTerminate(void) {
  pteTlsGlobalDestroy();
  return PTE_OS_OK;
}
#endif

/****************************************************************************
 *
 * Threads
 *
 ***************************************************************************/
#ifdef F_pte_osThreadCreate
pte_osResult pte_osThreadCreate(pte_osThreadEntryPoint entryPoint,
                                int stackSize,
                                int initialPriority,
                                void *argv,
                                pte_osThreadHandle* ppte_osThreadHandle)
{
  char threadName[64];
  char cancelSemName[64];
  static int threadNum = 1;
  int pspAttr;
  void *pTls;
  SceUID threadId;
  pte_osResult result;
  pspThreadData *pThreadData;

  if (threadNum++ > MAX_PSP_UID) {
    threadNum = 0;
  }

  /* Make sure that the stack we're going to allocate is big enough */
  if (stackSize < DEFAULT_STACK_SIZE_BYTES) {
    stackSize = DEFAULT_STACK_SIZE_BYTES;
  }

  /* Allocate TLS structure for this thread. */
  pTls = pteTlsThreadInit();
  if (pTls == NULL) {
    PSP_DEBUG("pteTlsThreadInit: PTE_OS_NO_RESOURCES\n");
    result = PTE_OS_NO_RESOURCES;
    goto FAIL0;
  }

  /* Allocate some memory for our per-thread control data.  We use this for:
   * 1. Entry point and parameters for the user thread's main function.
   * 2. Semaphore used for thread cancellation.
   */
  pThreadData = (pspThreadData *) malloc(sizeof(pspThreadData));

  if (pThreadData == NULL) {
    pteTlsThreadDestroy(pTls);

    PSP_DEBUG("malloc(pspThreadData): PTE_OS_NO_RESOURCES\n");
    result = PTE_OS_NO_RESOURCES;
    goto FAIL0;
  }

  /* Save a pointer to our per-thread control data as a TLS value */
  __pteTlsSetValue(pTls, __threadDataKey, pThreadData);

  pThreadData->entryPoint = entryPoint;
  pThreadData->argv = argv;

  /* Create a semaphore used to cancel threads */
  snprintf(cancelSemName, sizeof(cancelSemName), "pthread_cancelSem%04d", threadNum);

  pThreadData->cancelSem = sceKernelCreateSema(cancelSemName,
                           0,          /* attributes (default) */
                           0,          /* initial value        */
                           255,        /* maximum value        */
                           0);         /* options (default)    */


  /* In order to emulate TLS functionality, we append the address of the TLS structure that we
   * allocated above to the thread's name.  To set or get TLS values for this thread, the user
   * needs to get the name of the thread from the OS and then parse the name to extract
   * a pointer to the TLS structure.
   */
  snprintf(threadName, sizeof(threadName), "pthread%04d__%x", threadNum, (unsigned int) pTls);

  pspAttr = 0;

  //  printf("%s %p %d %d %d\n",threadName, __pspStubThreadEntry, initialPriority, stackSize, pspAttr);
  threadId = sceKernelCreateThread(threadName,
                                   __pspStubThreadEntry,
                                   initialPriority,
                                   stackSize,
                                   pspAttr,
                                   NULL);

  if (threadId == (SceUID) SCE_KERNEL_ERROR_NO_MEMORY) {
    free(pThreadData);
    pteTlsThreadDestroy(pTls);

    PSP_DEBUG("sceKernelCreateThread: PTE_OS_NO_RESOURCES\n");
    result =  PTE_OS_NO_RESOURCES;
  } else if (threadId < 0) {
    free(pThreadData);
    pteTlsThreadDestroy(pTls);

    PSP_DEBUG("sceKernelCreateThread: PTE_OS_GENERAL_FAILURE\n");
    result = PTE_OS_GENERAL_FAILURE;
  } else {
    *ppte_osThreadHandle = threadId;
    result = PTE_OS_OK;
  }

FAIL0:
  return result;
}
#endif

#ifdef F_pte_osThreadStart
pte_osResult pte_osThreadStart(pte_osThreadHandle osThreadHandle)
{
  sceKernelStartThread(osThreadHandle, 0, 0);

  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osThreadDelete
pte_osResult pte_osThreadDelete(pte_osThreadHandle handle)
{
  pspThreadData *pThreadData;
  void *pTls;

  pTls = __getTlsStructFromThread(handle);
  pThreadData = __getThreadData(handle);
  sceKernelDeleteSema(pThreadData->cancelSem);  
  free(pThreadData);
  pteTlsThreadDestroy(pTls);
  sceKernelDeleteThread(handle);

  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osThreadExitAndDelete
pte_osResult pte_osThreadExitAndDelete(pte_osThreadHandle handle)
{
  pte_osThreadDelete(handle);
  sceKernelExitDeleteThread(0);

  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osThreadExit
void pte_osThreadExit()
{
  sceKernelExitThread(0);
}
#endif

/*
 * This has to be cancellable, so we can't just call sceKernelWaitThreadEnd.
 * Instead, poll on this in a loop, like we do for a cancellable semaphore.
 */
#ifdef F_pte_osThreadWaitForEnd
pte_osResult pte_osThreadWaitForEnd(pte_osThreadHandle threadHandle)
{
  pte_osResult result;
  pspThreadData *pThreadData;

  pThreadData = __getThreadData(sceKernelGetThreadId());

  while (1) {
    SceKernelThreadRunStatus info;

    /* Poll task to see if it has ended */
    memset(&info,0,sizeof(info));
    info.size = sizeof(info);
    sceKernelReferThreadRunStatus(threadHandle, &info);      

    if (info.status == PSP_THREAD_STOPPED) {
      /* Thread has ended */
      result = PTE_OS_OK;
      break;
    } else {
      SceKernelSemaInfo semInfo;

      if (pThreadData != NULL) {
        SceUID osResult;

        osResult = sceKernelReferSemaStatus(pThreadData->cancelSem, &semInfo);
        if (osResult == SCE_KERNEL_ERROR_OK) {
          if (semInfo.currentCount > 0) {
            result = PTE_OS_INTERRUPTED;
            break;
          } else {
            /* Nothing found and not timed out yet; let's yield so we're not
              * in busy loop.
              */
            sceKernelDelayThread(POLLING_DELAY_IN_us);
          }
        } else {
          result = PTE_OS_GENERAL_FAILURE;
          break;
        }
      }
    }
  }

  return result;
}
#endif

#ifdef F_pte_osThreadGetHandle
pte_osThreadHandle pte_osThreadGetHandle(void)
{
  return sceKernelGetThreadId();
}
#endif

#ifdef F_pte_osThreadGetPriority
int pte_osThreadGetPriority(pte_osThreadHandle threadHandle)
{
  SceKernelThreadInfo thinfo;

  thinfo.size = sizeof(SceKernelThreadInfo);
  sceKernelReferThreadStatus(threadHandle, &thinfo);

  return thinfo.currentPriority;
}
#endif

#ifdef F_pte_osThreadSetPriority
pte_osResult pte_osThreadSetPriority(pte_osThreadHandle threadHandle, int newPriority)
{
  sceKernelChangeThreadPriority(threadHandle, newPriority);
  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osThreadCancel
pte_osResult pte_osThreadCancel(pte_osThreadHandle threadHandle)
{
  SceUID osResult;
  pte_osResult result;
  pspThreadData *pThreadData;

  pThreadData = __getThreadData(threadHandle);
  osResult = sceKernelSignalSema(pThreadData->cancelSem, 1);

  if (osResult == SCE_KERNEL_ERROR_OK) {
    result = PTE_OS_OK;
  } else {
    result = PTE_OS_GENERAL_FAILURE;
  }

  return result;
}
#endif

#ifdef F_pte_osThreadCheckCancel
pte_osResult pte_osThreadCheckCancel(pte_osThreadHandle threadHandle)
{
  pspThreadData *pThreadData;
  SceKernelSemaInfo semInfo;
  SceUID osResult;
  pte_osResult result;

  pThreadData = __getThreadData(threadHandle);
  if (pThreadData != NULL) {
    osResult = sceKernelReferSemaStatus(pThreadData->cancelSem, &semInfo);

    if (osResult == SCE_KERNEL_ERROR_OK) {
      if (semInfo.currentCount > 0) {
        result = PTE_OS_INTERRUPTED;
      } else {
        result = PTE_OS_OK;
      }
    } else {
      /* sceKernelReferSemaStatus returned an error */
      result = PTE_OS_GENERAL_FAILURE;
    }
  } else {
    /* For some reason, we couldn't get thread data */
    result = PTE_OS_GENERAL_FAILURE;
  }

  return result;
}
#endif

#ifdef F_pte_osThreadSleep
void pte_osThreadSleep(unsigned int msecs)
{
  sceKernelDelayThread(msecs*1000);
}
#endif

#ifdef  F_pte_osThreadGetMinPriority
int pte_osThreadGetMinPriority()
{
  return 17;
}
#endif

#ifdef F_pte_osThreadGetMaxPriority
int pte_osThreadGetMaxPriority()
{
  return 30;
}
#endif

#ifdef F_pte_osThreadGetDefaultPriority
int pte_osThreadGetDefaultPriority()
{
  return 18;
}
#endif

#ifdef F_pthread_num_processors_np
int pthread_num_processors_np(void)
{
  return 1;
}
#endif

/****************************************************************************
 *
 * Mutexes
 *
 ****************************************************************************/
#ifdef F_pte_osMutexCreate
pte_osResult pte_osMutexCreate(pte_osMutexHandle *pHandle)
{
  static int mutexCtr = 0;
  char mutexName[32];
  pte_osMutexHandle handle;

  if (mutexCtr++ > MAX_PSP_UID) {
    mutexCtr = 0;
  }

  snprintf(mutexName,sizeof(mutexName),"mutex%d",mutexCtr);
  handle = sceKernelCreateSema(mutexName,
                               0,          /* attributes (default) */
                               1,          /* initial value        */
                               1,          /* maximum value        */
                               0);         /* options (default)    */

  *pHandle = handle;
  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osMutexDelete
pte_osResult pte_osMutexDelete(pte_osMutexHandle handle)
{
  sceKernelDeleteSema(handle);

  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osMutexLock
pte_osResult pte_osMutexLock(pte_osMutexHandle handle)
{
  sceKernelWaitSema(handle, 1, NULL);

  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osMutexTimedLock
pte_osResult pte_osMutexTimedLock(pte_osMutexHandle handle, unsigned int timeoutMsecs)
{
  pte_osResult result;
  SceUInt timeoutUsecs = timeoutMsecs*1000;

  int status = sceKernelWaitSema(handle, 1, &timeoutUsecs);
  if (status < 0) {
    // Assume that any error from sceKernelWaitSema was due to a timeout
    result = PTE_OS_TIMEOUT;
  } else {
    result = PTE_OS_OK;
  }

  return result;
}
#endif

#ifdef F_pte_osMutexUnlock
pte_osResult pte_osMutexUnlock(pte_osMutexHandle handle)
{
  sceKernelSignalSema(handle, 1);
  return PTE_OS_OK;
}
#endif

/****************************************************************************
 *
 * Semaphores
 *
 ***************************************************************************/
#ifdef F_pte_osSemaphoreCreate
pte_osResult pte_osSemaphoreCreate(int initialValue, pte_osSemaphoreHandle *pHandle)
{
  pte_osSemaphoreHandle handle;
  static int semCtr = 0;
  char semName[32];

  if (semCtr++ > MAX_PSP_UID) {
    semCtr = 0;
  }

  snprintf(semName,sizeof(semName),"pthread_sem%d",semCtr);
  handle = sceKernelCreateSema(semName,
                               0,              /* attributes (default) */
                               initialValue,   /* initial value        */
                               SEM_VALUE_MAX,  /* maximum value        */
                               0);             /* options (default)    */

  *pHandle = handle;
  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osSemaphoreDelete
pte_osResult pte_osSemaphoreDelete(pte_osSemaphoreHandle handle)
{
  sceKernelDeleteSema(handle);
  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osSemaphorePost
pte_osResult pte_osSemaphorePost(pte_osSemaphoreHandle handle, int count)
{
  sceKernelSignalSema(handle, count);
  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osSemaphorePend
pte_osResult pte_osSemaphorePend(pte_osSemaphoreHandle handle, unsigned int *pTimeoutMsecs)
{
  unsigned int timeoutUsecs;
  unsigned int *pTimeoutUsecs;
  SceUInt result;
  pte_osResult osResult;

  if (pTimeoutMsecs == NULL) {
    pTimeoutUsecs = NULL;
  } else {
    timeoutUsecs = *pTimeoutMsecs * 1000;
    pTimeoutUsecs = &timeoutUsecs;
  }

  result = sceKernelWaitSema(handle, 1, pTimeoutUsecs);
  if (result == SCE_KERNEL_ERROR_OK) {
    osResult = PTE_OS_OK;
  } else if (result == SCE_KERNEL_ERROR_WAIT_TIMEOUT) {
    osResult = PTE_OS_TIMEOUT;
  } else {
    osResult = PTE_OS_GENERAL_FAILURE;
  }

  return osResult;
}
#endif


/*
 * Pend on a semaphore- and allow the pend to be cancelled.
 *
 * PSP OS provides no functionality to asynchronously interrupt a blocked call.  We simulte
 * this by polling on the main semaphore and the cancellation semaphore and sleeping in a loop.
 */
#ifdef F_pte_osSemaphoreCancellablePend
pte_osResult pte_osSemaphoreCancellablePend(pte_osSemaphoreHandle semHandle, unsigned int *pTimeout)
{
  pspThreadData *pThreadData;

  pThreadData = __getThreadData(sceKernelGetThreadId());

  clock_t start_time;
  pte_osResult result =  PTE_OS_OK;
  unsigned int timeout;
  unsigned char timeoutEnabled;

  start_time = clock();

  // clock() is in microseconds, timeout as passed in was in milliseconds
  if (pTimeout == NULL) {
    timeout = 0;
    timeoutEnabled = 0;
  } else {
    timeout = *pTimeout * 1000;
    timeoutEnabled = 1;
  }

  while (1) {
    SceUInt semTimeout;
    int status;

    /* Poll semaphore */
    semTimeout = 0;
    status = sceKernelWaitSema(semHandle, 1, &semTimeout);

    if (status == SCE_KERNEL_ERROR_OK) {
      /* User semaphore posted to */
      result = PTE_OS_OK;
      break;
    } else if ((timeoutEnabled) && ((clock() - start_time) > timeout)) {
      /* Timeout expired */
      result = PTE_OS_TIMEOUT;
      break;
    } else {
      SceKernelSemaInfo semInfo;

      if (pThreadData != NULL) {
        SceUID osResult;

        osResult = sceKernelReferSemaStatus(pThreadData->cancelSem, &semInfo);
        if (osResult == SCE_KERNEL_ERROR_OK) {
          if (semInfo.currentCount > 0) {
              result = PTE_OS_INTERRUPTED;
              break;
          } else {
            /* Nothing found and not timed out yet; let's yield so we're not
              * in busy loop.
              */
            sceKernelDelayThread(POLLING_DELAY_IN_us);
          }
        } else {
          result = PTE_OS_GENERAL_FAILURE;
          break;
        }
      }
    }
  }

  return result;
}
#endif


/****************************************************************************
 *
 * Atomic Operations
 *
 ***************************************************************************/
#ifdef F_pte_osAtomicExchange
int pte_osAtomicExchange(int *ptarg, int val)
{
  int intc = pspSdkDisableInterrupts();
  int origVal;

  origVal = *ptarg;
  *ptarg = val;

  pspSdkEnableInterrupts(intc);
  return origVal;
}
#endif

#ifdef F_pte_osAtomicCompareExchange
int pte_osAtomicCompareExchange(int *pdest, int exchange, int comp)
{
  int intc = pspSdkDisableInterrupts();
  int origVal;

  origVal = *pdest;
  if (*pdest == comp){
    *pdest = exchange;
  }

  pspSdkEnableInterrupts(intc);
  return origVal;
}
#endif

#ifdef F_pte_osAtomicExchangeAdd
int pte_osAtomicExchangeAdd(int volatile* pAddend, int value)
{
  int origVal;
  int intc = pspSdkDisableInterrupts();

  origVal = *pAddend;
  *pAddend += value;

  pspSdkEnableInterrupts(intc);
  return origVal;
}
#endif

#ifdef F_pte_osAtomicDecrement
int pte_osAtomicDecrement(int *pdest)
{
  int val;
  int intc = pspSdkDisableInterrupts();

  (*pdest)--;
  val = *pdest;

  pspSdkEnableInterrupts(intc);
  return val;
}
#endif

#ifdef F_pte_osAtomicIncrement
int pte_osAtomicIncrement(int *pdest)
{
  int val;
  int intc = pspSdkDisableInterrupts();

  (*pdest)++;
  val = *pdest;

  pspSdkEnableInterrupts(intc);
  return val;
}
#endif
