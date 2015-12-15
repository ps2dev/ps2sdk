# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#

LIBPFS_PATH = ../libpfs

IOP_OBJS_DIR = obj/
IOP_BIN_DIR = irx/
IOP_SRC_DIR = src/
LIBPFS_SRC_DIR = $(LIBPFS_PATH)/src/
IOP_INC_DIR = include/

IOP_BIN  = $(IOP_BIN_DIR)ps2fs.irx
PFS_OBJS = bitmap.o dir.o inode.o journal.o misc.o super.o superWrite.o cache.o block.o blockWrite.o
IOP_OBJS = pfs.o pfs_fio.o pfs_fioctl.o imports.o $(PFS_OBJS)
IOP_OBJS := $(IOP_OBJS:%=$(IOP_OBJS_DIR)%)

IOP_CFLAGS  += -fno-builtin-printf
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
