# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

include $(PS2SDKSRC)/Defs.make

IOP_CC_VERSION := $(shell $(IOP_CC) --version 2>&1 | sed -n 's/^.*(GCC) //p')

IOP_OBJS_DIR = obj/
IOP_BIN_DIR = bin/
IOP_SRC_DIR = src/
IOP_INC_DIR = include/

IOP_CFLAGS  += -Wall -fno-builtin-printf -fno-builtin-memcpy -std=c99
IOP_LDFLAGS += -s

ifeq ($(IOP_CC_VERSION),3.2.2)
IOP_BIN  = bin/ps2snd.irx
IOP_OBJS2 = ps2snd.o adpcm-stream.o imports.o
IOP_OBJS  = $(IOP_OBJS2:%=$(IOP_OBJS_DIR)%)
endif

all: $(IOP_OBJS_DIR) $(IOP_BIN_DIR) $(IOP_BIN)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_BIN_DIR)

include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release


