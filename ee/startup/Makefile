# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$


EE_OBJS_DIR = obj/
EE_SRC_DIR = src/

all:  $(EE_OBJS_DIR) obj/crt0.o

clean:
	rm -f -r $(EE_OBJS_DIR)

release:
	cp -f src/linkfile $(PS2SDK)/ee/startup
	cp -f obj/crt0.o $(PS2SDK)/ee/startup

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make

