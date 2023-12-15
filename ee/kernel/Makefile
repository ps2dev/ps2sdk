# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.


EXEC_SYSCALLS = KExit.o _LoadExecPS2.o _ExecPS2.o
EXECOSD_SYSCALL = _ExecOSD.o
ALARM_SYSCALLS = _SetAlarm.o SetAlarm.o _ReleaseAlarm.o ReleaseAlarm.o
ALARM_INTR_SYSCALLS = _iSetAlarm.o iSetAlarm.o _iReleaseAlarm.o iReleaseAlarm.o
ROTATE_THREAD_READY_QUEUE_SYSCALL = _iRotateThreadReadyQueue.o
IWAKEUP_THREAD_SYSCALL = _iWakeupThread.o
ISUSPEND_THREAD_SYSCALL = _iSuspendThread.o
TLB_SYSCALLS = PutTLBEntry.o iPutTLBEntry.o _SetTLBEntry.o iSetTLBEntry.o GetTLBEntry.o iGetTLBEntry.o ProbeTLBEntry.o iProbeTLBEntry.o ExpandScratchPad.o

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
GLUE_OBJS += iEnableIntc.o iDisableIntc.o iEnableDmac.o iDisableDmac.o
GLUE_OBJS += SyncDCache.o iSyncDCache.o InvalidDCache.o iInvalidDCache.o

### SIO objects

SIO_OBJS = sio_init.o sio_putc.o sio_getc.o sio_write.o sio_read.o sio_puts.o \
	sio_gets.o sio_getc_block.o sio_flush.o sio_putsn.o

ROM0_OBJS = _info_internals.o GetRomNameWithIODriver.o GetRomName.o IsDESRMachineWithIODriver.o IsDESRMachine.o IsT10KWithIODriver.o IsT10K.o

### Config objects

CONFIG_OBJS = _config_internals.o converttobcd.o convertfrombcd.o __adjustTime.o IsEarlyJap.o \
	configGetLanguageWithIODriver.o configGetLanguage.o \
	configSetLanguageWithIODriver.o configSetLanguage.o \
	configGetTvScreenTypeWithIODriver.o configGetTvScreenType.o \
	configSetTvScreenTypeWithIODriver.o configSetTvScreenType.o \
	configGetDateFormatWithIODriver.o configGetDateFormat.o \
	configSetDateFormatWithIODriver.o configSetDateFormat.o \
	configGetTimeFormatWithIODriver.o configGetTimeFormat.o \
	configSetTimeFormatWithIODriver.o configSetTimeFormat.o \
	configGetTimezoneWithIODriver.o configGetTimezone.o \
	configSetTimezoneWithIODriver.o configSetTimezone.o \
	configIsSpdifEnabledWithIODriver.o configIsSpdifEnabled.o \
	configSetSpdifEnabledWithIODriver.o configSetSpdifEnabled.o \
	configIsDaylightSavingEnabledWithIODriver.o configIsDaylightSavingEnabled.o \
	configSetDaylightSavingEnabledWithIODriver.o configSetDaylightSavingEnabled.o \
	configConvertToGmtTime.o configConvertToLocalTimeWithIODriver.o configConvertToLocalTime.o

### Patch objects

LIBOSD_OBJS = libosd.o libosd_full.o libosd_common.o osdsrc_bin.o

TLBFUNC_OBJS = tlbfunc.o tlbsrc_bin.o

ALARM_OBJS = alarm.o srcfile_bin.o eenull_bin.o

THREAD_OBJS = _thread_internals.o iWakeupThread.o iRotateThreadReadyQueue.o iSuspendThread.o

SETUP_OBJS = kCopy.o kCopyBytes.o

EXIT_OBJS += _exit_internals.o SetArg.o Exit.o ExecPS2.o LoadExecPS2.o ExecOSD.o

### Timer objects

TIMER_OBJS = __time_internals.o StartTimerSystemTime.o StopTimerSystemTime.o iGetTimerSystemTime.o GetTimerSystemTime.o cpu_ticks.o
TIMER_ALARM_OBJS = TimerAlarmInternals.o __Timer2Resched.o InitTimerAlarm.o DeinitTimerAlarm.o InitializeTimerAlarm.o \
	iStopTimerAlarm.o StopTimerAlarm.o iStartTimerAlarm.o StartTimerAlarm.o SetTimerAlarm.o ThreadWaitClock.o

### Getter objects

GETKERNEL_OBJS = GetSyscallHandler.o GetSyscall.o GetExceptionHandler.o GetInterruptHandler.o

### System initialization objects

INITSYS_OBJS = _InitSys.o TerminateLibrary.o

### Kernel utility objects

KERNEL_UTIL_OBJS = WaitSemaEx.o

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
	SifGetReg.o $(EXECOSD_SYSCALL) Deci2Call.o PSMode.o MachineType.o GetMemorySize.o _GetGsDxDyOffset.o \
	_InitTLB.o SetMemoryMode.o \
	SifWriteBackDCache.o _SyncDCache.o _InvalidDCache.o __errno.o errno.o \
	strncpy.o strlen.o memcpy.o memset.o __syscall.o GPfuncs.o _print.o

