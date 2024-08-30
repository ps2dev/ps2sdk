# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_INCS += \
	-I$(PS2SDKSRC)/iop/arcade/accore/include \
	-I$(PS2SDKSRC)/iop/arcade/acjv/include \
	-I$(PS2SDKSRC)/iop/arcade/acmem/include \
	-I$(PS2SDKSRC)/iop/arcade/acram/include \
	-I$(PS2SDKSRC)/iop/arcade/acsram/include

IOP_OBJS = \
	acmeme-entry.o \
	meme.o \
	imports.o \
	exports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
