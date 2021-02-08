# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN = remote_sample.elf
EE_OBJS = remote.o SIO2MAN_irx.o RMMAN_irx.o
EE_LIBS = -ldebug -lrm -lpatches

all: $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS) SIO2MAN_irx.c RMMAN_irx.c

SIO2MAN_irx.c:
	bin2c $(PS2SDK)/iop/irx/rsio2man.irx SIO2MAN_irx.c SIO2MAN_irx

RMMAN_irx.c:
	bin2c $(PS2SDK)/iop/irx/rmman.irx RMMAN_irx.c RMMAN_irx

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
