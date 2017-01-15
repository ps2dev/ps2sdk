# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_ERL = hello.erl
EE_OBJS = hello.o

all: $(EE_ERL) erl-loader.elf libc.erl libkernel.erl

clean:
	rm -f $(EE_ERL) $(EE_OBJS) erl-loader.elf libc.erl libkernel.erl

erl-loader.elf:
	cp $(PS2SDK)/ee/bin/erl-loader.elf $@

libc.erl:
	cp $(PS2SDK)/ee/lib/libc.erl $@

libkernel.erl:
	cp $(PS2SDK)/ee/lib/libkernel.erl $@

run: $(EE_ERL)
	ps2client execee host:erl-loader.elf $(EE_ERL)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
