# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_CFLAGS += -mno-check-zero-division

IOP_INCS += \
	-I$(PS2SDKSRC)/iop/cdvd/cdvdman/include \
	-I$(PS2SDKSRC)/iop/system/dmacman/include \
	-I$(PS2SDKSRC)/iop/system/intrman/include \
	-I$(PS2SDKSRC)/iop/system/ioman/include \
	-I$(PS2SDKSRC)/iop/system/loadcore/include \
	-I$(PS2SDKSRC)/iop/system/stdio/include \
	-I$(PS2SDKSRC)/iop/system/sysclib/include \
	-I$(PS2SDKSRC)/iop/system/sysmem/include \
	-I$(PS2SDKSRC)/iop/system/threadman/include

IOP_OBJS = imports.o exports.o iLink.o iLink_PHY.o iLink_internal.o iLink_crom.o iLink_intr.o iLink_TxRx.o iLink_DMA.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
