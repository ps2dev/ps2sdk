IOP_BIN  = bin/ps2dev9.irx
IOP_OBJS = obj/ps2dev9.o obj/exports.o obj/imports.o

#IOP_INCS    += -I../ps2smap
IOP_CFLAGS  += -Wall -fno-builtin-printf
IOP_LDFLAGS += -s

all: $(IOP_BIN)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
