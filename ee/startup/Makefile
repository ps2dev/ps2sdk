# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_LIB = libdummy.a

EE_OBJS = crt0.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

release: all
	@$(PRINTF) 'Installing %slinkfile into %s/ee/startup\n' $(EE_SRC_DIR) $(PS2SDK)
	cp -f $(EE_SRC_DIR)linkfile $(PS2SDK)/ee/startup
	@# Override the generated by newlib
	@$(PRINTF) 'Installing %scrt0.o into %s/ee/mips64r5900el-ps2-elf/lib overriding the dummy crt0 created by newlib\n' $(EE_OBJS_DIR) $(PS2DEV)
	cp -f $(EE_OBJS_DIR)crt0.o $(PS2DEV)/ee/mips64r5900el-ps2-elf/lib
