# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

#Define to 1, to build a special version of libkernel that does not contain any runtime patches (useful for loaders/resident programs).
KERNEL_NO_PATCHES ?= 0

ifeq ($(KERNEL_NO_PATCHES),1)
	EE_CFLAGS += -DKERNEL_NO_PATCHES=1
	EXEC_SYSCALLS = Exit.o LoadExecPS2.o ExecPS2.o
	EXECOSD_SYSCALL = ExecOSD.o
	ALARM_SYSCALLS = _SetAlarm.o _ReleaseAlarm.o
	ALARM_INTR_SYSCALLS = _iSetAlarm.o _iReleaseAlarm.o
	ROTATE_THREAD_READY_QUEUE_SYSCALL = iRotateThreadReadyQueue.o
	IWAKEUP_THREAD_SYSCALL = iWakeupThread.o
	ISUSPEND_THREAD_SYSCALL = iSuspendThread.o
	TLB_SYSCALLS =
else
	EXEC_SYSCALLS = _Exit.o _LoadExecPS2.o _ExecPS2.o
	EXECOSD_SYSCALL = _ExecOSD.o
	ALARM_SYSCALLS = _SetAlarm.o SetAlarm.o _ReleaseAlarm.o ReleaseAlarm.o
	ALARM_INTR_SYSCALLS = _iSetAlarm.o iSetAlarm.o _iReleaseAlarm.o iReleaseAlarm.o
	ROTATE_THREAD_READY_QUEUE_SYSCALL = _iRotateThreadReadyQueue.o
	IWAKEUP_THREAD_SYSCALL = _iWakeupThread.o
	ISUSPEND_THREAD_SYSCALL = _iSuspendThread.o
	TLB_SYSCALLS = PutTLBEntry.o iPutTLBEntry.o _SetTLBEntry.o iSetTLBEntry.o GetTLBEntry.o iGetTLBEntry.o ProbeTLBEntry.o iProbeTLBEntry.o ExpandScratchPad.o
endif

### SIFCMD objects objects

SIFCMD_OBJS = sif_cmd_send.o _sif_cmd_int_handler.o sif_cmd_main.o \
	sif_cmd_client.o sif_cmd_remove_cmdhandler.o sif_sreg_get.o

### SIFRPC objects

SIFRPC_OBJS = SifBindRpc.o SifCallRpc.o SifRpcGetOtherData.o \
	SifRegisterRpc.o SifRemoveRpc.o SifSetRpcQueue.o SifRemoveRpcQueue.o SifGetNextRequest.o \
	SifExecRequest.o SifRpcLoop.o SifRpcMain.o _rpc_get_packet.o \
	_rpc_get_fpacket.o SifCheckStatRpc.o

### FILEIO client objects

FILEIO_OBJS = __fio_internals.o fio_init.o _fio_intr.o fio_sync.o fio_setblockmode.o \
	fio_exit.o fio_open.o fio_close.o fio_read.o \
	fio_write.o fio_lseek.o fio_mkdir.o _fio_read_intr.o fio_getc.o fio_putc.o \
	fio_ioctl.o fio_dopen.o fio_dclose.o fio_dread.o fio_getstat.o fio_chstat.o \
	fio_remove.o fio_format.o fio_rmdir.o fio_gets.o

### LOADFILE client objects

LOADFILE_OBJS = SifLoadFileInit.o SifLoadFileExit.o _SifLoadModule.o SifLoadModule.o \
	SifLoadStartModule.o SifLoadModuleEncrypted.o SifStopModule.o SifUnloadModule.o \
	SifSearchModuleByName.o SifSearchModuleByAddress.o _SifLoadElfPart.o SifLoadElfPart.o \
	SifLoadElf.o SifLoadElfEncrypted.o SifIopSetVal.o SifIopGetVal.o \
	_SifLoadModuleBuffer.o SifLoadModuleBuffer.o SifLoadStartModuleBuffer.o \
	SifExecModuleBuffer.o SifExecModuleFile.o

### IOPHEAP client objects

