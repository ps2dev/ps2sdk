# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# Build the newer version of the memory card server module?
MCMAN_BUILDING_XMCSERV ?= 1

# IOP_CFLAGS += -DSIO_DEBUG -DDEBUG

IOP_INCS += \
	-I$(PS2SDKSRC)/iop/debug/sior/include \
	-I$(PS2SDKSRC)/iop/memorycard/mcman/include \
	-I$(PS2SDKSRC)/iop/system/intrman/include \
	-I$(PS2SDKSRC)/iop/system/loadcore/include \
	-I$(PS2SDKSRC)/iop/system/sifcmd/include \
	-I$(PS2SDKSRC)/iop/system/sifman/include \
	-I$(PS2SDKSRC)/iop/system/stdio/include \
	-I$(PS2SDKSRC)/iop/system/sysclib/include \
	-I$(PS2SDKSRC)/iop/system/threadman/include

IOP_OBJS = mcserv.o imports.o exports.o

ifneq (x$(MCMAN_BUILDING_XMCSERV),x0)
IOP_CFLAGS += -DBUILDING_XMCSERV
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
