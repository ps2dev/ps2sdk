# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$


SAMPLES_TARGET = $(PS2SDK)/samples

all:
	@$(ECHO) Nothing to do in samples

clean:
	@$(ECHO) Nothing to clean in samples

release: release-samples
	@$(ECHO) Released samples dir.

release-samples:
	$(MKDIR) -p $(SAMPLES_TARGET)
	cp -f Makefile_sample $(SAMPLES_TARGET)/Makefile
	cp -f Makefile.pref_sample $(SAMPLES_TARGET)/Makefile.pref
	cp -f Makefile.eeglobal_sample $(SAMPLES_TARGET)/Makefile.eeglobal
	cp -f Makefile.eeglobal_cpp_sample $(SAMPLES_TARGET)/Makefile.eeglobal_cpp
	cp -f Makefile.iopglobal_sample $(SAMPLES_TARGET)/Makefile.iopglobal


include $(PS2SDKSRC)/Defs.make