IOPHEAP_OBJS = SifInitIopHeap.o SifExitIopHeap.o SifAllocIopHeap.o \
	SifFreeIopHeap.o SifLoadIopHeap.o

### IOP-management objects

IOPCONTROL_OBJS = SifIopReboot.o SifIopReset.o SifIopIsAlive.o SifIopSync.o __iop_control_internals.o

### Glue objects

GLUE_OBJS = DIntr.o EIntr.o EnableIntc.o DisableIntc.o EnableDmac.o DisableDmac.o
ifeq ($(KERNEL_NO_PATCHES),1)
	GLUE_OBJS += SetAlarm.o ReleaseAlarm.o
endif
GLUE_OBJS += iEnableIntc.o iDisableIntc.o iEnableDmac.o iDisableDmac.o
ifeq ($(KERNEL_NO_PATCHES),1)
	GLUE_OBJS += iSetAlarm.o iReleaseAlarm.o
endif
GLUE_OBJS += SyncDCache.o iSyncDCache.o InvalidDCache.o iInvalidDCache.o

### SIO objects

SIO_OBJS = sio_init.o sio_putc.o sio_getc.o sio_write.o sio_read.o sio_puts.o \
	sio_gets.o sio_getc_block.o sio_flush.o sio_putsn.o

### Config objects

CONFIG_OBJS = _config_internals.o GetRomName.o IsT10K.o IsEarlyJap.o configGetLanguage.o \
	configSetLanguage.o configGetTvScreenType.o configSetTvScreenType.o \
	configGetDateFormat.o configSetDateFormat.o configGetTimeFormat.o \
	configSetTimeFormat.o configGetTimezone.o configSetTimezone.o \
	configIsSpdifEnabled.o configSetSpdifEnabled.o configGetTime.o \
	configIsDaylightSavingEnabled.o configSetDaylightSavingEnabled.o

### Patch objects

ifneq ($(KERNEL_NO_PATCHES),1)
	LIBOSD_OBJS = libosd.o libosd_full.o libosd_common.o osdsrc_bin.o

	TLBFUNC_OBJS = tlbfunc.o tlbsrc_bin.o

	ALARM_OBJS = alarm.o srcfile_bin.o eenull_bin.o

	THREAD_OBJS = _thread_internals.o iWakeupThread.o iRotateThreadReadyQueue.o iSuspendThread.o

	SETUP_OBJS = kCopy.o kCopyBytes.o

	EXIT_OBJS += _exit_internals.o SetArg.o Exit.o ExecPS2.o LoadExecPS2.o ExecOSD.o
endif

### Timer objects

TIMER_OBJS = cpu_ticks.o

### Getter objects

GETKERNEL_OBJS = GetSyscallHandler.o GetSyscall.o GetExceptionHandler.o GetInterruptHandler.o

### System initialization objects

INITSYS_OBJS = _InitSys.o TerminateLibrary.o

### SYSCALL OBJECTS

