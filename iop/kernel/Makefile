# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.


IOP_OBJS_DIR = obj/
IOP_LIB_DIR = lib/
IOP_SRC_DIR = src/
IOP_INC_DIR = include/

IOP_LIB  = lib/libkernel.a

IOP_OBJS = obj/cdvdman.o obj/heaplib.o obj/intrman.o obj/ioman.o obj/libsd.o obj/loadcore.o obj/modload.o obj/sifcmd.o obj/sifman.o obj/stdio.o obj/sysclib.o obj/sysmem.o obj/thbase.o obj/thevent.o obj/thsemap.o obj/usbd.o obj/vblank.o

IOP_CFLAGS  += -Wall -fno-builtin
IOP_LDFLAGS += -s

all: $(IOP_OBJS_DIR) $(IOP_LIB_DIR) $(IOP_LIB)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_LIB_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
