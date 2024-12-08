# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# Build the newer version of the SIO2 module with support for multitap?
SIO2MAN_BUILDING_XSIO2MAN ?= 1

# Build the newer version of the SIO2 module with support for remote and PDA devices?
SIO2MAN_BUILDING_XSIO2MAN_V2 ?= 0

# Enable logging?
SIO2MAN_ENABLE_LOGGING ?= 0

IOP_BIN_ALTNAMES ?= freesio2.irx

IOP_INCS += \
	-I$(PS2SDKSRC)/iop/system/dmacman/include \
	-I$(PS2SDKSRC)/iop/system/intrman/include \
	-I$(PS2SDKSRC)/iop/system/ioman/include \
	-I$(PS2SDKSRC)/iop/system/loadcore/include \
	-I$(PS2SDKSRC)/iop/system/sio2man/include \
	-I$(PS2SDKSRC)/iop/system/stdio/include \
	-I$(PS2SDKSRC)/iop/system/threadman/include

IOP_OBJS = sio2man.o log.o imports.o exports.o

ifneq (x$(SIO2MAN_BUILDING_XSIO2MAN),x0)
IOP_CFLAGS += -DBUILDING_XSIO2MAN
endif

ifneq (x$(SIO2MAN_BUILDING_XSIO2MAN_V2),x0)
IOP_CFLAGS += -DBUILDING_XSIO2MAN_V2
endif

ifneq (x$(SIO2MAN_ENABLE_LOGGING),x0)
IOP_CFLAGS += -DSIO2LOG
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
