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

# C compiler flags
# -fno-builtin is required to prevent the GCC built-in functions from being included,
#   for finer-grained control over what goes into each IRX.
IOP_CFLAGS := -D_IOP -fno-builtin -Os -G0 -Wall $(IOP_INCS) $(IOP_CFLAGS)
# Linker flags
IOP_LDFLAGS := -nostdlib -s $(IOP_LDFLAGS)

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

# Assembler flags
IOP_ASFLAGS := $(ASFLAGS_TARGET) -EL -G0 $(IOP_ASFLAGS)

IOP_OBJS := $(IOP_OBJS:%=$(IOP_OBJS_DIR)%)

# Externally defined variables: IOP_BIN, IOP_OBJS, IOP_LIB

# These macros can be used to simplify certain build rules.
IOP_C_COMPILE = $(IOP_CC) $(IOP_CFLAGS)

$(IOP_OBJS_DIR)%.o: $(IOP_SRC_DIR)%.c
	$(IOP_C_COMPILE) -c $< -o $@

$(IOP_OBJS_DIR)%.o: $(IOP_SRC_DIR)%.S
	$(IOP_C_COMPILE) -c $< -o $@

$(IOP_OBJS_DIR)%.o: $(IOP_SRC_DIR)%.s
	$(IOP_AS) $(IOP_ASFLAGS) $< -o $@

.INTERMEDIATE: $(IOP_OBJS_DIR)build-imports.c $(IOP_OBJS_DIR)build-exports.c

# Rules to build imports.lst.
$(IOP_OBJS_DIR)build-imports.c: $(IOP_SRC_DIR)imports.lst
	$(ECHO) "#include \"irx_imports.h\"" > $@
	cat $< >> $@

$(IOP_OBJS_DIR)imports.o: $(IOP_OBJS_DIR)build-imports.c
	$(IOP_C_COMPILE) $(IOP_IETABLE_CFLAGS) -c $< -o $@

# Rules to build exports.tab.
$(IOP_OBJS_DIR)build-exports.c: $(IOP_SRC_DIR)exports.tab
	$(ECHO) "#include \"irx.h\"" > $@
	cat $< >> $@

$(IOP_OBJS_DIR)exports.o: $(IOP_OBJS_DIR)build-exports.c
	$(IOP_C_COMPILE) $(IOP_IETABLE_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR):
	$(MKDIR) -p $(IOP_OBJS_DIR)

$(IOP_BIN_DIR):
	$(MKDIR) -p $(IOP_BIN_DIR)

$(IOP_LIB_DIR):
	$(MKDIR) -p $(IOP_LIB_DIR)

$(IOP_OBJS): | $(IOP_OBJS_DIR)

$(IOP_BIN): $(IOP_OBJS) | $(IOP_BIN_DIR)
	$(IOP_C_COMPILE) -o $(IOP_BIN) $(IOP_OBJS) $(IOP_LDFLAGS) $(IOP_LIBS)

$(IOP_LIB): $(IOP_OBJS) | $(IOP_LIB_DIR)
	$(IOP_AR) cru $(IOP_LIB) $(IOP_OBJS)