EE_OBJS = $(KERNEL_OBJS) $(SIFCMD_OBJS) $(SIFRPC_OBJS) $(FILEIO_OBJS) \
	$(LOADFILE_OBJS) $(IOPHEAP_OBJS) $(IOPCONTROL_OBJS) $(ROM0_OBJS) $(CONFIG_OBJS) \
	$(GLUE_OBJS) $(SIO_OBJS) $(TIMER_OBJS) $(TIMER_ALARM_OBJS) $(GETKERNEL_OBJS) \
	$(INITSYS_OBJS) $(KERNEL_UTIL_OBJS) erl-support.o


EE_OBJS += $(THREAD_OBJS) $(LIBOSD_OBJS) $(TLBFUNC_OBJS) $(ALARM_OBJS) $(EXIT_OBJS) $(SETUP_OBJS) setup_syscalls.o debug.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(KERNEL_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)kernel.S
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SIFCMD_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)sifcmd.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SIFRPC_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)sifrpc.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(FILEIO_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)fileio.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(LOADFILE_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)loadfile.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(ROM0_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)rom0_info.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(CONFIG_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)osd_config.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -I$(PS2SDKSRC)/ee/rpc/cdvd/include -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(TIMER_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)timer.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(TIMER_ALARM_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)timer_alarm.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(GETKERNEL_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)getkernel.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SETUP_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)setup.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(IOPHEAP_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)iopheap.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(IOPCONTROL_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)iopcontrol.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(GLUE_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)glue.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(THREAD_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)thread.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SIO_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)sio.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(INITSYS_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)initsys.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(KERNEL_UTIL_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)kernel_util.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(EXIT_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)exit.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(EE_OBJS_DIR)libosd.o: $(EE_SRC_DIR)libosd.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -I$(PS2SDKSRC)/ee/rpc/cdvd/include $< -c -o $@

$(EE_OBJS_DIR)libosd_full.o: $(EE_SRC_DIR)libosd_full.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -I$(PS2SDKSRC)/ee/rpc/cdvd/include $< -c -o $@

$(EE_OBJS_DIR)libosd_common.o: $(EE_SRC_DIR)libosd_common.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -I$(PS2SDKSRC)/ee/rpc/cdvd/include $< -c -o $@

$(EE_OBJS_DIR)osdsrc_bin.o: $(EE_OBJS_DIR)osdsrc_bin.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -I$(PS2SDKSRC)/ee/rpc/cdvd/include $< -c -o $@

$(EE_OBJS_DIR)libosd_syscalls.o: $(EE_SRC_DIR)libosd_syscalls.S
	$(DIR_GUARD)
	$(EE_C_COMPILE) -I$(PS2SDKSRC)/ee/rpc/cdvd/include $< -c -o $@

$(EE_OBJS_DIR)tlbfunc.o: $(EE_SRC_DIR)tlbfunc.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) $< -c -o $@

$(EE_OBJS_DIR)tlbsrc_bin.o: $(EE_OBJS_DIR)tlbsrc_bin.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) $< -c -o $@

$(EE_OBJS_DIR)srcfile_bin.o: $(EE_OBJS_DIR)srcfile_bin.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) $< -c -o $@

$(EE_OBJS_DIR)eenull_bin.o: $(EE_OBJS_DIR)eenull_bin.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) $< -c -o $@

$(PS2SDKSRC)/tools/bin2c/bin/bin2c: $(PS2SDKSRC)/tools/bin2c
	$(MAKEREC) $<

$(EE_SRC_DIR)osdsrc/bin/osdsrc.bin: $(EE_SRC_DIR)osdsrc
	$(MAKEREC) $<

$(EE_OBJS_DIR)osdsrc_bin.c: $(EE_SRC_DIR)osdsrc/bin/osdsrc.bin $(PS2SDKSRC)/tools/bin2c/bin/bin2c
	$(DIR_GUARD)
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c $< $@ osdsrc

$(EE_SRC_DIR)tlbsrc/bin/tlbsrc.bin: $(EE_SRC_DIR)tlbsrc
	$(MAKEREC) $<

$(EE_OBJS_DIR)tlbsrc_bin.c: $(EE_SRC_DIR)tlbsrc/bin/tlbsrc.bin $(PS2SDKSRC)/tools/bin2c/bin/bin2c
	$(DIR_GUARD)
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c $< $@ tlbsrc

$(EE_SRC_DIR)srcfile/bin/srcfile.bin: $(EE_SRC_DIR)srcfile
	$(MAKEREC) $<

$(EE_OBJS_DIR)srcfile_bin.c: $(EE_SRC_DIR)srcfile/bin/srcfile.bin $(PS2SDKSRC)/tools/bin2c/bin/bin2c
	$(DIR_GUARD)
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c $< $@ srcfile

$(EE_SRC_DIR)eenull/bin/eenull.bin: $(EE_SRC_DIR)eenull
	$(MAKEREC) $<

$(EE_OBJS_DIR)eenull_bin.c: $(EE_SRC_DIR)eenull/bin/eenull.bin $(PS2SDKSRC)/tools/bin2c/bin/bin2c
	$(DIR_GUARD)
	$(PS2SDKSRC)/tools/bin2c/bin/bin2c $< $@ eenull

clean::
	$(MAKEREC) $(EE_SRC_DIR)osdsrc clean
	$(MAKEREC) $(EE_SRC_DIR)tlbsrc clean
	$(MAKEREC) $(EE_SRC_DIR)srcfile clean
	$(MAKEREC) $(EE_SRC_DIR)eenull clean
