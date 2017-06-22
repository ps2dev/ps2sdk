# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN = poweroff_example.elf
EE_OBJS = poweroff.o
EE_LIBS = -lpoweroff

all: $(EE_BIN) poweroff.irx

clean:
	rm -f $(EE_BIN) $(EE_OBJS) poweroff.irx

poweroff.irx: $(PS2SDK)/iop/irx/poweroff.irx
	cp $< $@

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
