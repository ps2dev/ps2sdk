# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.


SUBDIRS = ee iop tools

all: build 
	@echo .;
	@echo .PS2SDK Built.;
	@echo .;

# Common rules shared by all build targets.

.PHONY: dummy

# Use SUBDIRS to descend into subdirectories.
subdir_list  = $(patsubst %,all-%,$(SUBDIRS))
subdir_clean = $(patsubst %,clean-%,$(SUBDIRS))
subdir_release = $(patsubst %,release-%,$(SUBDIRS))
subdirs: dummy $(subdir_list)

$(subdir_list): dummy
	$(MAKE) -C $(patsubst all-%,%,$@)
$(subdir_clean): dummy
	$(MAKE) -C $(patsubst clean-%,%,$@) clean
$(subdir_release): dummy
	$(MAKE) -C $(patsubst release-%,%,$@) release


build: env_build_check $(subdir_list)

clean: env_build_check $(subdir_clean)

release: env_release_check env_build_check $(subdir_release)


env_build_check: 
	@if test -z $(PS2SDKSRC) ; \
	then \
	  echo PS2SDKSRC environment variable must be defined. ; \
	  exit 1; \
	fi

env_release_check:
	@if test -z $(PS2SDK) ; \
	then \
	  echo PS2SDK environment variable must be defined. ; \
	  exit 1; \
	fi


include Defs.make
