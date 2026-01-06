# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_BIN ?= s147ctrl.irx

IOP_IMPORT_INCS += \
	arcade/acdev \
	system/intrman \
	system/ioman \
	system/sifcmd \
	system/sifman \
	system/sysclib \
	system/sysmem \
	system/threadman \
	system/timrman \
	system/loadcore

IOP_OBJS = s147ctrl.o imports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
