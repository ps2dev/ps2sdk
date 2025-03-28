# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_CFLAGS += -DBUILDING_CDVD_MODULE

IOP_IMPORT_INCS += \
	cdvd/cdvdman \
	system/intrman \
	system/iomanx \
	system/loadcore \
	system/sifman \
	system/stdio \
	system/sysclib \
	system/sysmem \
	system/threadman

IOP_OBJS = cdvdstm.o imports.o exports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
