# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# Enable compatibility with PS1 mode?
TIMRMAN_PS1 ?= 0

IOP_BIN_ALTNAMES ?= timemani.irx

IOP_INCS += \
	-I$(PS2SDKSRC)/iop/system/intrman/include \
	-I$(PS2SDKSRC)/iop/system/loadcore/include

IOP_OBJS = timrman.o imports.o exports.o

ifneq (x$(TIMRMAN_PS1),x0)
IOP_CFLAGS += -DBUILDING_TIMRMANP
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