KERNEL_OBJS = ResetEE.o SetGsCrt.o $(EXEC_SYSCALLS) \
	RFU009.o AddSbusIntcHandler.o RemoveSbusIntcHandler.o Interrupt2Iop.o \
	SetVTLBRefillHandler.o SetVCommonHandler.o SetVInterruptHandler.o \
	AddIntcHandler.o AddIntcHandler2.o RemoveIntcHandler.o AddDmacHandler.o AddDmacHandler2.o \
	RemoveDmacHandler.o _EnableIntc.o _DisableIntc.o _EnableDmac.o _DisableDmac.o \
	$(ALARM_SYSCALLS) _iEnableIntc.o _iDisableIntc.o _iEnableDmac.o \
	_iDisableDmac.o $(ALARM_INTR_SYSCALLS) CreateThread.o DeleteThread.o \
	StartThread.o ExitThread.o ExitDeleteThread.o TerminateThread.o \
	iTerminateThread.o DisableDispatchThread.o EnableDispatchThread.o \
	ChangeThreadPriority.o iChangeThreadPriority.o RotateThreadReadyQueue.o \
	$(ROTATE_THREAD_READY_QUEUE_SYSCALL) ReleaseWaitThread.o iReleaseWaitThread.o \
	GetThreadId.o _iGetThreadId.o ReferThreadStatus.o iReferThreadStatus.o SleepThread.o \
	WakeupThread.o $(IWAKEUP_THREAD_SYSCALL) CancelWakeupThread.o iCancelWakeupThread.o \
	SuspendThread.o $(ISUSPEND_THREAD_SYSCALL) ResumeThread.o iResumeThread.o \
	RFU059.o RFU060.o SetupThread.o RFU061.o SetupHeap.o EndOfHeap.o CreateSema.o DeleteSema.o \
	iSignalSema.o SignalSema.o WaitSema.o PollSema.o iPollSema.o \
	ReferSemaStatus.o iReferSemaStatus.o iDeleteSema.o SetOsdConfigParam.o \
	GetOsdConfigParam.o GetGsHParam.o GetGsVParam.o SetGsHParam.o \
	SetGsVParam.o CreateEventFlag.o DeleteEventFlag.o SetEventFlag.o \
	iSetEventFlag.o $(TLB_SYSCALLS) \
	EnableIntcHandler.o iEnableIntcHandler.o DisableIntcHandler.o iDisableIntcHandler.o \
	EnableDmacHandler.o iEnableDmacHandler.o DisableDmacHandler.o iDisableDmacHandler.o \
	KSeg0.o EnableCache.o DisableCache.o GetCop0.o FlushCache.o CpuConfig.o \
	iGetCop0.o iFlushCache.o RFU105.o iCpuConfig.o SifStopDma.o \
	SetCPUTimerHandler.o SetCPUTimer.o SetOsdConfigParam2.o \
	GetOsdConfigParam2.o GsGetIMR.o iGsGetIMR.o GsPutIMR.o iGsPutIMR.o \
	SetPgifHandler.o SetVSyncFlag.o SetSyscall.o SifDmaStat.o iSifDmaStat.o \
	SifSetDma.o iSifSetDma.o SifSetDChain.o iSifSetDChain.o SifSetReg.o \
	SifGetReg.o $(EXECOSD_SYSCALL) Deci2Call.o PSMode.o MachineType.o GetMemorySize.o _GetGsDxDyOffset.o _InitTLB.o \
	SifWriteBackDCache.o _SyncDCache.o _InvalidDCache.o __errno.o errno.o \
	strncpy.o strlen.o memcpy.o memset.o __syscall.o

EE_OBJS = $(KERNEL_OBJS) $(SIFCMD_OBJS) $(SIFRPC_OBJS) $(FILEIO_OBJS) \
	$(LOADFILE_OBJS) $(IOPHEAP_OBJS) $(IOPCONTROL_OBJS) $(CONFIG_OBJS) \
	$(GLUE_OBJS) $(SIO_OBJS) $(TIMER_OBJS) $(GETKERNEL_OBJS) \
	$(INITSYS_OBJS) erl-support.o

ifneq ($(KERNEL_NO_PATCHES),1)
	EE_OBJS += $(THREAD_OBJS) $(LIBOSD_OBJS) $(TLBFUNC_OBJS) $(ALARM_OBJS) $(EXIT_OBJS) $(SETUP_OBJS) setup_syscalls.o
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(KERNEL_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)kernel.S
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SIFCMD_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)sifcmd.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SIFRPC_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)sifrpc.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(FILEIO_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)fileio.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(LOADFILE_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)loadfile.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(CONFIG_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)osd_config.c
	$(EE_C_COMPILE) -I$(PS2SDKSRC)/ee/rpc/cdvd/include -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(TIMER_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)timer.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(GETKERNEL_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)getkernel.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SETUP_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)setup.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(IOPHEAP_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)iopheap.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(IOPCONTROL_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)iopcontrol.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(GLUE_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)glue.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(THREAD_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)thread.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SIO_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)sio.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(INITSYS_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)initsys.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(EXIT_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)exit.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(EE_OBJS_DIR)libosd.o: $(EE_SRC_DIR)libosd.c
	$(EE_C_COMPILE) -I$(PS2SDKSRC)/ee/rpc/cdvd/include $< -c -o $@

