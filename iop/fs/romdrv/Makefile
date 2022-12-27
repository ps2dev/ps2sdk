# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_OBJS = romdrv.o imports.o exports.o

# Enable the romGetDevice function?
ROMDRV_EXPORT_DEVICES ?= 1

ifneq (x$(ROMDRV_EXPORT_DEVICES),x0)
IOP_CFLAGS += -DROMDRV_EXPORT_DEVICES
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
