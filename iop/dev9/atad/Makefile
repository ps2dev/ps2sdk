# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.


IOP_BIN  = bin/ps2atad.irx
IOP_OBJS = obj/ps2atad.o obj/exports.o obj/imports.o

IOP_CFLAGS  += -Wall -fno-builtin
IOP_LDFLAGS += -s
IOP_INCS += -I$(PS2SDKSRC)/iop/dev9/dev9/include 

all: $(IOP_BIN)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
