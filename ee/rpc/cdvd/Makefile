# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$


EE_OBJS_DIR = obj/
EE_LIB_DIR = lib/
EE_SRC_DIR = src/
EE_INC_DIR = include/

EE_LIB = libcdvd.a

LIBCDVD_OBJS = _libcdvd_internals.o cdInit.o cdIntToPos.o cdPosToInt.o \
    cdSearchFile.o cdDiskReady.o cdSemaInit.o cdSemaExit.o \
    cdInitCallbackThread.o cdSetCallback.o cdCallback.o
LIBCDVD_OBJS := $(LIBCDVD_OBJS:%=$(EE_OBJS_DIR)%)

NCMD_OBJS = _ncmd_internals.o _cdAlignReadBuffer.o cdRead.o cdDvdRead.o \
    cdCddaRead.o cdGetToc.o cdSeek.o cdStandby.o cdStop.o cdPause.o \
    cdApplyNCmd.o cdReadIOPMem.o cdNCmdDiskReady.o cdReadChain.o \
    cdGetReadPos.o cdStStart.o cdStRead.o cdStStop.o cdStSeek.o cdStInit.o \
    cdStStat.o cdStPause.o cdStResume.o cdStream.o cdCddaStream.o cdSync.o \
    cdCheckNCmd.o
NCMD_OBJS := $(NCMD_OBJS:%=$(EE_OBJS_DIR)%)

SCMD_OBJS = _scmd_internals.o cdReadClock.o cdWriteClock.o cdGetDiscType.o \
    cdGetError.o cdTrayReq.o cdApplySCmd.o cdStatus.o cdBreak.o \
    cdCancelPowerOff.o cdBlueLedCtrl.o cdPowerOff.o cdSetMediaMode.o \
    cdChangeThreadPriority.o cdCheckSCmd.o cdSyncS.o
SCMD_OBJS := $(SCMD_OBJS:%=$(EE_OBJS_DIR)%)

EE_LIB := $(EE_LIB:%=$(EE_LIB_DIR)%)
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

all: $(EE_OBJS_DIR) $(EE_LIB_DIR) $(EE_LIB)

.c.o :
	$(EE_CC) $(EE_CFLAGS) $(EE_INC) -c $< -o obj/$@

$(LIBCDVD_OBJS): src/libcdvd.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(NCMD_OBJS): src/ncmd.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(SCMD_OBJS): src/scmd.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

EE_OBJS = $(LIBCDVD_OBJS) $(NCMD_OBJS) $(SCMD_OBJS)

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_LIB_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release
