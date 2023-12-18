# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_LIB = libpthreadglue.a

OSAL_OBJS = __threadInfo.o __threadDataKey.o __getThreadData.o __ps2StubThreadEntry.o \
	pte_osInit.o pte_osTerminate.o pte_osThreadCreate.o pte_osThreadStart.o pte_osThreadDelete.o \
	pte_osThreadExitAndDelete.o pte_osThreadExit.o pte_osThreadWaitForEnd.o pte_osThreadGetHandle.o pte_osThreadGetPriority.o pthread_num_processors_np.o \
	pte_osThreadSetPriority.o pte_osThreadCancel.o pte_osThreadCheckCancel.o pte_osThreadSleep.o pte_osThreadGetMinPriority.o pte_osThreadGetMaxPriority.o \
	pte_osThreadGetDefaultPriority.o pte_osMutexCreate.o pte_osMutexDelete.o pte_osMutexLock.o pte_osMutexTimedLock.o pte_osMutexUnlock.o \
	pte_osSemaphoreCreate.o pte_osSemaphoreDelete.o pte_osSemaphorePost.o pte_osSemaphorePend.o pte_osSemaphoreCancellablePend.o \
	pte_osAtomicExchange.o pte_osAtomicCompareExchange.o pte_osAtomicExchangeAdd.o pte_osAtomicDecrement.o pte_osAtomicIncrement.o

TLS_HELPER_OBJS = __keysUsed.o __maxTlsValues.o __globalTlsLock.o __globalTls.o pteTlsGlobalInit.o pteTlsThreadInit.o \
	__pteTlsAlloc.o pteTlsGetValue.o __pteTlsSetValue.o __getTlsStructFromThread.o getTlsStructFromThread.o pteTlsFree.o \
	pteTlsThreadDestroy.o pteTlsGlobalDestroy.o pte_osTlsSetValue.o pte_osTlsGetValue.o pte_osTlsAlloc.o pte_osTlsFree.o

EE_OBJS = $(OSAL_OBJS) $(TLS_HELPER_OBJS)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(OSAL_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)osal.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(TLS_HELPER_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)tls-helper.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@