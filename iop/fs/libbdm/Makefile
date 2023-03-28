# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# IOP_CFLAGS += -DDEBUG -Wno-strict-aliasing

IOP_INCS += -I$(PS2SDKSRC)/iop/fs/bdm/include

IOP_OBJS = bd_defrag.o bd_cache.o
IOP_LIB = libbdm.a

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.lib.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
