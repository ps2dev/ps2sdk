# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$

BINS = playwav.elf playcdda.elf playwav2.elf testcd.elf playadpcm.elf

OBJS = playwav.o playcdda.o playwav2.o testcd.o playadpcm.o
EE_INCS = -I../include
EE_LIBS = -L../lib -lc -laudsrv

all: $(BINS)  evillaugh.adp

install:

%.elf : %.o $(PS2SDK)/ee/startup/crt0.o
	$(EE_CC) -mno-crt0 -T$(PS2SDK)/ee/startup/linkfile $(EE_LDFLAGS) \
		-o $@ $(PS2SDK)/ee/startup/crt0.o $< $(EE_LIBS)

%.adp : %.wav
	$(PS2SDK)/bin/adpenc $< $@

clean:
	rm -f $(BINS) $(OBJS)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
