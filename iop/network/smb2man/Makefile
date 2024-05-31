# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

DEBUG ?= 0

ifeq ($(DEBUG),1)
	IOP_CFLAGS += -DDEBUG
endif

IOP_INCS += -I. -I$(PS2SDK)/iop/include -I$(PS2SDKSRC)/iop/tcpip/tcpip/include 

IOP_LDFLAGS += -L$(PS2SDK)/iop/lib -lsmb2 -lgcc

IOP_OBJS = smb2man.o smb2_fio.o imports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
