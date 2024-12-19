add_library(cdvd)
target_include_directories(cdvd PUBLIC include)
target_include_directories(cdvd PRIVATE ${EE_INC})

compile_multiple(cdvd src/libcdvd.c OBJECTS 
    _libcdvd_internals.o sceCdInit.o sceCdIntToPos.o sceCdPosToInt.o
	sceCdSearchFile.o sceCdDiskReady.o _CdSemaInit.o _CdSemaExit.o
	sceCdInitEeCB.o sceCdCallback.o _CdGenericCallbackFunction.o
)

compile_multiple(cdvd src/ncmd.c OBJECTS 
    _ncmd_internals.o _CdAlignReadBuffer.o sceCdRead.o sceCdReadDVDV.o
	sceCdReadCDDA.o sceCdGetToc.o sceCdSeek.o sceCdStandby.o sceCdStop.o sceCdPause.o
	sceCdApplyNCmd.o sceCdReadIOPMem.o sceCdNCmdDiskReady.o
	sceCdGetReadPos.o sceCdStStart.o sceCdStRead.o sceCdStStop.o sceCdStSeek.o sceCdStInit.o
	sceCdStStat.o sceCdStPause.o sceCdStResume.o sceCdStream.o sceCdCddaStream.o sceCdSync.o
	_CdCheckNCmd.o sceCdReadKey.o
)

compile_multiple(cdvd src/scmd.c OBJECTS 
    _scmd_internals.o sceCdReadClock.o ps2time.o sceCdWriteClock.o sceCdGetDiskType.o
	sceCdGetError.o sceCdTrayReq.o sceCdApplySCmd.o sceCdStatus.o sceCdBreak.o
	_CdCheckSCmd.o sceCdCtrlADout.o sceCdMV.o sceCdReadSUBQ.o
	sceCdForbidDVDP.o sceCdAutoAdjustCtrl.o sceCdDecSet.o sceCdSetHDMode.o sceCdOpenConfig.o sceCdCloseConfig.o
	sceCdReadConfig.o sceCdWriteConfig.o
	sceCdReadNVM.o sceCdWriteNVM.o sceCdRI.o sceCdWI.o sceCdReadConsoleID.o sceCdWriteConsoleID.o
	sceCdNoticeGameStart.o
	_CdSyncS.o
)

target_sources(cdvd PRIVATE src/erl-support.c)
install(TARGETS cdvd)
#target_add_erl(cdvd) TODO doesn't work with the object libraries