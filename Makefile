# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.


SUBDIRS = tools ee iop common samples

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

$(PS2SDK)/common/include:
	mkdir $(PS2SDK)/common
	mkdir $(PS2SDK)/common/include
	cp -f $(PS2SDKSRC)/common/include/*.h $(PS2SDK)/common/include/

release: env_build_check release_base $(PS2SDK)/common/include $(subdir_release) 


release_base: env_release_check
	@if test ! -d $(PS2SDK) ; then \
	  mkdir -p $(PS2SDK) ; \
	fi
	cp -f README $(PS2SDK)
	cp -f AUTHORS $(PS2SDK)
	cp -f LICENSE $(PS2SDK)
	cp -f ID $(PS2SDK)

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
