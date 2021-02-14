# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN   = ps2tests.elf
EE_OBJS  = runner.o testsuite.o stdio_tests.o

# make EE_CC=gcc will compile a native version of the regression test.
# ./ps2tests.elf to compare output.
ifneq ($(EE_CC),gcc)
EE_LIBS = -lpatches
endif

all: $(EE_BIN) input

input:
	mkdir -p testfiles/
	echo "hello world" > testfiles/dummy


clean:
	rm -rf $(EE_BIN) $(EE_OBJS) dummydir testfiles

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
