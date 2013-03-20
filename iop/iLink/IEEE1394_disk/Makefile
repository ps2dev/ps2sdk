# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$

IOP_OBJS_DIR = obj/
IOP_BIN_DIR = bin/
IOP_SRC_DIR = src/
IOP_INC_DIR = include/

IOP_BIN  = bin/IEEE1394_disk.irx
IOP_OBJS = obj/sbp2_driver.o obj/imports.o obj/main.o obj/scsi.o obj/fat_write.o obj/fs_driver.o obj/scache.o obj/fat_driver.o obj/part_driver.o

# Define "SIF_CALLBACKS_12_13" if you want the driver to send a SIF packet to the EE via SIF command channels 12 or 13 whenever a device has been disconnected or connected.
# SIF command channel 12 is used for connection events, while channel 13 is used for disconnection events.

IOP_INCS += -I$(PS2SDKSRC)/iop/kernel/include -I$(PS2SDKSRC)/common/include -I$(IOP_SRC_DIR)/include
IOP_CFLAGS += -Wall -fno-builtin -O2 -G0 -mno-check-zero-division #-DSIF_CALLBACKS_12_13
IOP_LDFLAGS += -s

all: $(IOP_OBJS_DIR) $(IOP_BIN_DIR) $(IOP_BIN)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_BIN_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
