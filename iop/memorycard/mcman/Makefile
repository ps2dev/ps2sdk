# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#

IOP_OBJS_DIR = obj/
IOP_BIN_DIR = irx/
IOP_SRC_DIR = src/
IOP_INC_DIR = include/

IOP_BIN  = $(IOP_BIN_DIR)mcman.irx
IOP_OBJS = main.o mcdev.o mcsio2.o ps2mc_fio.o ps1mc_fio.o imports.o exports.o
IOP_OBJS := $(IOP_OBJS:%=$(IOP_OBJS_DIR)%)

IOP_INCS += -I$(PS2SDKSRC)/iop/system/sio2log/include
IOP_CFLAGS += #-DSIO_DEBUG #-DDEBUG

all: $(IOP_OBJS_DIR) $(IOP_BIN_DIR) $(IOP_BIN)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_BIN_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
