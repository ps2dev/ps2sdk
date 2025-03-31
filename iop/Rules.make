# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_CC_VERSION := $(shell $(IOP_CC) -dumpversion)

IOP_OBJS_DIR ?= obj/
IOP_SRC_DIR ?= src/
IOP_INC_DIR ?= include/

ifeq ($(IOP_CC_VERSION),3.2.2)
ASFLAGS_TARGET = -march=r3000
endif

ifeq ($(IOP_CC_VERSION),3.2.3)
ASFLAGS_TARGET = -march=r3000
endif

# include dir
IOP_INCS := $(IOP_INCS) -I$(IOP_SRC_DIR) -I$(IOP_SRC_DIR)include -I$(IOP_INC_DIR) -I$(PS2SDKSRC)/iop/kernel/include -I$(PS2SDKSRC)/common/include

ifdef IOP_IMPORT_INCS
IOP_INCS += $(addprefix -I$(PS2SDKSRC)/iop/, $(addsuffix /include, $(IOP_IMPORT_INCS)))
endif

# Optimization compiler flags
IOP_OPTFLAGS ?= -Os

# Warning compiler flags
IOP_WARNFLAGS ?= -Wall -Werror

# Debug information flags
IOP_DBGINFOFLAGS ?= -gdwarf-2 -gz

# C compiler flags
# -fno-builtin is required to prevent the GCC built-in functions from being included,
#   for finer-grained control over what goes into each IRX.
IOP_CFLAGS := -D_IOP -fno-builtin -G0 $(IOP_OPTFLAGS) $(IOP_WARNFLAGS) $(IOP_DBGINFOFLAGS) $(IOP_INCS) $(IOP_CFLAGS)
ifeq ($(DEBUG),1)
IOP_CFLAGS += -DDEBUG
endif
# Linker flags
IOP_LDFLAGS := -nostdlib -dc -r $(IOP_LDFLAGS)

# Additional C compiler flags for GCC >=v5.3.0
# -msoft-float is to "remind" GCC/Binutils that the soft-float ABI is to be used. This is due to a bug, which
#   results in the ABI not being passed correctly to binutils and iop-as defaults to the hard-float ABI instead.
# -mno-explicit-relocs is required to work around the fact that GCC is now known to
#   output multiple LO relocs after one HI reloc (which the IOP kernel cannot deal with).
# -fno-toplevel-reorder (for IOP import and export tables only) disables toplevel reordering by GCC v4.2 and later.
#   Without it, the import and export tables can be broken apart by GCC's optimizations.
ifneq ($(IOP_CC_VERSION),3.2.2)
ifneq ($(IOP_CC_VERSION),3.2.3)
IOP_CFLAGS += -msoft-float -mno-explicit-relocs
IOP_IETABLE_CFLAGS := -fno-toplevel-reorder
endif
endif

# If gpopt is requested, use it if the GCC version is compatible
ifneq (x$(IOP_PREFER_GPOPT),x)
ifeq ($(IOP_CC_VERSION),3.2.2)
IOP_CFLAGS += -DUSE_GP_REGISTER=1 -mgpopt -G$(IOP_PREFER_GPOPT)
endif
ifeq ($(IOP_CC_VERSION),3.2.3)
IOP_CFLAGS += -DUSE_GP_REGISTER=1 -mgpopt -G$(IOP_PREFER_GPOPT)
endif
endif

# Assembler flags
IOP_ASFLAGS := $(ASFLAGS_TARGET) -EL -G0 $(IOP_ASFLAGS)

# Default link file
ifeq ($(IOP_LINKFILE),)
IOP_LINKFILE := $(PS2SDKSRC)/iop/startup/src/linkfile
endif

IOP_OBJS := $(IOP_OBJS:%=$(IOP_OBJS_DIR)%)

IOP_BIN_ELF := $(IOP_BIN:.irx=.notiopmod.elf)

IOP_BIN_STRIPPED_ELF := $(IOP_BIN:.irx=.notiopmod.stripped.elf)

