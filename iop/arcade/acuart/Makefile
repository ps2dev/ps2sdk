# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_INCS += \
	-I$(PS2SDKSRC)/iop/arcade/accore/include

ADD_TTY ?= 0

IOP_OBJS = \
	acuart-entry.o \
	uart.o \
	imports.o \
	exports.o

ifeq ($(ADD_TTY), 1)
    IOP_CFLAGS += -DTTY_DEVICE
	IOP_OBJS += tty.o
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
