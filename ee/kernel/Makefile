# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.


LIBSYSCALL_LIB = lib/libsyscall.a

EE_LIB = $(LIBSYSCALL_LIB)
EE_INC_DIR = include/

OBJDIR = obj/
OBJEXIST = $(OBJDIR).created
LIBDIR = lib/
LIBEXIST = $(LIBDIR).created


LIBSYSCALL_OBJS = ResetEE.o SetGsCrt.o Exit.o LoadExecPS2.o ExecPS2.o \
	RFU009.o AddSbusIntcHandler.o RemoveSbusIntcHandler.o Interrupt2Iop.o \
	SetVTLBRefillHandler.o SetVCommonHandler.o SetVInterruptHandler.o \
	AddIntcHandler.o AddIntcHandler2.o RemoveIntcHandler.o AddDmacHandler.o \
	RemoveDmacHandler.o _EnableIntc.o _DisableIntc.o _EnableDmac.o _DisableDmac.o \
	SetAlarm.o ReleaseAlarm.o _iEnableIntc.o _iDisableIntc.o _iEnableDmac.o \
	_iDisableDmac.o iSetAlarm.o iReleaseAlarm.o CreateThread.o DeleteThread.o \
	StartThread.o ExitThread.o ExitDeleteThread.o TerminateThread.o \
	iTerminateThread.o DisableDispatchThread.o EnableDispatchThread.o \
	ChangeThreadPriority.o iChangeThreadPriority.o RotateThreadReadyQueue.o \
	iRotateThreadReadyQueue.o ReleaseWaitThread.o iReleaseWaitThread.o \
	GetThreadId.o ReferThreadStatus.o iReferThreadStatus.o SleepThread.o \
	WakeupThread.o iWakeupThread.o CancelWakeupThread.o iCancelWakeupThread.o \
	SuspendThread.o iSuspendThread.o ResumeThread.o iResumeThread.o \
	RFU059.o RFU060.o RFU061.o EndOfHeap.o CreateSema.o DeleteSema.o \
	iSignalSema.o SignalSema.o WaitSema.o PollSema.o iPollSema.o \
	ReferSemaStatus.o iReferSemaStatus.o iDeleteSema.o SetOsdConfigParam.o \
	GetOsdConfigParam.o GetGsHParam.o GetGsVParam.o SetGsHParam.o \
	SetGsVParam.o CreateEventFlag.o DeleteEventFlag.o SetEventFlag.o \
	iSetEventFlag.o EnableIntcHandler.o iEnableIntcHandler.o \
	DisableIntcHandler.o iDisableIntcHandler.o EnableDmacHandler.o \
	iEnableDmacHandler.o DisableDmacHandler.o iDisableDmacHandler.o KSeg0.o \
	EnableCache.o DisableCache.o GetCop0.o FlushCache.o CpuConfig.o \
	iGetCop0.o iFlushCache.o RFU105.o iCpuConfig.o SifStopDma.o \
	SetCPUTimerHandler.o SetCPUTimer.o SetOsdConfigParam2.o \
	GetOsdConfigParam2.o GsGetIMR.o iGsGetIMR.o GsPutIMR.o iGsPutIMR.o \
	SetPgifHandler.o SetVSyncFlag.o SetSyscall.o SifDmaStat.o iSifDmaStat.o \
	SifSetDma.o iSifSetDma.o SifSetDChain.o iSifSetDChain.o SifSetReg.o \
	SifGetReg.o ExecOSD.o Deci2Call.o PSMode.o MachineType.o GetMemorySize.o \
	SifWriteBackDCache.o _SyncDCache.o _InvalidDCache.o __errno.o errno.o

LIBSYSCALL_OBJS := $(LIBSYSCALL_OBJS:%=$(OBJDIR)%)

EE_OBJS = $(LIBSYSCALL_OBJS)

all: $(OBJEXIST) $(LIBEXIST) $(LIBSYSCALL_LIB)

# $(LIBSYSCALL_LIB): $(LIBSYSCALL_OBJS)
#	$(EE_AR) cru $(LIBSYSCALL_LIB) $(LIBSYSCALL_OBJS)

$(OBJEXIST):
	@echo; echo "Creating object directory"
	test -d $(dir $@) || mkdir $(dir $@)
	touch $@

$(LIBEXIST):
	@echo; echo "Creating library directory"
	test -d $(dir $@) || mkdir $(dir $@)
	touch $@

$(LIBSYSCALL_OBJS): src/kernel.S
	$(EE_C_COMPILE) -DF_$(*:$(OBJDIR)%=%) $< -c -o $@


clean:
	$(RM) -rf $(OBJDIR)

install:
	$(CP) $(LIBSYSCALL_LIB) $(PS2SDK)/ee/lib
	$(CP) include/*.h $(PS2SDK)/ee/include

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release
