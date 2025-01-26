# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_LIB = libdummy.a

IOP_INCS += \
	-I$(PS2SDKSRC)/iop/system/stdio/include

IOP_OBJS = crt0.o
# disable array bounds warning for __do_global_ctors
IOP_WARNFLAGS += -Wno-array-bounds

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.lib.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

release:
	@$(PRINTF) 'Installing %slinkfile into %s/iop/startup\n' $(IOP_SRC_DIR) $(PS2SDK)
	$(ECHO) Installing $(IOP_SRC_DIR)linkfile into $(PS2SDK)/iop/startup
	cp -f $(IOP_SRC_DIR)linkfile $(PS2SDK)/iop/startup
	@$(ECHO) Installing $(IOP_OBJS_DIR)crt0.o into $(PS2DEV)/iop/mipsel-none-elf/lib
	cp -f $(IOP_OBJS_DIR)crt0.o $(PS2DEV)/iop/mipsel-none-elf/lib
