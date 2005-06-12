# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#

EE_BIN = teapot.elf
EE_OBJS = teapot.o
EE_LIBS = -ldraw -lgraph -lmath3d -lmf -lpacket -ldma

all: $(EE_BIN)
	ee-strip --strip-all $(EE_BIN)

clean:
	rm -f *.elf *.o *.a

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
