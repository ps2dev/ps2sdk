# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

HDLFS_SUPPORT ?= 1

LIBAPA_PATH = $(PS2SDKSRC)/iop/hdd/libapa/

LIBAPA_SRC_DIR = $(LIBAPA_PATH)src/

IOP_BIN ?= hdsk.irx

IOP_CFLAGS += -DIOMANX_OLD_NAME_COMPATIBILITY=0 -DIOMANX_OLD_NAME_ADDDELDRV=0

IOP_CFLAGS += -DAPA_OSD_VER

IOP_IMPORT_INCS += \
	cdvd/cdvdman \
	dev9/atad \
	dev9/dev9 \
	dev9/poweroff \
	system/intrman \
	system/iomanx \
	system/loadcore \
	system/stdio \
	system/sysclib \
	system/sysmem \
	system/threadman

IOP_INCS += -I$(LIBAPA_PATH)include

APA_OBJS = misc.o cache.o apa.o journal.o free.o
IOP_OBJS = hdsk.o misc_hdsk.o sim.o imports.o $(APA_OBJS)

ifneq (x$(HDLFS_SUPPORT),x0)
IOP_CFLAGS += -DHDSK_SUPPORT_HDLFS=1
IOP_OBJS += hdl.o
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

$(IOP_OBJS_DIR)%.o: $(LIBAPA_SRC_DIR)%.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@
