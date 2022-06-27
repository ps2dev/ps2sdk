# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

LIBCDVD_OBJS += _libcdvd_internals.o sceCdInit.o sceCdIntToPos.o sceCdPosToInt.o \
	sceCdSearchFile.o sceCdDiskReady.o _CdSemaInit.o _CdSemaExit.o \
	sceCdInitEeCB.o sceCdCallback.o _CdGenericCallbackFunction.o

NCMD_OBJS += _ncmd_internals.o _CdAlignReadBuffer.o sceCdRead.o sceCdReadDVDV.o \
	sceCdReadCDDA.o sceCdGetToc.o sceCdSeek.o sceCdStandby.o sceCdStop.o sceCdPause.o \
	sceCdApplyNCmd.o sceCdReadIOPMem.o sceCdNCmdDiskReady.o \
	sceCdGetReadPos.o sceCdStStart.o sceCdStRead.o sceCdStStop.o sceCdStSeek.o sceCdStInit.o \
	sceCdStStat.o sceCdStPause.o sceCdStResume.o sceCdStream.o sceCdCddaStream.o sceCdSync.o \
	_CdCheckNCmd.o sceCdReadKey.o

SCMD_OBJS += _scmd_internals.o sceCdReadClock.o ps2time.o sceCdWriteClock.o sceCdGetDiskType.o \
	sceCdGetError.o sceCdTrayReq.o sceCdApplySCmd.o sceCdStatus.o sceCdBreak.o \
	_CdCheckSCmd.o sceCdCtrlADout.o sceCdMV.o sceCdReadSUBQ.o \
	sceCdForbidDVDP.o sceCdAutoAdjustCtrl.o sceCdDecSet.o sceCdSetHDMode.o sceCdOpenConfig.o sceCdCloseConfig.o \
	sceCdReadConfig.o sceCdWriteConfig.o \
	sceCdReadNVM.o sceCdWriteNVM.o sceCdRI.o sceCdWI.o sceCdReadConsoleID.o sceCdWriteConsoleID.o \
	sceCdNoticeGameStart.o \
	_CdSyncS.o

EE_OBJS = $(LIBCDVD_OBJS) $(NCMD_OBJS) $(SCMD_OBJS) erl-support.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(LIBCDVD_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)libcdvd.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(NCMD_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)ncmd.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SCMD_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)scmd.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@
