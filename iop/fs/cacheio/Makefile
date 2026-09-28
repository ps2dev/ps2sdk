# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_BIN ?= cacheio.irx

IOP_CFLAGS += -DIOMANX_OLD_NAME_COMPATIBILITY=0 -DIOMANX_OLD_NAME_ADDDELDRV=0

IOP_IMPORT_INCS += \
	fs/bdm \
	fs/libbdm \
	system/intrman \
	system/iomanx \
	system/loadcore \
	system/sifcmd \
	system/sifman \
	system/stdio \
	system/sysclib \
	system/sysmem \
	system/threadman

IOP_OBJS = cacheio.o imports.o
IOP_LIB_ARCHIVES = $(PS2SDKSRC)/iop/fs/libbdm/lib/libbdm.a
IOP_LIBS += -lgcc

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

$(PS2SDKSRC)/iop/fs/libbdm/lib/libbdm.a:
	$(MAKEREC) $(PS2SDKSRC)/iop/fs/libbdm

$(IOP_BIN): $(PS2SDKSRC)/iop/fs/libbdm/lib/libbdm.a
