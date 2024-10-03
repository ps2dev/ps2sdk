cmake_minimum_required(VERSION 3.10)

include(${PS2SDKSRC_ROOT}/cmake/utils.cmake)

add_subdirectory(src/eenull)
add_subdirectory(src/osdsrc)
add_subdirectory(src/srcfile)
add_subdirectory(src/tlbsrc)

add_library(kernel)
target_include_directories(kernel PUBLIC include)
target_include_directories(kernel PRIVATE ${EE_INC})

compile_multiple(kernel src/sifcmd.c OBJECTS 
    sif_cmd_send.o _sif_cmd_int_handler.o sif_cmd_main.o
	sif_cmd_client.o sif_cmd_remove_cmdhandler.o sif_sreg_get.o
)

compile_multiple(kernel src/sifrpc.c OBJECTS
    SifBindRpc.o SifCallRpc.o SifRpcGetOtherData.o
	SifRegisterRpc.o SifRemoveRpc.o SifSetRpcQueue.o SifRemoveRpcQueue.o SifGetNextRequest.o
	SifExecRequest.o SifRpcLoop.o SifRpcMain.o _rpc_get_packet.o
	_rpc_get_fpacket.o SifCheckStatRpc.o
)

compile_multiple(kernel src/fileio.c OBJECTS
    __fio_internals.o fio_init.o _fio_intr.o fio_sync.o fio_setblockmode.o 
	fio_exit.o fio_open.o fio_close.o fio_read.o 
	fio_write.o fio_lseek.o fio_mkdir.o _fio_read_intr.o fio_getc.o fio_putc.o 
	fio_ioctl.o fio_dopen.o fio_dclose.o fio_dread.o fio_getstat.o fio_chstat.o 
	fio_remove.o fio_format.o fio_rmdir.o fio_gets.o
)

compile_multiple(kernel src/loadfile.c OBJECTS
    SifLoadFileInit.o SifLoadFileExit.o _SifLoadModule.o SifLoadModule.o 
	SifLoadStartModule.o SifLoadModuleEncrypted.o SifStopModule.o SifUnloadModule.o 
	SifSearchModuleByName.o SifSearchModuleByAddress.o _SifLoadElfPart.o SifLoadElfPart.o 
	SifLoadElf.o SifLoadElfEncrypted.o SifIopSetVal.o SifIopGetVal.o 
	_SifLoadModuleBuffer.o SifLoadModuleBuffer.o SifLoadStartModuleBuffer.o 
	SifExecModuleBuffer.o SifExecModuleFile.o
)

compile_multiple(kernel src/iopheap.c OBJECTS
    SifInitIopHeap.o SifExitIopHeap.o SifAllocIopHeap.o 
	SifFreeIopHeap.o SifLoadIopHeap.o
)

compile_multiple(kernel src/iopcontrol.c OBJECTS
    SifIopReboot.o SifIopReset.o SifIopIsAlive.o SifIopSync.o __iop_control_internals.o
)

compile_multiple(kernel src/glue.c OBJECTS
    DIntr.o EIntr.o EnableIntc.o DisableIntc.o EnableDmac.o DisableDmac.o
    iEnableIntc.o iDisableIntc.o iEnableDmac.o iDisableDmac.o
    SyncDCache.o iSyncDCache.o InvalidDCache.o iInvalidDCache.o
)

compile_multiple(kernel src/sio.c OBJECTS
    sio_init.o sio_putc.o sio_getc.o sio_write.o sio_read.o sio_puts.o
	sio_gets.o sio_getc_block.o sio_flush.o sio_putsn.o
)

compile_multiple(kernel src/rom0_info.c OBJECTS
    _info_internals.o GetRomNameWithIODriver.o GetRomName.o IsDESRMachineWithIODriver.o IsDESRMachine.o IsT10KWithIODriver.o IsT10K.0
)

compile_multiple(kernel src/osd_config.c OBJECTS
    _config_internals.o converttobcd.o convertfrombcd.o __adjustTime.o IsEarlyJap.o 
	configGetLanguageWithIODriver.o configGetLanguage.o
	configSetLanguageWithIODriver.o configSetLanguage.o
	configGetTvScreenTypeWithIODriver.o configGetTvScreenType.o
	configSetTvScreenTypeWithIODriver.o configSetTvScreenType.o
	configGetDateFormatWithIODriver.o configGetDateFormat.o
	configSetDateFormatWithIODriver.o configSetDateFormat.o
	configGetTimeFormatWithIODriver.o configGetTimeFormat.o
	configSetTimeFormatWithIODriver.o configSetTimeFormat.o
	configGetTimezoneWithIODriver.o configGetTimezone.o
	configSetTimezoneWithIODriver.o configSetTimezone.o
	configIsSpdifEnabledWithIODriver.o configIsSpdifEnabled.o
	configSetSpdifEnabledWithIODriver.o configSetSpdifEnabled.o
	configIsDaylightSavingEnabledWithIODriver.o configIsDaylightSavingEnabled.o
	configSetDaylightSavingEnabledWithIODriver.o configSetDaylightSavingEnabled.o
	configConvertToGmtTime.o configConvertToLocalTimeWithIODriver.o configConvertToLocalTime.o
)

