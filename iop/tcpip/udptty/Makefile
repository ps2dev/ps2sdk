# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.


IOP_BIN = bin/udptty.irx
IOP_OBJS = obj/main.o obj/smap.o obj/udp.o obj/tty.o obj/imports.o
IOP_INCS = -I. -I$(PS2SDKSRC)/iop/dev9/dev9/include

IOP_CFLAGS = -Wall
IOP_LDFLAGS = -s

# Paramaters for the UDP TTY driver.
UDPTTY_ETH_DST = 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
UDPTTY_ETH_SRC = 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
UDPTTY_IP_ADDR_DST = IP_ADDR(255, 255, 255, 255)
UDPTTY_IP_ADDR_SRC = IP_ADDR(192, 168, 1, 23)
UDPTTY_IP_PORT_DST = IP_PORT(0x4712)
UDPTTY_IP_PORT_SRC = IP_PORT(0x4712)

UDPTTY_PARAM = -DUDPTTY_ETH_DST='$(UDPTTY_ETH_DST)' -DUDPTTY_ETH_SRC='$(UDPTTY_ETH_SRC)' \
	-DUDPTTY_IP_ADDR_DST='$(UDPTTY_IP_ADDR_DST)' -DUDPTTY_IP_ADDR_SRC='$(UDPTTY_IP_ADDR_SRC)' \
	-DUDPTTY_IP_PORT_DST='$(UDPTTY_IP_PORT_DST)' -DUDPTTY_IP_PORT_SRC='$(UDPTTY_IP_PORT_SRC)'

IOP_CFLAGS += $(UDPTTY_PARAM)

all: $(IOP_BIN)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make

