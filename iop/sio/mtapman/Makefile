# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_BIN_ALTNAMES ?= freemtap.irx rmtapman.irx mtapman-2000.irx

IOP_IMPORT_INCS += \
	sio/sio2man \
	system/loadcore \
	system/sifcmd \
	system/sifman \
	system/stdio \
	system/threadman

IOP_OBJS = freemtap.o rpcservers.o exports.o imports.o

# Build the newer version of the multitap module that links against the remote-compatible SIO2MAN?
MTAPMAN_BUILDING_XMTAPMAN_V2 ?= 1

ifneq (x$(MTAPMAN_BUILDING_XMTAPMAN_V2),x0)
IOP_CFLAGS += -DSIO2MAN_V2
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
