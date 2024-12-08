# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

FATFS := $(PS2SDKSRC)/common/external_deps/fatfs

DEBUG ?= 0
DEBUG_EXTREME ?= 0

ifeq ($(DEBUG),1)
	IOP_CFLAGS += -DDEBUG
endif

ifeq ($(DEBUG_EXTREME),1)
	IOP_CFLAGS += -DDEBUG_EXTREME
endif

IOP_INCS += \
	-I$(PS2SDKSRC)/iop/cdvd/cdvdman/include \
	-I$(PS2SDKSRC)/iop/fs/bdm/include \
	-I$(PS2SDKSRC)/iop/system/intrman/include \
	-I$(PS2SDKSRC)/iop/system/ioman/include \
	-I$(PS2SDKSRC)/iop/system/iomanx/include \
	-I$(PS2SDKSRC)/iop/system/loadcore/include \
	-I$(PS2SDKSRC)/iop/system/stdio/include \
	-I$(PS2SDKSRC)/iop/system/sysclib/include \
	-I$(PS2SDKSRC)/iop/system/sysmem/include \
	-I$(PS2SDKSRC)/iop/system/threadman/include \
	-I$(FATFS)/source/include

IOP_LIBS += -lgcc

FATFS_OBJS = diskio.o ff.o ffsystem.o ffunicode.o 

IOP_OBJS = main.o imports.o fs_driver.o $(FATFS_OBJS)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

$(FATFS):
	$(MAKEREC) $(PS2SDKSRC)/common/external_deps all

vpath %.c $(FATFS)/source/

$(IOP_OBJS_DIR)%.o: %.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

.NOTPARALLEL:: \
	$(FATFS)
