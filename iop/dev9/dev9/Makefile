# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

#Enable to disable initialization of SMAP within DEV9 (i.e. clone adaptor compatibility).
DEV9_SKIP_SMAP_INIT ?= 0

#Enable to enable support for the PSX's devices
DEV9_PSX_SUPPORT ?= 1

#Disable to disable the workaround (skip SMAP initialization) for bad GameStar compatible adaptors.
DEV9_GAMESTAR_WORKAROUND ?= 1

IOP_BIN ?= ps2dev9.irx

IOP_INCS += -I$(PS2SDKSRC)/iop/dev9/poweroff/include
IOP_OBJS = ps2dev9.o exports.o imports.o

ifeq ($(DEV9_SKIP_SMAP_INIT),1)
IOP_CFLAGS += -DDEV9_SKIP_SMAP_INIT=1
endif

ifeq ($(DEV9_PSX_SUPPORT),1)
IOP_CFLAGS += -DDEV9_PSX_SUPPORT=1
endif

ifeq ($(DEV9_GAMESTAR_WORKAROUND),1)
IOP_CFLAGS += -DDEV9_GAMESTAR_WORKAROUND=1
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
