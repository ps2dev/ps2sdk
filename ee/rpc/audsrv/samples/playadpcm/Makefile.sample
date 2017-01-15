# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN = playadpcm.elf
EE_OBJS = playadpcm.o
EE_LIBS = -laudsrv -lc

all: $(EE_BIN) evillaugh.adp audsrv.irx

%.adp : %.wav
	$(PS2SDK)/bin/adpenc $< $@

audsrv.irx:
	cp $(PS2SDK)/iop/irx/audsrv.irx $@

clean:
	rm -f $(EE_BIN) $(EE_OBJS) *.adp audsrv.irx

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
