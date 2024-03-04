# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_BIN ?= intrman.irx

# Enable compatibility with PS1 mode?
INTRMAN_PS1 ?= 0

IOP_OBJS = intrman.o exports.o imports.o

ifneq (x$(INTRMAN_PS1),x0)
IOP_CFLAGS += -DBUILDING_INTRMANP
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
