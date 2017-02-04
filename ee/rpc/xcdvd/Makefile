# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_SRC_DIR = $(PS2SDKSRC)/ee/rpc/cdvd/src/
EE_INC_DIR = $(PS2SDKSRC)/ee/rpc/cdvd/include/

EE_CFLAGS += -D_XCDVD

NCMD_OBJS += sceCdReadChain.o

SCMD_OBJS += sceCdCancelPOffRdy.o sceCdBlueLedCtrl.o sceCdPowerOff.o sceCdMmode.o \
	sceCdForbidRead.o sceCdSpinCtrlEE.o sceCdBootCertify.o sceCdRM.o sceCdWM.o \
	sceCdChangeThreadPriority.o

include $(PS2SDKSRC)/ee/rpc/cdvd/Makefile
