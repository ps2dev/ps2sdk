# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.


IOP_BIN  = bin/freesd.irx
IOP_OBJS = obj/freesd.o obj/batch.o obj/block.o obj/effect.o \
	 obj/voice.o obj/exports.o obj/imports.o

IOP_CFLAGS  += -Wall -fno-builtin-printf -fno-builtin-memcpy
IOP_LDFLAGS += -s

all: $(IOP_BIN)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make



