# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.


IOP_BIN  = bin/ps2hdd.irx
IOP_OBJS = obj/misc.o obj/cache.o obj/apa.o obj/journal.o obj/hdd_fio.o obj/hdd.o

IOP_CFLAGS  += -Wall -fno-builtin-printf
IOP_LDFLAGS += -s

all: $(IOP_BIN)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make

