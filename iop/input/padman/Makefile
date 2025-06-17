# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_BIN_ALTNAMES ?= freepad.irx

# IOP_CFLAGS += -DDEBUG

IOP_IMPORT_INCS += \
	sio/sio2man \
	system/intrman \
	system/loadcore \
	system/sifcmd \
	system/sifman \
	system/stdio \
	system/sysclib \
	system/sysmem \
	system/threadman \
	system/vblank

IOP_OBJS = freepad.o rpcserver.o exports.o imports.o padInit.o padPortOpen.o padMiscFuncs.o sio2Cmds.o padData.o padCmds.o

# Build the newer version of the gamepad module?
PADMAN_BUILDING_XPADMAN ?= 1

# Build the newer version of the gamepad module that links against the remote-compatible SIO2MAN?
PADMAN_BUILDING_XPADMAN_V2 ?= 0

ifneq (x$(PADMAN_BUILDING_XPADMAN),x0)
IOP_CFLAGS += -DBUILDING_XPADMAN
endif

ifneq (x$(PADMAN_BUILDING_XPADMAN_V2),x0)
IOP_CFLAGS += -DBUILDING_XPADMAN_V2
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