# Generates a C array of the binary output of a target
# objcopy -Obinary <elf> <bin> && bin2c <bin> <output_name.c> <target_name>
macro(bin_include from_target output_name)
add_custom_command(OUTPUT "${output_name}.c"
  COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${from_target}> "${output_name}.bin"
  COMMAND bin2c "${output_name}.bin" "${output_name}.c" "${from_target}"
  BYPRODUCTS "${output_name}.bin"
  DEPENDS ${from_target}
)
endmacro()

bin_include(osdsrc osdsrc_bin)
target_sources(kernel PRIVATE src/libosd.c src/libosd_full.c src/libosd_common.c osdsrc_bin.c)

bin_include(tlbsrc tlbsrc_bin)
target_sources(kernel PRIVATE src/tlbfunc.c tlbsrc_bin.c)

bin_include(eenull eenull_bin)
bin_include(srcfile srcfile_bin)
target_sources(kernel PRIVATE src/alarm.c srcfile_bin.c)

compile_multiple(kernel src/thread.c OBJECTS
	_thread_internals.o iWakeupThread.o iRotateThreadReadyQueue.o iSuspendThread.o)

compile_multiple(kernel src/setup.c OBJECTS kCopy.o kCopyBytes.o)

compile_multiple(kernel src/exit.c OBJECTS
	_exit_internals.o SetArg.o Exit.o ExecPS2.o LoadExecPS2.o ExecOSD.o)

compile_multiple(kernel src/timer.c OBJECTS
	timer_data.o
	SetT2.o
	SetT2_COUNT.o
	SetT2_MODE.o
	SetT2_COMP.o
	InitTimer.o
	EndTimer.o 
	GetTimerPreScaleFactor.o
	StartTimerSystemTime.o
	StopTimerSystemTime.o
	SetNextComp.o
	InsertAlarm_ForTimer.o
	UnlinkAlarm_ForTimer.o
	TimerHandler_callback.o
	iGetTimerSystemTime.o
	GetTimerSystemTime.o
	iAllocTimerCounter.o
	AllocTimerCounter.o
	iFreeTimerCounter.o
	FreeTimerCounter.o
	iGetTimerUsedUnusedCounters.o
	GetTimerUsedUnusedCounters.o
	iStartTimerCounter.o
	StartTimerCounter.o
	iStopTimerCounter.o
	StopTimerCounter.o
	SetTimerCount.o
	iGetTimerBaseTime.o
	GetTimerBaseTime.o
	iGetTimerCount.o
	GetTimerCount.o
	iSetTimerHandler.o
	SetTimerHandler.o
	TimerBusClock2USec.o
	TimerUSec2BusClock.o
	TimerBusClock2Freq.o
	TimerFreq2BusClock.o
	cpu_ticks.o
)

compile_multiple(kernel src/timer_alarm.c OBJECTS
	alarm_data.o
	ForTimer_InitAlarm.o
	AlarmHandler.o
	iSetTimerAlarm.o
	SetTimerAlarm.o
	iReleaseTimerAlarm.o
	ReleaseTimerAlarm.o
)

compile_multiple(kernel src/delaythread.c OBJECTS DelayThread.o)

compile_multiple(kernel src/getkernel.c OBJECTS 
	GetSyscallHandler.o GetSyscall.o GetExceptionHandler.o GetInterruptHandler.o)

compile_multiple(kernel src/initsys.c OBJECTS _InitSys.o TerminateLibrary.o)

compile_multiple(kernel src/kernel_util.c OBJECTS WaitSemaEx.o)

