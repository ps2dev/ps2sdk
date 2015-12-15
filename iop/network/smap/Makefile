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

# Uncomment the line below to build for use with a DHCP-enabled LWIP stack.
LWIP_DHCP=1

IOP_BIN  = $(IOP_BIN_DIR)smap.irx
IOP_OBJS = main.o smap.o xfer.o imports.o exports.o
IOP_OBJS := $(IOP_OBJS:%=$(IOP_OBJS_DIR)%)

IOP_INCS += -I$(PS2SDKSRC)/iop/kernel/include -I$(PS2SDKSRC)/common/include -I$(PS2SDKSRC)/iop/dev9/dev9/include -I$(IOP_SRC_DIR)/include
IOP_CFLAGS += -mgpopt -G19 -mno-check-zero-division

ifdef LWIP_DHCP
IOP_CFLAGS += -DLWIP_DHCP=1
endif

all: $(IOP_OBJS_DIR) $(IOP_BIN_DIR) $(IOP_BIN)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_BIN_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
