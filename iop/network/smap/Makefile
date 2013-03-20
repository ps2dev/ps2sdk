#  _____     ___ ____
#   ____|   |    ____|      PSX2 OpenSource Project
#  |     ___|   |____       (C)2002, David Ryan ( Oobles@hotmail.com )
#                           (C)2003, adresd     ( adresd_ps2dev@yahoo.com )
#  ------------------------------------------------------------------------


IOP_BIN = smap.irx
IOP_OBJS = main.o smap.o xfer.o imports.o exports.o

IOP_INCS += -I$(PS2SDKSRC)/iop/kernel/include -I$(PS2SDKSRC)/iop/dev9/dev9/include -I$(PS2SDKSRC)/common/include
IOP_CFLAGS += -Wall -fno-builtin -D_IOP -mgpopt -G19
IOP_LDFLAGS += -s

all: $(IOP_BIN)

clean:
	rm -f $(IOP_OBJS) $(IOP_BIN)

include $(PS2SDKSRC)/Defs.make
include Rules.make
include $(PS2SDKSRC)/iop/Rules.release
