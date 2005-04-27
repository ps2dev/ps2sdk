# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$

ifeq (x$(PS2SDKSRC), x)
PS2SDKSRC=`pwd`
endif

SUBDIRS = tools iop ee common samples

all: build 
	@$(ECHO) .;
	@$(ECHO) .PS2SDK Built.;
	@$(ECHO) .;

# Common rules shared by all build targets.

.PHONY: dummy

# Use SUBDIRS to descend into subdirectories.
subdir_list  = $(patsubst %,all-%,$(SUBDIRS))
subdir_clean = $(patsubst %,clean-%,$(SUBDIRS))
subdir_release = $(patsubst %,release-%,$(SUBDIRS))
subdirs: dummy $(subdir_list)

SYSTEM := $(shell uname)
ifeq ($(SYSTEM),CYGWIN_NT-5.1)
	MAKEREC = $(GNUMAKE) -C 
else
	MAKEREC = PS2SDKSRC=$(PS2SDKSRC) $(GNUMAKE) -C 
endif

$(subdir_list): dummy
	$(MAKEREC) $(patsubst all-%,%,$@)
$(subdir_clean): dummy
	$(MAKEREC) $(patsubst clean-%,%,$@) clean
$(subdir_release): dummy
	$(MAKEREC) $(patsubst release-%,%,$@) release


build: env_build_check $(subdir_list)

clean: env_build_check $(subdir_clean)

rebuild: env_build_check $(subdir_clean) $(subdir_list)

$(PS2SDK)/common/include:
	$(MKDIR) -p $(PS2SDK)/common
	$(MKDIR) -p $(PS2SDK)/common/include
	cp -f $(PS2SDKSRC)/common/include/*.h $(PS2SDK)/common/include/

$(PS2SDK)/ports:
	$(MKDIR) -p $(PS2SDK)/ports

install: release

release: build release_base $(PS2SDK)/common/include $(PS2SDK)/ports $(subdir_release) 


release_base: env_release_check
	@if test ! -d $(PS2SDK) ; then \
	  $(MKDIR) -p $(PS2SDK) ; \
	fi
	cp -f README $(PS2SDK)
	cp -f CHANGELOG $(PS2SDK)
	cp -f AUTHORS $(PS2SDK)
	cp -f LICENSE $(PS2SDK)
	cp -f ID $(PS2SDK)
	cp -f Defs.make $(PS2SDK)

env_build_check: 
	@if test -z $(PS2SDKSRC) ; \
	then \
	  $(ECHO) PS2SDKSRC environment variable should be defined. ; \
	fi

env_release_check:
	@if test -z $(PS2SDK) ; \
	then \
	  $(ECHO) PS2SDK environment variable must be defined. ; \
	  exit 1; \
	fi

docs:
	doxygen doxy.conf

include Defs.make
