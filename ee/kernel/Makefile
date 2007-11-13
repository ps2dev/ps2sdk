# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$


LIBSYSCALL_LIB = lib/libsyscall.a
EE_LIB = lib/libkernel.a

EE_OBJS_DIR = obj/
EE_LIB_DIR = lib/
EE_SRC_DIR = src/
EE_INC_DIR = include/

SIFCMD_OBJS = sif_cmd_send.o _sif_cmd_int_handler.o sif_cmd_main.o \
	sif_cmd_addhandler.o sif_sreg_get.o
SIFCMD_OBJS := $(SIFCMD_OBJS:%=$(EE_OBJS_DIR)%)

SIFRPC_OBJS = SifBindRpc.o SifCallRpc.o SifRpcGetOtherData.o \
	SifRegisterRpc.o SifSetRpcQueue.o SifGetNextRequest.o \
	SifExecRequest.o SifRpcLoop.o SifRpcMain.o _rpc_get_packet.o \
	_rpc_get_fpacket.o SifCheckStatRpc.o
SIFRPC_OBJS := $(SIFRPC_OBJS:%=$(EE_OBJS_DIR)%)

FILEIO_OBJS = __fio_internals.o fio_init.o _fio_intr.o fio_sync.o fio_setblockmode.o \
	fio_exit.o fio_open.o fio_close.o fio_read.o \
	fio_write.o fio_lseek.o fio_mkdir.o _fio_read_intr.o fio_getc.o fio_putc.o \
	fio_ioctl.o fio_dopen.o fio_dclose.o fio_dread.o fio_getstat.o fio_chstat.o \
	fio_remove.o fio_format.o fio_rmdir.o fio_gets.o
FILEIO_OBJS := $(FILEIO_OBJS:%=$(EE_OBJS_DIR)%)

LOADFILE_OBJS = SifLoadFileInit.o SifLoadFileExit.o _SifLoadModule.o SifLoadModule.o \
	SifLoadStartModule.o SifLoadModuleEncrypted.o SifStopModule.o SifUnloadModule.o \
	SifSearchModuleByName.o SifSearchModuleByAddress.o _SifLoadElfPart.o SifLoadElfPart.o \
	SifLoadElf.o SifLoadElfEncrypted.o SifIopSetVal.o SifIopGetVal.o \
	_SifLoadModuleBuffer.o SifLoadModuleBuffer.o SifLoadStartModuleBuffer.o \
	SifExecModuleBuffer.o SifExecModuleFile.o 
LOADFILE_OBJS := $(LOADFILE_OBJS:%=$(EE_OBJS_DIR)%)

IOPHEAP_OBJS = SifInitIopHeap.o SifExitIopHeap.o SifAllocIopHeap.o \
	SifFreeIopHeap.o SifLoadIopHeap.o
IOPHEAP_OBJS := $(IOPHEAP_OBJS:%=$(EE_OBJS_DIR)%)

IOPCONTROL_OBJS = SifIopReboot.o SifIopReset.o SifResetIop.o SifIopIsAlive.o SifIopSync.o __iop_control_internals.o
IOPCONTROL_OBJS := $(IOPCONTROL_OBJS:%=$(EE_OBJS_DIR)%)

GLUE_OBJS = DIntr.o EIntr.o EnableIntc.o DisableIntc.o EnableDmac.o DisableDmac.o \
	iEnableIntc.o iDisableIntc.o iEnableDmac.o iDisableDmac.o SyncDCache.o \
	iSyncDCache.o InvalidDCache.o iInvalidDCache.o
GLUE_OBJS := $(GLUE_OBJS:%=$(EE_OBJS_DIR)%)

SIO_OBJS = sio_init.o sio_putc.o sio_getc.o sio_write.o sio_read.o sio_puts.o \
	sio_gets.o sio_getc_block.o sio_flush.o sio_putsn.o
SIO_OBJS := $(SIO_OBJS:%=$(EE_OBJS_DIR)%)

