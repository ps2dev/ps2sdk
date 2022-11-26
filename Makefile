# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

DEBUG ?= 0

ifeq (x$(PS2SDKSRC), x)
  export PS2SDKSRC=$(shell pwd)
endif

SUBDIRS = tools iop ee common samples

all: build
	@$(ECHO) .;
	@$(ECHO) .PS2SDK Built.;
	@$(ECHO) .;

# Common rules shared by all build targets.

.PHONY: dummy build clean_dependencies docs download_dependencies env_build_check env_release_check install rebuild release release_base subdirs

# Use SUBDIRS to descend into subdirectories.
subdir_list  = $(patsubst %,all-%,$(SUBDIRS))
subdir_clean = $(patsubst %,clean-%,$(SUBDIRS))
subdir_release = $(patsubst %,release-%,$(SUBDIRS))
subdirs: dummy $(subdir_list)

$(subdir_list): dummy
	+$(MAKEREC) $(patsubst all-%,%,$@)
$(subdir_clean): dummy
	+$(MAKEREC) $(patsubst clean-%,%,$@) clean
$(subdir_release): dummy
	$(MAKEREC) $(patsubst release-%,%,$@) release


build: $(subdir_list) | env_build_check download_dependencies

debug:
	$(MAKE) DEBUG=1 all

clean: $(subdir_clean) | env_build_check clean_dependencies

release-clean:
	+$(MAKE) -C common release-clean
	+$(MAKE) -C iop release-iop-clean
	+$(MAKE) -C ee release-ee-clean
	+$(MAKE) -C samples release-clean
	+$(MAKE) -C tools release-clean

rebuild: build | clean

$(PS2SDK)/common/include:
	$(MKDIR) -p $(PS2SDK)/common
	$(MKDIR) -p $(PS2SDK)/common/include
	cp -f $(PS2SDKSRC)/common/include/*.h $(PS2SDK)/common/include/

$(PS2SDK)/ports:
	$(MKDIR) -p $(PS2SDK)/ports

install: | release

release: | build
	$(MAKE) release_base
	$(MAKE) release-clean
	$(MAKE) $(PS2SDK)/common/include
	$(MAKE) $(PS2SDK)/ports
	$(MAKE) $(subdir_release)

release_base: | env_release_check
	@if test ! -d $(PS2SDK) ; then \
	  $(MKDIR) -p $(PS2SDK) ; \
	fi
	cp -f README.md $(PS2SDK)
	cp -f CHANGELOG $(PS2SDK)
	cp -f AUTHORS2004.md $(PS2SDK)
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

clean_dependencies:
	$(ECHO) Cleaning PS2SDK dependencies.
	$(ECHO) Cleaning lwip.
	rm -rf $(PS2SDKSRC)/common/external_deps/lwip

download_dependencies:
	./dowload_dependencies.sh

docs:
	doxygen

include Defs.make
