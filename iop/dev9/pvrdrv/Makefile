
IOP_BIN ?= dvrdrv.irx

IOP_INCS += -I$(PS2SDKSRC)/iop/dev9/dev9/include
IOP_OBJS = dvrdrv.o exports.o imports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
