# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_OBJS_DIR = obj/
IOP_BIN_DIR = bin/
IOP_SRC_DIR = src/iop/

IOP_BIN  = ahx.irx
IOP_OBJS = obj/hw.o obj/ahx_irx.o obj/spu2.o obj/imports.o obj/ahx.o

IOP_CFLAGS  += -Wall -fno-builtin-printf -I$(PS2SDKSRC)/iop/sound/freesd/include/
IOP_LDFLAGS += -s

all: clean $(IOP_OBJS_DIR) $(IOP_BIN_DIR) $(IOP_BIN)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_BIN_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
