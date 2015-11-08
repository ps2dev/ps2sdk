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

LIBEEDEBUG_LIB = $(EE_LIB_DIR)libeedebug.a

EE_INCS =

EE_OBJS  = ee_debug.o ee_exceptions.o ee_dbg_low.o
EE_OBJS += erl-support.o
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

EE_CFLAGS = -O0

EE_LIB = $(LIBEEDEBUG_LIB)

all: $(EE_OBJS_DIR) $(EE_LIB_DIR) $(EE_LIB)

.c.o :
	$(EE_CC) $(EE_CFLAGS) $(EE_INC) -c $< -o obj/$@

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_LIB_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release
