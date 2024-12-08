# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# Define to 1, to enable runtime support for both CEX and DEX units.
DEX_SUPPORT ?= 1

IOP_INCS += \
	-I$(PS2SDKSRC)/iop/cdvd/cdvdman/include \
	-I$(PS2SDKSRC)/iop/system/ioman/include \
	-I$(PS2SDKSRC)/iop/system/loadcore/include \
	-I$(PS2SDKSRC)/iop/system/modload/include \
	-I$(PS2SDKSRC)/iop/system/sio2man/include \
	-I$(PS2SDKSRC)/iop/system/stdio/include \
	-I$(PS2SDKSRC)/iop/system/sysclib/include

IOP_OBJS = secrman.o MechaAuth.o CardAuth.o keyman.o imports.o exports.o

ifneq (x$(DEX_SUPPORT),x9)
IOP_CFLAGS += -DDEX_SUPPORT=1
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
