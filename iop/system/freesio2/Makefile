# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_OBJS_DIR = obj/
IOP_BIN_DIR = bin/
IOP_SRC_DIR = $(PS2SDKSRC)/iop/system/sio2log/src/


IOP_BIN  = bin/freesio2.irx
IOP_OBJS = obj/sio2man.o

IOP_CFLAGS  += -Wall -fno-builtin-printf -fno-builtin-memcpy -DXSIO2MAN
IOP_LDFLAGS += -s
IOP_INCS += -I$(PS2SDKSRC)/iop/system/sio2log/include/

all: $(IOP_OBJS_DIR) $(IOP_BIN_DIR) $(IOP_BIN)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_BIN_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