$(EE_OBJS_DIR)libosd_full.o: $(EE_SRC_DIR)libosd_full.c
	$(EE_C_COMPILE) -I$(PS2SDKSRC)/ee/rpc/cdvd/include $< -c -o $@

$(EE_OBJS_DIR)libosd_common.o: $(EE_SRC_DIR)libosd_common.c
	$(EE_C_COMPILE) -I$(PS2SDKSRC)/ee/rpc/cdvd/include $< -c -o $@

$(EE_OBJS_DIR)osdsrc_bin.o: $(EE_OBJS_DIR)osdsrc_bin.c
	$(EE_C_COMPILE) -I$(PS2SDKSRC)/ee/rpc/cdvd/include $< -c -o $@

$(EE_OBJS_DIR)libosd_syscalls.o: $(EE_SRC_DIR)libosd_syscalls.S
	$(EE_C_COMPILE) -I$(PS2SDKSRC)/ee/rpc/cdvd/include $< -c -o $@

$(EE_OBJS_DIR)tibfunc.o: $(EE_SRC_DIR)tlbfunc.c 
	$(EE_C_COMPILE) $< -c -o $@

$(EE_OBJS_DIR)tlbsrc_bin.o: $(EE_OBJS_DIR)tlbsrc_bin.c
	$(EE_C_COMPILE) $< -c -o $@

$(EE_OBJS_DIR)srcfile_bin.o: $(EE_OBJS_DIR)srcfile_bin.c
	$(EE_C_COMPILE) $< -c -o $@

$(EE_OBJS_DIR)eenull_bin.o: $(EE_OBJS_DIR)eenull_bin.c
	$(EE_C_COMPILE) $< -c -o $@

$(PS2SDKSRC)/tools/bin2c/bin/bin2c: $(PS2SDKSRC)/tools/bin2c
	$(GNUMAKE) -C $<

$(EE_SRC_DIR)osdsrc/bin/osdsrc.bin: $(EE_SRC_DIR)osdsrc
	$(GNUMAKE) -C $<

$(EE_OBJS_DIR)osdsrc_bin.c: $(EE_SRC_DIR)osdsrc/bin/osdsrc.bin $(PS2SDKSRC)/tools/bin2c/bin/bin2c
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c $< $@ osdsrc

$(EE_SRC_DIR)tlbsrc/bin/tlbsrc.bin: $(EE_SRC_DIR)tlbsrc
	$(GNUMAKE) -C $<

$(EE_OBJS_DIR)tlbsrc_bin.c: $(EE_SRC_DIR)tlbsrc/bin/tlbsrc.bin $(PS2SDKSRC)/tools/bin2c/bin/bin2c
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c $< $@ tlbsrc

$(EE_SRC_DIR)srcfile/bin/srcfile.bin: $(EE_SRC_DIR)srcfile
	$(GNUMAKE) -C $<

$(EE_OBJS_DIR)srcfile_bin.c: $(EE_SRC_DIR)srcfile/bin/srcfile.bin $(PS2SDKSRC)/tools/bin2c/bin/bin2c
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c $< $@ srcfile

$(EE_SRC_DIR)eenull/bin/eenull.bin: $(EE_SRC_DIR)eenull
	$(GNUMAKE) -C $<

$(EE_OBJS_DIR)eenull_bin.c: $(EE_SRC_DIR)eenull/bin/eenull.bin $(PS2SDKSRC)/tools/bin2c/bin/bin2c
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c $< $@ eenull

clean::
	$(GNUMAKE) -C $(EE_SRC_DIR)osdsrc clean
	$(GNUMAKE) -C $(EE_SRC_DIR)tlbsrc clean
	$(GNUMAKE) -C $(EE_SRC_DIR)srcfile clean
	$(GNUMAKE) -C $(EE_SRC_DIR)eenull clean
