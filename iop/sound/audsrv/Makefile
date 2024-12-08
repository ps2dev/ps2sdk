# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.

IOP_INCS += \
	-I$(PS2SDKSRC)/iop/cdvd/cdvdman/include \
	-I$(PS2SDKSRC)/iop/sound/libsd/include \
	-I$(PS2SDKSRC)/iop/system/intrman/include \
	-I$(PS2SDKSRC)/iop/system/loadcore/include \
	-I$(PS2SDKSRC)/iop/system/sifcmd/include \
	-I$(PS2SDKSRC)/iop/system/stdio/include \
	-I$(PS2SDKSRC)/iop/system/sysclib/include \
	-I$(PS2SDKSRC)/iop/system/sysmem/include \
	-I$(PS2SDKSRC)/iop/system/threadman/include

IOP_OBJS = audsrv.o upsamplers.o hw.o rpc_server.o rpc_client.o common.o cdrom.o imports.o exports.o adpcm.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