CONFIG_OBJS = _config_internals.o GetRomName.o IsT10K.o IsEarlyJap.o configGetLanguage.o \
	configSetLanguage.o configGetTvScreenType.o configSetTvScreenType.o \
	configGetDateFormat.o configSetDateFormat.o configGetTimeFormat.o \
	configSetTimeFormat.o configGetTimezone.o configSetTimezone.o \
	configIsSpdifEnabled.o configSetSpdifEnabled.o configGetTime.o \
	configIsDaylightSavingEnabled.o configSetDaylightSavingEnabled.o
CONFIG_OBJS := $(CONFIG_OBJS:%=$(EE_OBJS_DIR)%)

TIMER_OBJS = cpu_ticks.o
TIMER_OBJS := $(TIMER_OBJS:%=$(EE_OBJS_DIR)%)

GETSYSCALL_OBJS = GetSyscall.o
GETSYSCALL_OBJS := $(GETSYSCALL_OBJS:%=$(EE_OBJS_DIR)%)


### SYSCALL OBJECTS

KERNEL_OBJS = ResetEE.o SetGsCrt.o Exit.o LoadExecPS2.o ExecPS2.o \
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
	RFU059.o RFU060.o SetupThread.o RFU061.o SetupHeap.o EndOfHeap.o CreateSema.o DeleteSema.o \
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
	SifWriteBackDCache.o _SyncDCache.o _InvalidDCache.o __errno.o errno.o \
	strncpy.o strlen.o memcpy.o memset.o __syscall.o

KERNEL_OBJS := $(KERNEL_OBJS:%=$(EE_OBJS_DIR)%)

LIBSYSCALL_OBJS = $(EE_OBJS_DIR)dummy.o

EE_OBJS = $(KERNEL_OBJS) $(SIFCMD_OBJS) $(SIFRPC_OBJS) $(FILEIO_OBJS) \
	$(LOADFILE_OBJS) $(IOPHEAP_OBJS) $(IOPCONTROL_OBJS) $(CONFIG_OBJS) \
	$(GLUE_OBJS) $(SIO_OBJS) $(TIMER_OBJS) $(GETSYSCALL_OBJS) $(EE_OBJS_DIR)erl-support.o

all: $(EE_OBJS_DIR) $(EE_LIB_DIR) $(LIBSYSCALL_LIB) $(EE_LIB)

$(LIBSYSCALL_LIB): $(LIBSYSCALL_OBJS)
	$(EE_AR) cru $(LIBSYSCALL_LIB) $(LIBSYSCALL_OBJS)

obj/dummy.o: src/kernel.S
	$(EE_C_COMPILE) $< -c -o $@

$(KERNEL_OBJS): src/kernel.S
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SIFCMD_OBJS): src/sifcmd.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SIFRPC_OBJS): src/sifrpc.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(FILEIO_OBJS): src/fileio.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(LOADFILE_OBJS): src/loadfile.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(CONFIG_OBJS): src/osd_config.c
	$(EE_C_COMPILE) -I../rpc/cdvd/include -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(TIMER_OBJS): src/timer.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(GETSYSCALL_OBJS): src/getsyscall.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(IOPHEAP_OBJS): src/iopheap.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(IOPCONTROL_OBJS): src/iopcontrol.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(GLUE_OBJS): src/glue.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SIO_OBJS): src/sio.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_LIB_DIR)


include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

release: release-dirs release-include
	cp -f $(LIBSYSCALL_LIB) $(PS2SDK)/ee/lib
	cp -f $(EE_LIB) $(PS2SDK)/ee/lib
	cp -f $(EE_LIB:%.a=%.erl) $(PS2SDK)/ee/lib
	chmod 644 $(PS2SDK)/ee/$(LIBSYSCALL_LIB) $(PS2SDK)/ee/$(EE_LIB) $(PS2SDK)/ee/$(EE_LIB:%.a=%.erl)
