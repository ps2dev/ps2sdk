# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#

IOP_OBJS_DIR = obj/
IOP_BIN_DIR = irx/
IOP_SRC_DIR = src/
IOP_INC_DIR = include/

#DEBUG=1
PS2IP_DNS=1

IOP_BIN = $(IOP_BIN_DIR)ps2ips.irx
IOP_OBJS = ps2ips.o imports.o
IOP_OBJS := $(IOP_OBJS:%=$(IOP_OBJS_DIR)%)

ifeq ($(DEBUG),1)
DEBUG_FLAGS = -DDEBUG
endif

ifeq ($(PS2IP_DNS),1)
IOP_CFLAGS += -DPS2IP_DNS
endif

IOP_CFLAGS += $(DEBUG_FLAGS)
IOP_INCS += -I$(PS2SDKSRC)/iop/tcpip/tcpip/include

all: $(IOP_OBJS_DIR) $(IOP_BIN_DIR) $(IOP_BIN)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_BIN_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
