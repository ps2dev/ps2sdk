# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_OBJS_DIR = ee_obj/
EE_LIB_DIR = lib/
EE_SRC_DIR = src/
EE_INC_DIR = include/

EE_LIB = $(EE_LIB_DIR)libsbus.a

EE_CFLAGS += -fno-builtin-printf

EE_OBJS = ps2_sbus.o ee_sbus.o ps2_sif.o ee_sif2.o sif2cmd.o ee_low.o
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

IOP_OBJS_DIR = iop_obj/
IOP_BIN_DIR = irx/
IOP_SRC_DIR = src/
IOP_INC_DIR = include/

IOP_BIN = $(IOP_BIN_DIR)sbus.irx

IOP_INCS += -I$(IOP_SRC_DIR)

IOP_OBJS = ps2_sbus.o iop_sbus.o ps2_sif.o iop_sif2.o sif2cmd.o imports.o exports.o
IOP_OBJS := $(IOP_OBJS:%=$(IOP_OBJS_DIR)%)

all: $(EE_OBJS_DIR) $(EE_LIB_DIR) $(EE_LIB) $(IOP_OBJS_DIR) $(IOP_BIN_DIR) $(IOP_BIN)

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_LIB_DIR) $(IOP_OBJS_DIR) $(IOP_BIN_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
