# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2022, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

TOOLS_BIN_ALTNAMES ?= eefixup erx-strip iopfixup irx-strip

TOOLS_OBJS = \
    srxfixup.o \
    anaarg.o \
    ring.o \
    swapmem.o \
    elflib.o \
    elfdump.o \
    mipsdis.o \
    srxgen.o \
    readconf.o \
    iopfixconf.o \
    eefixconf.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/tools/Rules.bin.make
include $(PS2SDKSRC)/tools/Rules.make
include $(PS2SDKSRC)/tools/Rules.release
