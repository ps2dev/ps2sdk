# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.


EE_LIB = lib/libfileXio.a
EE_OBJS = obj/fileXio_rpc.o

#EE_CFLAGS += -I$(PS2DRV)/ee/include -I$(PS2DRV)/common/include
EE_LDFLAGS += -s

all: $(EE_LIB)

clean:
	rm -f $(EE_LIB) $(EE_OBJS)

makedist:
	rm -f $(EE_OBJS)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
