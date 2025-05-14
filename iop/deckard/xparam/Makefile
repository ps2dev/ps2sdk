# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2024, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_BIN = xparam.irx

DATABASE ?= 0#0: Homebrew, 1: ROM 2.20, 2: ROM 2.50

IOP_CSTDFLAGS ?= -std=gnu99

IOP_OBJS = xparam.o imports.o


IOP_INCS += $(addprefix -I$(PS2SDKSRC)/iop/system/, sysclib/include loadcore/include)

ifeq ($(DATABASE),0)
  IOP_OBJS += database_merged.o
else ifeq ($(DATABASE),1)
  IOP_OBJS += database_0220_20050620.o
else ifeq ($(DATABASE),2)
  IOP_OBJS += database_0250_20100415.o 
else
  $(error Unknown DB value)
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
