# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#

LIBAPA_PATH = ../libapa

IOP_OBJS_DIR = obj/
IOP_BIN_DIR = irx/
IOP_SRC_DIR = src/
LIBAPA_SRC_DIR = $(LIBAPA_PATH)/src/
IOP_INC_DIR = include/

IOP_BIN  = $(IOP_BIN_DIR)ps2hdd.irx
APA_OBJS = misc.o cache.o apa.o journal.o
IOP_OBJS = hdd_fio.o hdd.o imports.o $(APA_OBJS)
IOP_OBJS := $(IOP_OBJS:%=$(IOP_OBJS_DIR)%)

IOP_INCS += -I$(CURDIR)/$(IOP_SRC_DIR) -I$(PS2SDKSRC)/iop/dev9/poweroff/include/ -I$(PS2SDKSRC)/iop/dev9/dev9/include/ -I$(PS2SDKSRC)/iop/dev9/atad/include/ -I$(LIBAPA_PATH)/include
IOP_CFLAGS  += -fno-builtin-printf

all: $(IOP_OBJS_DIR) $(IOP_BIN_DIR) $(IOP_BIN)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_BIN_DIR)

$(IOP_OBJS_DIR)%.o : $(LIBAPA_SRC_DIR)%.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
