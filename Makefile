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
  export PS2SDKSRC=$(shell pwd)
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

$(subdir_list): dummy
	$(MAKEREC) $(patsubst all-%,%,$@)
$(subdir_clean): dummy
	$(MAKEREC) $(patsubst clean-%,%,$@) clean
$(subdir_release): dummy
	$(MAKEREC) $(patsubst release-%,%,$@) release


build: env_build_check $(subdir_list)

clean: env_build_check $(subdir_clean)

clean_base: env_release_check
	  rm -f $(PS2SDK)/README.md
	  rm -f $(PS2SDK)/CHANGELOG 
	  rm -f $(PS2SDK)/AUTHORS
	  rm -f $(PS2SDK)/LICENSE
	  rm -f $(PS2SDK)/ID
	  rm -f $(PS2SDK)/Defs.make

release-clean: env_release_check clean_base
	make -C common release-clean
	make -C iop release-clean
	make -C ee release-clean
	make -C samples release-clean
	make -C tools release-clean

rebuild: env_build_check $(subdir_clean) $(subdir_list)

$(PS2SDK)/ports:
	$(MKDIR) -p $(PS2SDK)/ports

install: release

release: build release_base release-clean $(PS2SDK)/ports $(subdir_release)
	@$(ECHO) .;
	@$(ECHO) .PS2SDK Installed.;
	@$(ECHO) .;

release_base: env_release_check
	@if test ! -d $(PS2SDK) ; then \
	  $(MKDIR) -p $(PS2SDK) ; \
	fi
	cp -f README.md $(PS2SDK)
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

install-libc: env_release_check
	$(MAKEREC) common release-dirs
	$(MAKEREC) common release-include
	$(MAKEREC) ee/libc release-dirs
	$(MAKEREC) ee/libc release-include

install-headers: env_release_check release_base release-clean install-libc
	@$(ECHO) .;
	@$(ECHO) PS2SDK headers installed.;
	@$(ECHO) .;

docs:
	doxygen doxy.conf

include Defs.make
