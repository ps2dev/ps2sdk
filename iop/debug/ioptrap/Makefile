# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# IOP_CFLAGS += -DTEST_TRAP
IOP_CFLAGS += -DIOPTRAP_PRINT_MODULE_NAME

IOP_IMPORT_INCS += \
	system/excepman \
	system/loadcore \
	system/sifman \
	system/stdio \
	system/sysclib \
	system/threadman

IOP_OBJS = ioptrap.o handler.o breakpoint.o exports.o imports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
