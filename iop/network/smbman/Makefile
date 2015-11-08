# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#

IOP_OBJS_DIR = obj/
IOP_BIN_DIR = irx/
IOP_SRC_DIR = src/

IOP_BIN  = $(IOP_BIN_DIR)smbman.irx
IOP_OBJS = smbman.o smb_fio.o smb.o poll.o auth.o \
	des.o md4.o imports.o exports.o
IOP_OBJS := $(IOP_OBJS:%=$(IOP_OBJS_DIR)%)

IOP_CFLAGS  += #-DDEBUG
IOP_LDFLAGS += -lgcc
IOP_INCS += -I$(PS2SDKSRC)/iop/tcpip/tcpip/include

all: $(IOP_OBJS_DIR) $(IOP_BIN_DIR) $(IOP_BIN)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_BIN_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
