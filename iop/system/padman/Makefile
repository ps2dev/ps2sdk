# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_BIN_ALTNAMES ?= freepad.irx

# IOP_CFLAGS += -DDEBUG

IOP_INCS += -I$(PS2SDKSRC)/iop/system/sio2man/include

IOP_OBJS = freepad.o rpcserver.o exports.o imports.o padInit.o padPortOpen.o padMiscFuncs.o sio2Cmds.o padData.o padCmds.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
