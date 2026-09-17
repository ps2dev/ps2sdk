# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# Support the UEPCB board?
AN986_UEPCB ?= 0

IOP_IMPORT_INCS += \
	network/netman \
	system/intrman \
	system/loadcore \
	system/stdio \
	system/sysclib \
	system/threadman \
	usb/usbd

IOP_OBJS = an986.o imports.o exports.o

ifneq (x$(AN986_UEPCB),x0)
IOP_CFLAGS += -DAN986_UEPCB=1
endif

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
