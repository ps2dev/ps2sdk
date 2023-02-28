# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# tty2sior made by Matias Israelson (2023) based on UDPTTY.IRX and SIOR.IRX

IOP_OBJS = tty2sior.o xprintf.o exports.o imports.o
KPRTTY ?= 0

ifneq ($(KPRTTY), 0)
	IOP_CFLAGS += -DKPRTTY
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release