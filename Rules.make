# Common rules shared by all build targets.

.PHONY: dummy

# Use SUBDIRS to descend into subdirectories.
subdir_list  = $(patsubst %,all-%,$(SUBDIRS))
subdir_clean = $(patsubst %,clean-%,$(SUBDIRS))
subdirs: dummy $(subdir_list)

ifdef SUBDIRS
$(subdir_list): dummy
	$(MAKE) -C $(patsubst all-%,%,$@)
$(subdir_clean): dummy
	$(MAKE) -C $(patsubst clean-%,%,$@) clean
endif

# Default rule for clean.
clean: $(subdir_clean)

# A rule to do nothing.
dummy:
