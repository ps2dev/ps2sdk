# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN = ioprpgen_sample.elf
EE_OBJS = ioprpgen_sample.o sifinit_irx.o threadman_irx.o igreeting_irx.o
EE_LIBS = -lioprpgen -liopreboot -lc -lpatches -ldebug

all: $(EE_BIN)
	$(EE_STRIP) --strip-all $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS) *_irx.c

%_irx.c: $(PS2SDK)/iop/irx/%.irx
	bin2c $< $@ $(basename $(notdir $@))

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
