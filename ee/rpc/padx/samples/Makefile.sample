# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: Makefile.sample 577 2004-09-14 14:41:46Z pixel $


EE_BIN = padx_sample.elf
EE_OBJS = padx.o
EE_LIBS = -lpadx

all: $(EE_BIN) freesio2.irx freepad.irx

clean:
	rm -f *.elf *.o *.a freepad.irx freesio2.irx

freesio2.irx: 
	cp $(PS2SDK)/iop/irx/freesio2.irx $@

freepad.irx: 
	cp $(PS2SDK)/iop/irx/freepad.irx $@


include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal

