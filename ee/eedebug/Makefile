# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#

EE_OBJS_DIR = obj/
EE_LIB_DIR = lib/
EE_SRC_DIR = src/
EE_INC_DIR = include/

EE_LIB = $(EE_LIB_DIR)libeedebug.a

EE_CFLAGS += -O0

EE_OBJS = ee_debug.o ee_exceptions.o ee_dbg_low.o erl-support.o
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

all: $(EE_OBJS_DIR) $(EE_LIB_DIR) $(EE_LIB)

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_LIB_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release
