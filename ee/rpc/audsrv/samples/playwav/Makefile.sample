# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: Makefile 1187 2005-06-27 17:34:43Z lukasz $

EE_BIN = playwav.elf
EE_OBJS = playwav.o
EE_LIBS = -laudsrv -lc

all: $(EE_BIN)

clean:
	rm -f *.elf *.o

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
