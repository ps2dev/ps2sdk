/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <ps2_osal.h>
#include <time.h>

#define MAX_PS2_UID 2048 // SWAG
#define DEFAULT_STACK_SIZE_BYTES 4096
#define PS2_MAX_TLS 32
#define MAX_THREADS 256

#if 0
#define PS2_DEBUG(x) printf(x)
#else
#define PS2_DEBUG(x)
#endif

#define POLLING_DELAY_IN_us	100

#if F___threadInfo
struct OsalThreadInfo __threadInfo[MAX_THREADS];
#endif

/* TLS key used to access ps2ThreadData struct for reach thread. */
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
typedef struct ps2ThreadData
  {
    /* Entry point and parameters to thread's main function */
    pte_osThreadEntryPoint entryPoint;
    void * argv;

    /* Semaphore used for cancellation.  Posted to by pte_osThreadCancel, 
       polled in pte_osSemaphoreCancellablePend */
    s32 cancelSem;

  } ps2ThreadData;


/****************************************************************************
 *
 * Helper functions
 *
 ***************************************************************************/
#ifdef F___getThreadData
ps2ThreadData *__getThreadData(s32 threadHandle)
{
  ps2ThreadData *pThreadData;
  void *pTls;

  pTls = __getTlsStructFromThread(threadHandle);
  pThreadData = (ps2ThreadData *) pteTlsGetValue(pTls, __threadDataKey);

  return pThreadData;
}
#else
ps2ThreadData *__getThreadData(s32 threadHandle);
#endif

static inline void DelayThread(uint32_t usecs) {
  struct timespec tv = {0};
  tv.tv_sec          = usecs / 1000000;
  tv.tv_nsec         = (usecs % 1000000) * 1000;
  nanosleep(&tv, NULL);
}

static inline int SemWaitTimeout(s32 semHandle, uint32_t timeout)
{
    int ret;
    u64 timeoutUsec;
    u64 *timeoutPtr;

    if (timeout == 0) {
        if (PollSema(semHandle) < 0) {
            return -1;
        }
        return 0;
    }

    timeoutPtr = NULL;

    if (timeout > 0 && timeout != UINT32_MAX) {
        timeoutUsec = timeout * 1000;
        timeoutPtr = &timeoutUsec;
    }

    ret = WaitSemaEx(semHandle, 1, timeoutPtr);

    if (ret < 0)
        return -2;
    return 0; //Wait condition satisfied.
}

/* A new thread's stub entry point.  It retrieves the real entry point from the per thread control
 * data as well as any parameters to this function, and then calls the entry point.
 */
#ifdef F___ps2StubThreadEntry
int __ps2StubThreadEntry(void *argv)
{
  int result;
  ps2ThreadData *pThreadData;

  pThreadData = __getThreadData(GetThreadId());
  result = (*(pThreadData->entryPoint))(pThreadData->argv);

  return result;
}
#else 
extern int __ps2StubThreadEntry(void *argv);
#endif

/****************************************************************************
 *
 * Initialization
 *
 ***************************************************************************/
