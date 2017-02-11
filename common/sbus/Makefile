# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_OBJS_DIR = ee_obj/

EE_INCS += -I$(PS2SDKSRC)/ee/rpc/sior/include/
EE_CFLAGS += -fno-builtin-printf

EE_OBJS = ps2_sbus.o ee_sbus.o ps2_sif.o ee_sif2.o sif2cmd.o ee_low.o

IOP_OBJS_DIR = iop_obj/

IOP_OBJS = ps2_sbus.o iop_sbus.o ps2_sif.o iop_sif2.o sif2cmd.o imports.o exports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
