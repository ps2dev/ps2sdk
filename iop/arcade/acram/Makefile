# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

ACRAM_128MB ?= 0

IOP_IMPORT_INCS += \
	arcade/accore \
	system/intrman \
	system/loadcore \
	system/stdio \
	system/sysclib \
	system/sysmem \
	system/threadman

IOP_OBJS = \
	acram-entry.o \
	ram.o \
	imports.o \
	exports.o

ifeq ($(ACRAM_128MB),1)
  IOP_CFLAGS += -DALLOW_128MB_ACRAM
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
