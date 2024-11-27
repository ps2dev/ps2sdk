# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_BIN ?= iomanX.irx

IOP_CFLAGS += -DIOMANX_OLD_NAME_ADDDELDRV=0 -DIOMANX_OLD_NAME_COMPATIBILITY=0 -DIOMANX_ENABLE_LEGACY_IOMAN_HOOK -DIOMAN_USE_FILE_STRUCT_TEMP_STACK

IOP_OBJS = iomanX.o ioman_sbv.o exports.o imports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
