# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.


EE_LIB = lib/libkbd.a
EE_OBJS = obj/libkbd.o
#EE_INCS = -I../../iop/include -I../include

all: $(EE_LIB)

clean:
	rm -f *.elf *.o *.a $(EE_LIB)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make

