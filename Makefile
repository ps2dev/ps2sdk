# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

DEBUG ?= 0
ONLY_HOST_TOOLS ?= 0

ifeq (x$(PS2SDKSRC), x)
  export PS2SDKSRC=$(shell pwd)
endif

# If ONLY_HOST_TOOLS is set, only build the host tools.
ifeq ($(ONLY_HOST_TOOLS), 1)
  SUBDIRS = tools
else
  SUBDIRS = tools common iop ee samples
endif

all: build
	@$(PRINTF) '.\n.PS2SDK Built.\n.\n'

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

# Directory-level parallelism has been disabled due to issues with
# multiple Make instances running inside a directory at once
# and causing output file corruption
.NOTPARALLEL: build $(subdir_list) $(subdir_clean) $(subdir_release)

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

$(PS2SDK)/ports_iop:
	$(MKDIR) -p $(PS2SDK)/ports_iop

$(PS2SDK)/ports_irx:
	$(MKDIR) -p $(PS2SDK)/ports_iop/irx

install: | release

release: | build
	$(MAKE) release_base
	$(MAKE) release-clean
	$(MAKE) $(PS2SDK)/common/include
	$(MAKE) $(PS2SDK)/ports
	$(MAKE) $(PS2SDK)/ports_iop
	$(MAKE) $(PS2SDK)/ports_irx	
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
	  $(PRINTF) 'PS2SDKSRC environment variable should be defined.\n' ; \
	fi

env_release_check:
	@if test -z $(PS2SDK) ; \
	then \
	  $(PRINTF) 'PS2SDK environment variable must be defined.\n' ; \
	  exit 1; \
	fi

# Don't do anything if ONLY_HOST_TOOLS is set.
download_dependencies:
	@if test $(ONLY_HOST_TOOLS) -eq 0 ; \
	then \
	  $(MAKEREC) $(PS2SDKSRC)/common/external_deps all ; \
	fi
	
# Don't do anything if ONLY_HOST_TOOLS is set.
clean_dependencies:
	@if test $(ONLY_HOST_TOOLS) -eq 0 ; \
	then \
	  $(MAKEREC) $(PS2SDKSRC)/common/external_deps clean ; \
	fi

docs:
	doxygen

include Defs.make
