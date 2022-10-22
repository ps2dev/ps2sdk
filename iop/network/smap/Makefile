# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# Build for use with a DHCP-enabled LWIP stack.
LWIP_DHCP ?= 1

# Build against the netman library.
SMAP_NETMAN ?= 1

# Build against the ps2ip library.
SMAP_PS2IP ?= 0

# Enable the modular interface.
SMAP_MODULAR ?= 0

# Poll instead of relying on interrupts when receiving multiple packets.
# This will avoid the IOP locking up due to the large amount of interrupts.
SMAP_RX_PACKETS_POLLING_MODE ?= 1

IOP_PREFER_GPOPT = 16384
IOP_CFLAGS += -mno-check-zero-division

ifneq (x$(LWIP_DHCP),x0)
IOP_CFLAGS += -DLWIP_DHCP=1
endif

ifneq (x$(SMAP_NETMAN),x0)
IOP_CFLAGS += -DBUILDING_SMAP_NETMAN=1
endif

ifneq (x$(SMAP_PS2IP),x0)
IOP_CFLAGS += -DBUILDING_SMAP_PS2IP=1
IOP_INCS += -I$(PS2SDKSRC)/iop/tcpip/tcpip/include
endif

ifneq (x$(SMAP_MODULAR),x0)
IOP_CFLAGS += -DBUILDING_SMAP_MODULAR=1
endif

ifneq (x$(SMAP_RX_PACKETS_POLLING_MODE),x0)
IOP_CFLAGS += -DSMAP_RX_PACKETS_POLLING_MODE=1
endif

IOP_INCS += -I$(PS2SDKSRC)/iop/dev9/dev9/include

IOP_OBJS = ipstack.o main.o smap.o xfer.o imports.o exports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
