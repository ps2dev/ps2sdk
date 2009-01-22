# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: Makefile.sample 1187 2005-06-27 17:34:43Z lukasz $

EE_BIN = playadpcm.elf

EE_OBJS = playadpcm.o
EE_LIBS = -laudsrv -lc

all: $(EE_BIN)  evillaugh.adp

%.adp : %.wav
	$(PS2SDK)/bin/adpenc $< $@

clean:
	rm -f *.elf *.o *.adp

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
