# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$

.SUFFIXES:.c
VPATH = src:obj

LIBDEBUG_LIB  = lib/libdebug.a

EE_OBJS_DIR = obj/
EE_LIB_DIR = lib/
EE_SRC_DIR = src/
EE_INC_DIR = include/
EE_SAMPLE_DIR = samples/

EE_LIB = $(LIBDEBUG_LIB)

EE_OBJS = scr_printf.o  font.o screenshot.o callstack.o callstackget.o
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

all: $(EE_OBJS_DIR) $(EE_LIB_DIR) $(LIBDEBUG_LIB) 


.c.o :
	$(EE_CC) $(EE_CFLAGS) $(EE_INC) -c $< -o obj/$@

clean:
	rm -f -r $(EE_OBJS_DIR)/* $(EE_LIB_DIR)/*


include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release
