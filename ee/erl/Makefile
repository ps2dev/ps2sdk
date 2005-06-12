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

EE_OBJS_DIR = obj/
EE_LIB_DIR = lib/
EE_SRC_DIR = src/
EE_INC_DIR = include/
EE_SAMPLE_DIR = samples/

EE_LIB = $(EE_LIB_DIR)liberl.a
EE_SAMPLES = hello

EE_SAMPLES := $(EE_SAMPLES:%=$(EE_SAMPLE_DIR)%)

EE_OBJS = erl.o hashtab.o lookupa.o recycle.o
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

EE_CFLAGS = -I $(EE_INC_DIR) -I $(EE_SRC_DIR)

all: $(EE_OBJS_DIR) $(EE_LIB_DIR) $(EE_LIB) 

.c.o :
	$(EE_CC) $(EE_CFLAGS) $(EE_INC) -c $< -o obj/$@

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_LIB_DIR)


include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release
