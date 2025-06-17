# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_BIN_ALTNAMES ?= freemtap.irx

IOP_IMPORT_INCS += \
	sio/sio2man \
	system/loadcore \
	system/sifcmd \
	system/sifman \
	system/stdio \
	system/threadman

IOP_OBJS = freemtap.o rpcservers.o exports.o imports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