#ifdef F_pte_osInit
pte_osResult pte_osInit(void)
{
  ee_sema_t sema;
  pte_osResult result;
  ps2ThreadData *pThreadData;

  /* Allocate and initialize TLS support */
  result = pteTlsGlobalInit(PS2_MAX_TLS);

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
      pThreadData = (ps2ThreadData *) malloc(sizeof(ps2ThreadData));

      if (pThreadData == NULL) {
        result = PTE_OS_NO_RESOURCES;
      } else {
        /* Save a pointer to our per-thread control data as a TLS value */
        __pteTlsSetValue(__globalTls, __threadDataKey, pThreadData);

        sema.init_count = 0;
        sema.max_count  = 255;
        sema.option     = 0;
        pThreadData->cancelSem = CreateSema(&sema);

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
  ee_thread_t eethread;
  ee_sema_t sema;
  struct OsalThreadInfo *threadInfo;
  void *stack;
  static int threadNum = 1;
  void *pTls;
  s32 threadId;
  pte_osResult result;
  ps2ThreadData *pThreadData;

  if (threadNum++ > MAX_PS2_UID) {
    threadNum = 0;
  }

  /* Make sure that the stack we're going to allocate is big enough */
  if (stackSize < DEFAULT_STACK_SIZE_BYTES) {
    stackSize = DEFAULT_STACK_SIZE_BYTES;
  }

  /* Allocate TLS structure for this thread. */
  pTls = pteTlsThreadInit();
  if (pTls == NULL) {
    PS2_DEBUG("pteTlsThreadInit: PTE_OS_NO_RESOURCES\n");
    result = PTE_OS_NO_RESOURCES;
    goto FAIL0;
  }

  /* Allocate some memory for our per-thread control data.  We use this for:
   * 1. Entry point and parameters for the user thread's main function.
   * 2. Semaphore used for thread cancellation.
   */
  pThreadData = (ps2ThreadData *) malloc(sizeof(ps2ThreadData));

  if (pThreadData == NULL) {
    pteTlsThreadDestroy(pTls);

    PS2_DEBUG("malloc(ps2ThreadData): PTE_OS_NO_RESOURCES\n");
    result = PTE_OS_NO_RESOURCES;
    goto FAIL0;
  }

  /* Save a pointer to our per-thread control data as a TLS value */
  __pteTlsSetValue(pTls, __threadDataKey, pThreadData);

  pThreadData->entryPoint = entryPoint;
  pThreadData->argv = argv;

  sema.init_count = 0;
  sema.max_count  = 255;
  sema.option     = 0;
  pThreadData->cancelSem = CreateSema(&sema);

  /* Create EE Thread */
  stack = malloc(stackSize);

	eethread.attr = 0;
	eethread.option = 0;
	eethread.func = &__ps2StubThreadEntry;
	eethread.stack = stack;
	eethread.stack_size = stackSize;
	eethread.gp_reg = &_gp;
	eethread.initial_priority = initialPriority;
	threadId = CreateThread(&eethread);
  
  /* In order to emulate TLS functionality, we append the address of the TLS structure that we
   * allocated above to an additional struct. To set or get TLS values for this thread, the user
   * needs to get the threadId from the OS and then extract
   * a pointer to the TLS structure.
   */
  threadInfo = &__threadInfo[threadId];
  threadInfo->threadNumber = threadNum;
  threadInfo->tlsPtr = pTls;

  if (!stack) {
    free(pThreadData);
    pteTlsThreadDestroy(pTls);

    PS2_DEBUG("CreateThread: PTE_OS_NO_RESOURCES\n");
    result =  PTE_OS_NO_RESOURCES;
  } else if (threadId < 0) {
    free(pThreadData);
    pteTlsThreadDestroy(pTls);

    PS2_DEBUG("CreateThread: PTE_OS_GENERAL_FAILURE\n");
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
  StartThread(osThreadHandle, 0);

  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osThreadDelete
pte_osResult pte_osThreadDelete(pte_osThreadHandle handle)
{
  ps2ThreadData *pThreadData;
  void *pTls;
  ee_thread_status_t info;
  struct OsalThreadInfo *threadInfo;
  int res;

  res = ReferThreadStatus(handle, &info);

  pTls = __getTlsStructFromThread(handle);
  pThreadData = __getThreadData(handle);
  DeleteSema(pThreadData->cancelSem);  
  free(pThreadData);
  pteTlsThreadDestroy(pTls);
  TerminateThread(handle);
  DeleteThread(handle);

  if (res > 0 && info.stack) {
    free(info.stack);
  }

  threadInfo = &__threadInfo[handle];
  threadInfo->threadNumber = 0;
  threadInfo->tlsPtr = NULL;

  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osThreadExitAndDelete
pte_osResult pte_osThreadExitAndDelete(pte_osThreadHandle handle)
{
  pte_osThreadDelete(handle);
  ExitDeleteThread();

  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osThreadExit
void pte_osThreadExit()
{
  ExitThread();
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
  ps2ThreadData *pThreadData;

  pThreadData = __getThreadData(GetThreadId());

  while (1) {
    ee_thread_status_t info;

    /* Poll task to see if it has ended */
    ReferThreadStatus(threadHandle, &info);

    if (info.status == THS_DORMANT) {
      /* Thread has ended */
      result = PTE_OS_OK;
      break;
    } else {
      ee_sema_t semInfo;

      if (pThreadData != NULL) {
        s32 osResult;

        osResult = ReferSemaStatus(pThreadData->cancelSem, &semInfo);
        if (osResult == pThreadData->cancelSem) {
          if (semInfo.count > 0) {
            result = PTE_OS_INTERRUPTED;
            break;
          } else {
            /* Nothing found and not timed out yet; let's yield so we're not
              * in busy loop.
              */
            DelayThread(POLLING_DELAY_IN_us);
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
  return GetThreadId();
}
#endif

#ifdef F_pte_osThreadGetPriority
int pte_osThreadGetPriority(pte_osThreadHandle threadHandle)
{
  ee_thread_status_t thinfo;

  ReferThreadStatus(threadHandle, &thinfo);

  return thinfo.current_priority;
}
#endif

#ifdef F_pte_osThreadSetPriority
pte_osResult pte_osThreadSetPriority(pte_osThreadHandle threadHandle, int newPriority)
{
  ChangeThreadPriority(threadHandle, newPriority);
  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osThreadCancel
pte_osResult pte_osThreadCancel(pte_osThreadHandle threadHandle)
{
  s32 osResult;
  pte_osResult result;
  ps2ThreadData *pThreadData;

  pThreadData = __getThreadData(threadHandle);
  osResult = SignalSema(pThreadData->cancelSem);

  if (osResult == pThreadData->cancelSem) {
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
  ps2ThreadData *pThreadData;
  ee_sema_t semInfo;
  s32 osResult;
  pte_osResult result;

  pThreadData = __getThreadData(threadHandle);
  if (pThreadData != NULL) {
    osResult = ReferSemaStatus(pThreadData->cancelSem, &semInfo);

    if (osResult == pThreadData->cancelSem) {
      if (semInfo.count > 0) {
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
  DelayThread(msecs * 1000);
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
  ee_sema_t sema;
  pte_osMutexHandle handle;

  if (mutexCtr++ > MAX_PS2_UID) {
    mutexCtr = 0;
  }

  sema.init_count = 1;
  sema.max_count  = 1;
  sema.option     = 0;
  handle = CreateSema(&sema);

  *pHandle = handle;
  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osMutexDelete
pte_osResult pte_osMutexDelete(pte_osMutexHandle handle)
{
  DeleteSema(handle);

  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osMutexLock
pte_osResult pte_osMutexLock(pte_osMutexHandle handle)
{
  SemWaitTimeout(handle, UINT32_MAX);

  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osMutexTimedLock
pte_osResult pte_osMutexTimedLock(pte_osMutexHandle handle, unsigned int timeoutMsecs)
{
  pte_osResult result;

  int status = SemWaitTimeout(handle, timeoutMsecs);
  if (status < 0) {
    // Assume that any error from SemWaitTimeout was due to a timeout
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
  SignalSema(handle);
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
  ee_sema_t sema;
  static int semCtr = 0;

  if (semCtr++ > MAX_PS2_UID) {
    semCtr = 0;
  }

  sema.init_count = initialValue;
  sema.max_count  = 32767;
  sema.option     = 0;
  handle = CreateSema(&sema);

  *pHandle = handle;
  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osSemaphoreDelete
pte_osResult pte_osSemaphoreDelete(pte_osSemaphoreHandle handle)
{
  DeleteSema(handle);
  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osSemaphorePost
pte_osResult pte_osSemaphorePost(pte_osSemaphoreHandle handle, int count)
{
  int i;
  for (i = 0; i < count; i++)
    SignalSema(handle);

  return PTE_OS_OK;
}
#endif

#ifdef F_pte_osSemaphorePend
pte_osResult pte_osSemaphorePend(pte_osSemaphoreHandle handle, unsigned int *pTimeoutMsecs)
{
  uint32_t timeout;
  uint32_t result;
  pte_osResult osResult;
  
  if (pTimeoutMsecs == NULL) {
    timeout = UINT32_MAX;
  } else {
    timeout = *pTimeoutMsecs;
  }

  result = SemWaitTimeout(handle, timeout);
  if (result == 0) {
    osResult = PTE_OS_OK;
  } else if (result == -2) {
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
 * PS2 OS provides no functionality to asynchronously interrupt a blocked call.  We simulte
 * this by polling on the main semaphore and the cancellation semaphore and sleeping in a loop.
 */
#ifdef F_pte_osSemaphoreCancellablePend
pte_osResult pte_osSemaphoreCancellablePend(pte_osSemaphoreHandle semHandle, unsigned int *pTimeout)
{
  ps2ThreadData *pThreadData;

  pThreadData = __getThreadData(GetThreadId());

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
    int status;

    /* Poll semaphore */
    status = SemWaitTimeout(semHandle, UINT32_MAX);

    if (status == 0) {
      /* User semaphore posted to */
      result = PTE_OS_OK;
      break;
    } else if ((timeoutEnabled) && ((clock() - start_time) > timeout)) {
      /* Timeout expired */
      result = PTE_OS_TIMEOUT;
      break;
    } else {
      ee_sema_t semInfo;

      if (pThreadData != NULL) {
        s32 osResult;

        osResult = ReferSemaStatus(pThreadData->cancelSem, &semInfo);
        if (osResult == pThreadData->cancelSem) {
          if (semInfo.count > 0) {
              result = PTE_OS_INTERRUPTED;
              break;
          } else {
            /* Nothing found and not timed out yet; let's yield so we're not
              * in busy loop.
              */
            DelayThread(POLLING_DELAY_IN_us);
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
  int intc = DIntr();
  int origVal;

  origVal = *ptarg;
  *ptarg = val;

  if(intc) { EIntr(); }
  return origVal;
}
#endif

#ifdef F_pte_osAtomicCompareExchange
int pte_osAtomicCompareExchange(int *pdest, int exchange, int comp)
{
  int intc = DIntr();
  int origVal;

  origVal = *pdest;
  if (*pdest == comp){
    *pdest = exchange;
  }

  if(intc) { EIntr(); }
  return origVal;
}
#endif

#ifdef F_pte_osAtomicExchangeAdd
int pte_osAtomicExchangeAdd(int volatile* pAddend, int value)
{
  int origVal;
  int intc = DIntr();

  origVal = *pAddend;
  *pAddend += value;

  if(intc) { EIntr(); }
  return origVal;
}
#endif

#ifdef F_pte_osAtomicDecrement
int pte_osAtomicDecrement(int *pdest)
{
  int val;
  int intc = DIntr();

  (*pdest)--;
  val = *pdest;

  if(intc) { EIntr(); }
  return val;
}
#endif

#ifdef F_pte_osAtomicIncrement
int pte_osAtomicIncrement(int *pdest)
{
  int val;
  int intc = DIntr();

  (*pdest)++;
  val = *pdest;

  if(intc) { EIntr(); }
  return val;
}
#endif