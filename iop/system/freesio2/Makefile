# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#

IOP_OBJS_DIR = obj/
IOP_BIN_DIR = irx/
IOP_SRC_DIR = $(PS2SDKSRC)/iop/system/sio2log/src/

IOP_BIN  = $(IOP_BIN_DIR)freesio2.irx
IOP_OBJS = sio2man.o
IOP_OBJS := $(IOP_OBJS:%=$(IOP_OBJS_DIR)%)

IOP_CFLAGS += -fno-builtin-printf -fno-builtin-memcpy -DXSIO2MAN
IOP_INCS += -I$(PS2SDKSRC)/iop/system/sio2log/include/

all: $(IOP_OBJS_DIR) $(IOP_BIN_DIR) $(IOP_BIN)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_BIN_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
