# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# DEBUG = 1
PS2IP_DNS = 1

IOP_BIN ?= ps2ips.irx

ifeq ($(DEBUG),1)
IOP_CFLAGS += -DDEBUG
endif

ifeq ($(PS2IP_DNS),1)
IOP_CFLAGS += -DPS2IP_DNS
endif

IOP_INCS += -I$(PS2SDKSRC)/iop/tcpip/tcpip/include

IOP_OBJS = ps2ips.o imports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
