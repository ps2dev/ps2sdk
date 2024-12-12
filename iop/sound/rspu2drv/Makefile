# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_INCS += \
	-I$(PS2SDKSRC)/iop/sound/libspu2/include \
	-I$(PS2SDKSRC)/iop/system/intrman/include \
	-I$(PS2SDKSRC)/iop/system/loadcore/include \
	-I$(PS2SDKSRC)/iop/system/sifcmd/include \
	-I$(PS2SDKSRC)/iop/system/sifman/include \
	-I$(PS2SDKSRC)/iop/system/stdio/include \
	-I$(PS2SDKSRC)/iop/system/sysclib/include \
	-I$(PS2SDKSRC)/iop/system/sysmem/include \
	-I$(PS2SDKSRC)/iop/system/threadman/include \
	-I$(PS2SDKSRC)/iop/system/vblank/include

IOP_OBJS = \
	imports.o \
	rsd_cb.o \
	rsd_com.o \
	rsd_main.o

IOP_LIB_ARCHIVES = $(PS2SDKSRC)/iop/sound/libsnd2/lib/libsnd2.a $(PS2SDKSRC)/iop/sound/libspu2/lib/libspu2.a
IOP_CFLAGS += -I$(PS2SDKSRC)/iop/sound/libspu2/include/ -I$(PS2SDKSRC)/iop/sound/libsnd2/include/


include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

$(PS2SDKSRC)/iop/sound/libspu2/lib/libspu2.a:
	$(MAKEREC) $(PS2SDKSRC)/iop/sound/libspu2

$(PS2SDKSRC)/iop/sound/libsnd2/lib/libsnd2.a:
	$(MAKEREC) $(PS2SDKSRC)/iop/sound/libsnd2

.NOTPARALLEL:: \
	$(PS2SDKSRC)/iop/sound/libspu2/lib/libspu2.a \
	$(PS2SDKSRC)/iop/sound/libsnd2/lib/libsnd2.a
