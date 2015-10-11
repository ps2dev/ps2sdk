# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$

LIBPFS_PATH = ../libpfs

IOP_OBJS_DIR = obj/
IOP_BIN_DIR = bin/
IOP_SRC_DIR = src/
LIBPFS_SRC_DIR = $(LIBPFS_PATH)/src/
IOP_INC_DIR = include/

IOP_BIN  = bin/ps2fs.irx
PFS_OBJS = obj/bitmap.o obj/dir.o obj/inode.o obj/journal.o obj/misc.o obj/super.o obj/superWrite.o obj/cache.o obj/block.o obj/blockWrite.o
IOP_OBJS = obj/pfs.o obj/pfs_fio.o obj/pfs_fioctl.o obj/imports.o $(PFS_OBJS)

IOP_CFLAGS  += -Wall -fno-builtin-printf
IOP_LDFLAGS += -s
IOP_INCS += -I$(CURDIR)/$(IOP_SRC_DIR) -I$(PS2SDKSRC)/iop/dev9/atad/include -I$(PS2SDKSRC)/iop/dev9/poweroff/include -I$(LIBPFS_PATH)/include
IOP_LIBS += -lgcc

all: $(IOP_OBJS_DIR) $(IOP_BIN_DIR) $(IOP_BIN)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_BIN_DIR)

$(IOP_OBJS_DIR)%.o : $(LIBPFS_SRC_DIR)%.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
