# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN = vu1.elf
EE_OBJS = draw_3D.o main.o
EE_LIBS = -ldraw -lgraph -lmath3d -lpacket2 -ldma
EE_DVP = dvp-as
#EE_VCL = vcl

all: zbyszek.c $(EE_BIN)
	$(EE_STRIP) --strip-all $(EE_BIN)

# Original VCL tool preferred. 
# It can be runned on WSL, but with some tricky commands: 
# https://github.com/microsoft/wsl/issues/2468#issuecomment-374904520
#%.vsm: %.vcl
#	$(EE_VCL) $< >> $@

%.o: %.vsm
	$(EE_DVP) $< -o $@

zbyszek.c:
	bin2c zbyszek.raw zbyszek.c zbyszek

clean:
	rm -f $(EE_BIN) $(EE_OBJS) zbyszek.c

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
