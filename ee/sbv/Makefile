# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_OBJS_DIR = obj/
EE_LIB_DIR = lib/
EE_SRC_DIR = src/
EE_INC_DIR = include/

EE_LIB = $(EE_LIB_DIR)libpatches.a

EE_OBJS = smem.o smod.o slib.o patch_enable_lmb.o patch_disable_prefix_check.o \
	patch_user_mem_clear.o patch_fileio.o \
	common.o erl-support.o
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

all: $(EE_OBJS_DIR) $(EE_LIB_DIR) $(EE_LIB)

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_LIB_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release