# Externally defined variables: IOP_BIN, IOP_OBJS, IOP_LIB

# These macros can be used to simplify certain build rules.
IOP_C_COMPILE = $(IOP_CC) $(IOP_CFLAGS)

# Command for ensuring the output directory for the rule exists.
DIR_GUARD = @$(MKDIR) -p $(@D)

MAKE_CURPID := $(shell printf '%s' $$PPID)

$(IOP_OBJS_DIR)%.o: $(IOP_SRC_DIR)%.c
	$(DIR_GUARD)
	$(IOP_C_COMPILE) -c $< -o $@

$(IOP_OBJS_DIR)%.o: $(IOP_SRC_DIR)%.S
	$(DIR_GUARD)
	$(IOP_C_COMPILE) -c $< -o $@

$(IOP_OBJS_DIR)%.o: $(IOP_SRC_DIR)%.s
	$(DIR_GUARD)
	$(IOP_AS) $(IOP_ASFLAGS) $< -o $@

.INTERMEDIATE:: $(IOP_LIB)_tmp$(MAKE_CURPID) $(IOP_OBJS_DIR)build-imports.c $(IOP_OBJS_DIR)build-exports.c

$(PS2SDKSRC)/tools/srxfixup/bin/srxfixup: $(PS2SDKSRC)/tools/srxfixup
	$(MAKEREC) $<

$(IOP_OBJS_DIR)template-imports.h:
	$(DIR_GUARD)
	$(PRINTF) '%s\n' "#include \"irx_imports.h\"" > $@

# Rules to build imports.lst.
$(IOP_OBJS_DIR)build-imports.c: $(IOP_OBJS_DIR)template-imports.h $(IOP_SRC_DIR)imports.lst
	$(DIR_GUARD)
	cat $^ > $@

$(IOP_OBJS_DIR)imports.o: $(IOP_OBJS_DIR)build-imports.c
	$(DIR_GUARD)
	$(IOP_C_COMPILE) $(IOP_IETABLE_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)template-exports.h:
	$(DIR_GUARD)
	$(PRINTF) '%s\n' "#include \"irx.h\"" > $@

# Rules to build exports.tab.
$(IOP_OBJS_DIR)build-exports.c: $(IOP_OBJS_DIR)template-exports.h $(IOP_SRC_DIR)exports.tab
	$(DIR_GUARD)
	cat $^ > $@

$(IOP_OBJS_DIR)exports.o: $(IOP_OBJS_DIR)build-exports.c
	$(DIR_GUARD)
	$(IOP_C_COMPILE) $(IOP_IETABLE_CFLAGS) -c $< -o $@

$(IOP_BIN_ELF): $(IOP_OBJS) $(IOP_LIB_ARCHIVES) $(IOP_ADDITIONAL_DEPS)
	$(DIR_GUARD)
	$(IOP_C_COMPILE) -T$(IOP_LINKFILE) $(IOP_OPTFLAGS) -o $@ $(IOP_OBJS) $(IOP_LDFLAGS) $(IOP_LIB_ARCHIVES) $(IOP_LIBS)

$(IOP_BIN_STRIPPED_ELF): $(IOP_BIN_ELF)
	$(DIR_GUARD)
	$(IOP_STRIP) --strip-unneeded --remove-section=.pdr --remove-section=.comment --remove-section=.mdebug.abi32 --remove-section=.gnu.attributes -o $@ $<

$(IOP_BIN): $(IOP_BIN_STRIPPED_ELF) $(PS2SDKSRC)/tools/srxfixup/bin/srxfixup
	$(PS2SDKSRC)/tools/srxfixup/bin/srxfixup --irx1 -o $@ $<

$(IOP_LIB)_tmp$(MAKE_CURPID): $(IOP_OBJS)
	$(DIR_GUARD)
	$(IOP_AR) cru $@ $(IOP_OBJS)

$(IOP_LIB): $(IOP_LIB)_tmp$(MAKE_CURPID)
	$(DIR_GUARD)
	mv $< $@
