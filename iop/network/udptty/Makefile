# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# Paramaters for the UDP TTY driver.
UDPTTY_ETH_DST = 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
UDPTTY_IP_ADDR_DST = IP_ADDR(255, 255, 255, 255)
UDPTTY_IP_ADDR_SRC = IP_ADDR(192, 168, 0, 10)
UDPTTY_IP_PORT_DST = IP_PORT(0x4712)
UDPTTY_IP_PORT_SRC = IP_PORT(0x4712)

UDPTTY_PARAM = -DUDPTTY_ETH_DST='$(UDPTTY_ETH_DST)' \
	-DUDPTTY_IP_ADDR_DST='$(UDPTTY_IP_ADDR_DST)' -DUDPTTY_IP_ADDR_SRC='$(UDPTTY_IP_ADDR_SRC)' \
	-DUDPTTY_IP_PORT_DST='$(UDPTTY_IP_PORT_DST)' -DUDPTTY_IP_PORT_SRC='$(UDPTTY_IP_PORT_SRC)'
IOP_CFLAGS += $(UDPTTY_PARAM)

IOP_INCS += -I$(PS2SDKSRC)/iop/dev9/dev9/include

IOP_LDFLAGS = -s

IOP_OBJS = main.o smap.o udp.o tty.o imports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
