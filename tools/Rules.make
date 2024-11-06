# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

TOOLS_CC_VERSION := $(shell $(CC) -dumpversion)

TOOLS_OBJS_DIR ?= obj/
TOOLS_SRC_DIR ?= src/
TOOLS_INC_DIR ?= include/
TOOLS_SAMPLE_DIR ?= samples/

TOOLS_INCS := $(TOOLS_INCS) -I$(TOOLS_SRC_DIR) -I$(TOOLS_SRC_DIR)include -I$(TOOLS_INC_DIR)

# Optimization compiler flags
TOOLS_OPTFLAGS ?= -O2

# Warning compiler flags
TOOLS_WARNFLAGS ?= -Wall -Werror

# C compiler flags
TOOLS_CFLAGS := $(TOOLS_OPTFLAGS) $(TOOLS_WARNFLAGS) $(TOOLS_INCS) $(TOOLS_CFLAGS)

# C++ compiler flags
TOOLS_CXXFLAGS := $(TOOLS_OPTFLAGS) $(TOOLS_WARNFLAGS) $(TOOLS_INCS) $(TOOLS_CXXFLAGS)

# Linker flags
#TOOLS_LDFLAGS := $(TOOLS_LDFLAGS)

# Assembler flags
#TOOLS_ASFLAGS := $(TOOLS_ASFLAGS)

TOOLS_OBJS := $(TOOLS_OBJS:%=$(TOOLS_OBJS_DIR)%)

# Externally defined variables: TOOLS_BIN, TOOLS_OBJS, TOOLS_LIB

# These macros can be used to simplify certain build rules.
TOOLS_C_COMPILE = $(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_INCS)
TOOLS_CXX_COMPILE = $(TOOLS_CC) $(TOOLS_CXXFLAGS) $(TOOLS_INCS)

# Command for ensuring the output directory for the rule exists.
DIR_GUARD = @$(MKDIR) -p $(@D)

MAKE_CURPID := $(shell printf '%s' $$PPID)

$(TOOLS_OBJS_DIR)%.o : $(TOOLS_SRC_DIR)%.c
	$(DIR_GUARD)
	$(CC) $(TOOLS_CFLAGS) $(TOOLS_INCS) -c $< -o $@

$(TOOLS_OBJS_DIR)%.o : $(TOOLS_SRC_DIR)%.cpp
	$(DIR_GUARD)
	$(CXX) $(TOOLS_CXXFLAGS) $(TOOLS_INCS) -c $< -o $@

$(TOOLS_OBJS_DIR)%.o : $(TOOLS_SRC_DIR)%.S
	$(DIR_GUARD)
	$(CC) $(TOOLS_CFLAGS) $(TOOLS_INCS) -c $< -o $@

$(TOOLS_OBJS_DIR)%.o : $(TOOLS_SRC_DIR)%.s
	$(DIR_GUARD)
	$(AS) $(TOOLS_ASFLAGS) $< -o $@

.INTERMEDIATE:: $(TOOLS_BIN)_tmp$(MAKE_CURPID) $(TOOLS_LIB)_tmp$(MAKE_CURPID)

$(TOOLS_BIN)_tmp$(MAKE_CURPID) : $(TOOLS_OBJS) $(TOOLS_LIB_ARCHIVES) $(TOOLS_ADDITIONAL_DEPS)
	$(DIR_GUARD)
	$(CC) $(TOOLS_LDFLAGS) -o $@ $(TOOLS_OBJS) $(TOOLS_LIB_ARCHIVES) $(TOOLS_LIBS)

$(TOOLS_BIN): $(TOOLS_BIN)_tmp$(MAKE_CURPID)
	$(DIR_GUARD)
	mv $< $@

$(TOOLS_LIB)_tmp$(MAKE_CURPID) : $(TOOLS_OBJS)
	$(DIR_GUARD)
	$(AR) cru $@ $<

$(TOOLS_LIB): $(TOOLS_LIB)_tmp$(MAKE_CURPID)
	$(DIR_GUARD)
	mv $< $@
