# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# Define to 1, to enable runtime support for both CEX and DEX units.
DEX_SUPPORT ?= 1

IOP_IMPORT_INCS += \
	cdvd/cdvdman \
	system/ioman \
	system/loadcore \
	system/modload \
	system/sio2man \
	system/stdio \
	system/sysclib

IOP_OBJS = secrman.o MechaAuth.o CardAuth.o keyman.o imports.o exports.o

ifneq (x$(DEX_SUPPORT),x9)
IOP_CFLAGS += -DDEX_SUPPORT=1
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
