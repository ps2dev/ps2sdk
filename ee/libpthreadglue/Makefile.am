
libdir = @PSPSDK_LIBDIR@

CC = @PSP_CC@
CCAS = $(CC)
AR = @PSP_AR@
RANLIB = @PSP_RANLIB@

CPPFLAGS = -I$(top_srcdir)/src/base \
	-I$(top_srcdir)/src/debug \
	-I$(top_srcdir)/src/kernel \
	-I$(top_srcdir)/src/sdk \
	-I$(top_srcdir)/src/user

CFLAGS = @PSPSDK_CFLAGS@
CCASFLAGS = $(CFLAGS)

OSAL_OBJS = __threadDataKey.o __getThreadData.o __pspStubThreadEntry.o \
	pte_osInit.o pte_osTerminate.o pte_osThreadCreate.o pte_osThreadStart.o pte_osThreadDelete.o \
	pte_osThreadExitAndDelete.o pte_osThreadExit.o pte_osThreadWaitForEnd.o pte_osThreadGetHandle.o pte_osThreadGetPriority.o pthread_num_processors_np.o \
	pte_osThreadSetPriority.o pte_osThreadCancel.o pte_osThreadCheckCancel.o pte_osThreadSleep.o pte_osThreadGetMinPriority.o pte_osThreadGetMaxPriority.o \
	pte_osThreadGetDefaultPriority.o pte_osMutexCreate.o pte_osMutexDelete.o pte_osMutexLock.o pte_osMutexTimedLock.o pte_osMutexUnlock.o \
	pte_osSemaphoreCreate.o pte_osSemaphoreDelete.o pte_osSemaphorePost.o pte_osSemaphorePend.o pte_osSemaphoreCancellablePend.o \
	pte_osAtomicExchange.o pte_osAtomicCompareExchange.o pte_osAtomicExchangeAdd.o pte_osAtomicDecrement.o pte_osAtomicIncrement.o

TLS_HELPER_OBJS = __keysUsed.o __maxTlsValues.o __globalTlsLock.o __globalTls.o pteTlsGlobalInit.o pteTlsThreadInit.o \
	__pteTlsAlloc.o pteTlsGetValue.o __pteTlsSetValue.o __getTlsStructFromThread.o getTlsStructFromThread.o pteTlsFree.o \
	pteTlsThreadDestroy.o pteTlsGlobalDestroy.o pte_osTlsSetValue.o pte_osTlsGetValue.o pte_osTlsAlloc.o pte_osTlsFree.o

lib_LIBRARIES = libpthreadglue.a

libpthreadglue_a_SOURCES = \
	osal.c \
	tls-helper.c

libpthreadglue_a_LIBADD  = $(OSAL_OBJS) $(TLS_HELPER_OBJS)

$(OSAL_OBJS): osal.c
	$(AM_V_CC)$(COMPILE) -DF_$* $< -c -o $@

$(TLS_HELPER_OBJS): tls-helper.c
	$(AM_V_CC)$(COMPILE) -DF_$* $< -c -o $@