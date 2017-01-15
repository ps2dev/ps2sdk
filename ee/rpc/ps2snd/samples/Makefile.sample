# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN = ps2snd-sample.elf
EE_OBJS = stream.o
EE_LIBS = -lps2snd -lc

all: $(EE_BIN) ps2snd.irx

ps2snd.irx:
	cp $(PS2SDK)/iop/irx/ps2snd.irx $@

clean:
	rm -f $(EE_BIN) $(EE_OBJS) ps2snd.irx

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
