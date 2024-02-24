# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_INCS += -I$(PS2SDKSRC)/iop/system/sio2man/include/

IOP_OBJS = secrman.o MechaAuth.o CardAuth.o keyman.o imports.o exports.o

# Define to 1, to enable runtime support for both CEX and DEX units.
DEX_SUPPORT ?= 1
COH_SUPPORT ?= 0

ifneq (x$(DEX_SUPPORT),x9)
IOP_CFLAGS += -DDEX_SUPPORT=1
endif
ifeq ($(COH_SUPPORT),1)
IOP_CFLAGS += -DBUILDING_SYSTEM2x6_SECRMAN=1
IOP_OBJS += DongleAuth.o
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
