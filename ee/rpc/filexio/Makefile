# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_LIB = libfileXio.a

CORE_OBJS = erl-support.o

FILEXIO_RPC_OBJS = \
	__cd0.o __sbuff.o __intr_data.o __fileXioInited.o __fileXioBlockMode.o __fileXioCompletionSema.o __lock_sema_id.o \
	fileXioInit.o fileXioExit.o fileXioStop.o fileXioGetDeviceList.o fileXioGetdir.o fileXioMount.o fileXioUmount.o \
	fileXioCopyfile.o fileXioMkdir.o fileXioRmdir.o fileXioRemove.o fileXioRename.o fileXioSymlink.o fileXioReadlink.o \
	fileXioChdir.o fileXioOpen.o fileXioClose.o fileXioRead.o fileXioWrite.o fileXioLseek.o fileXioLseek64.o fileXioChStat.o \
	fileXioGetStat.o fileXioFormat.o fileXioSync.o fileXioDopen.o fileXioDclose.o fileXioDread.o fileXioDevctl.o \
	fileXioIoctl.o fileXioIoctl2.o fileXioWaitAsync.o fileXioSetBlockMode.o fileXioSetRWBufferSize.o

FILEXIO_PS2SDK_OBJS = \
	__fileXioGetstatHelper.o __fileXioDreadHelper.o _ps2sdk_fileXio_init_deinit.o

EE_OBJS = $(CORE_OBJS) $(FILEXIO_RPC_OBJS) $(FILEXIO_PS2SDK_OBJS) 

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(FILEXIO_RPC_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)fileXio_rpc.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(FILEXIO_PS2SDK_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)fileXio_ps2sdk.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@