# IDK why these were on their own in the makefile, keep it for now
set(EXEC_SYSCALLS KExit.o _LoadExecPS2.o _ExecPS2.o)
set(EXECOSD_SYSCALL _ExecOSD.o)
set(ALARM_SYSCALLS _SetAlarm.o SetAlarm.o _ReleaseAlarm.o ReleaseAlarm.o)
set(ALARM_INTR_SYSCALLS _iSetAlarm.o iSetAlarm.o _iReleaseAlarm.o iReleaseAlarm.o)
set(ROTATE_THREAD_READY_QUEUE_SYSCALL _iRotateThreadReadyQueue.o)
set(IWAKEUP_THREAD_SYSCALL _iWakeupThread.o)
set(ISUSPEND_THREAD_SYSCALL _iSuspendThread.o)
set(TLB_SYSCALLS PutTLBEntry.o iPutTLBEntry.o _SetTLBEntry.o iSetTLBEntry.o GetTLBEntry.o iGetTLBEntry.o ProbeTLBEntry.o iProbeTLBEntry.o ExpandScratchPad.o)

compile_multiple(kernel src/kernel.S OBJECTS 
	ResetEE.o SetGsCrt.o ${EXEC_SYSCALLS}
	RFU009.o AddSbusIntcHandler.o RemoveSbusIntcHandler.o Interrupt2Iop.o
	SetVTLBRefillHandler.o SetVCommonHandler.o SetVInterruptHandler.o
	AddIntcHandler.o AddIntcHandler2.o RemoveIntcHandler.o AddDmacHandler.o AddDmacHandler2.o
	RemoveDmacHandler.o _EnableIntc.o _DisableIntc.o _EnableDmac.o _DisableDmac.o
	${ALARM_SYSCALLS} _iEnableIntc.o _iDisableIntc.o _iEnableDmac.o
	_iDisableDmac.o ${ALARM_INTR_SYSCALLS} CreateThread.o DeleteThread.o
	StartThread.o ExitThread.o ExitDeleteThread.o TerminateThread.o
	iTerminateThread.o DisableDispatchThread.o EnableDispatchThread.o
	ChangeThreadPriority.o iChangeThreadPriority.o RotateThreadReadyQueue.o
	${ROTATE_THREAD_READY_QUEUE_SYSCALL} ReleaseWaitThread.o iReleaseWaitThread.o
	GetThreadId.o _iGetThreadId.o ReferThreadStatus.o iReferThreadStatus.o SleepThread.o
	WakeupThread.o ${IWAKEUP_THREAD_SYSCALL} CancelWakeupThread.o iCancelWakeupThread.o
	SuspendThread.o ${ISUSPEND_THREAD_SYSCALL} ResumeThread.o iResumeThread.o
	RFU059.o RFU060.o SetupThread.o RFU061.o SetupHeap.o EndOfHeap.o CreateSema.o DeleteSema.o
	iSignalSema.o SignalSema.o WaitSema.o PollSema.o iPollSema.o
	ReferSemaStatus.o iReferSemaStatus.o iDeleteSema.o SetOsdConfigParam.o
	GetOsdConfigParam.o GetGsHParam.o GetGsVParam.o SetGsHParam.o
	SetGsVParam.o CreateEventFlag.o DeleteEventFlag.o SetEventFlag.o
	iSetEventFlag.o ${TLB_SYSCALLS}
	EnableIntcHandler.o iEnableIntcHandler.o DisableIntcHandler.o iDisableIntcHandler.o
	EnableDmacHandler.o iEnableDmacHandler.o DisableDmacHandler.o iDisableDmacHandler.o
	KSeg0.o EnableCache.o DisableCache.o GetCop0.o FlushCache.o CpuConfig.o
	iGetCop0.o iFlushCache.o RFU105.o iCpuConfig.o SifStopDma.o
	SetCPUTimerHandler.o SetCPUTimer.o SetOsdConfigParam2.o
	GetOsdConfigParam2.o GsGetIMR.o iGsGetIMR.o GsPutIMR.o iGsPutIMR.o
	SetPgifHandler.o SetVSyncFlag.o SetSyscall.o SifDmaStat.o iSifDmaStat.o
	SifSetDma.o iSifSetDma.o SifSetDChain.o iSifSetDChain.o SifSetReg.o
	SifGetReg.o ${EXECOSD_SYSCALL} Deci2Call.o PSMode.o MachineType.o GetMemorySize.o _GetGsDxDyOffset.o
	_InitTLB.o SetMemoryMode.o
	SifWriteBackDCache.o _SyncDCache.o _InvalidDCache.o __errno.o errno.o
	strncpy.o strlen.o memcpy.o memset.o __syscall.o GPfuncs.o _print.o
)

target_sources(kernel PRIVATE src/setup_syscalls.S src/debug.c)

file(GLOB KERNEL_INCLUDE_FILES "include/*.h*")
set_target_properties(kernel PROPERTIES PUBLIC_HEADER "${KERNEL_INCLUDE_FILES}")

install(TARGETS kernel)