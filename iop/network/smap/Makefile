# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# Build for use with a DHCP-enabled LWIP stack.
LWIP_DHCP = 1

IOP_CFLAGS += -mgpopt -G16384 -mno-check-zero-division

ifdef LWIP_DHCP
IOP_CFLAGS += -DLWIP_DHCP=1
endif

IOP_INCS = -I$(PS2SDKSRC)/iop/dev9/dev9/include

IOP_OBJS = main.o smap.o xfer.o imports.o exports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
