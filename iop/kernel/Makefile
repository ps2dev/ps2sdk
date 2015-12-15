# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#

IOP_OBJS_DIR = obj/
IOP_LIB_DIR = lib/
IOP_SRC_DIR = src/
IOP_INC_DIR = include/

IOP_LIB  = $(IOP_LIB_DIR)libkernel.a

IOP_OBJS = cdvdman.o heaplib.o intrman.o ioman.o libsd.o loadcore.o modload.o secrman.o sifcmd.o sifman.o sio2man.o stdio.o sysclib.o sysmem.o thbase.o thevent.o thsemap.o timrman.o usbd.o vblank.o
IOP_OBJS := $(IOP_OBJS:%=$(IOP_OBJS_DIR)%)

all: $(IOP_OBJS_DIR) $(IOP_LIB_DIR) $(IOP_LIB)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_LIB_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
