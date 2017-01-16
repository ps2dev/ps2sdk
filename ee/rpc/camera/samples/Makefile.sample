# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN = camera-test.elf
EE_OBJS = camera-test.o
EE_LIBS = -lps2cam -lc

all: $(EE_BIN) usbd.irx ps2cam.irx

clean:
	rm -f $(EE_BIN) $(EE_OBJS) usbd.irx ps2cam.irx

usbd.irx:
	cp $(PS2SDK)/iop/irx/usbd.irx $@

ps2cam.irx:
	cp $(PS2SDK)/iop/irx/ps2cam.irx $@

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
