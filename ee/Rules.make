# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_CC_VERSION := $(shell $(EE_CC) -dumpversion)

EE_OBJS_DIR ?= obj/
EE_SRC_DIR ?= src/
EE_INC_DIR ?= include/
EE_SAMPLE_DIR ?= samples/

# Helpers to make easy the use of newlib-nano
EE_NEWLIB_NANO ?= 0
EE_COMPACT_EXECUTABLE ?= 0

EE_NANO_NEWLIB_LIBS ?= $(PS2SDKSRC)/ee/rpc/cdvd/lib/libcdvd.a $(PS2SDKSRC)/ee/libcglue/lib/libcglue.a $(PS2SDKSRC)/ee/kernel/lib/libkernel.a

ifneq (x$(EE_NEWLIB_NANO), x0)
EE_ADDITIONAL_DEPS := $(EE_ADDITIONAL_DEPS) $(EE_NANO_NEWLIB_LIBS)
endif

EE_INCS := $(EE_INCS) -I$(EE_SRC_DIR) -I$(EE_SRC_DIR)include -I$(EE_INC_DIR) -I$(PS2SDKSRC)/ee/kernel/include -I$(PS2SDKSRC)/common/include -I$(PS2SDKSRC)/ee/libcglue/include -I$(PS2SDKSRC)/ee/erl/include

ifneq (x$(EE_COMPACT_EXECUTABLE), x0)
EE_OPTFLAGS ?= -Os
endif

# Optimization compiler flags
EE_OPTFLAGS ?= -O2

# Warning compiler flags
EE_WARNFLAGS ?= -Wall -Werror

# Debug information flags
EE_DBGINFOFLAGS ?= -gdwarf-2 -gz

# These flags will generate LTO and non-LTO code in the same object file,
# allowing the choice of using LTO or not in the final linked binary.
EE_FATLTOFLAGS ?= -flto -ffat-lto-objects

# C compiler flags
EE_CFLAGS := -D_EE -G0 $(EE_OPTFLAGS) $(EE_WARNFLAGS) $(EE_DBGINFOFLAGS) $(EE_INCS) $(EE_CFLAGS)

ifeq ($(DEBUG),1)
EE_CFLAGS += -DDEBUG
endif
# C++ compiler flags
EE_CXXFLAGS := -D_EE -G0 -$(EE_OPTFLAGS) $(EE_WARNFLAGS) $(EE_DBGINFOFLAGS) $(EE_INCS) $(EE_CXXFLAGS)

# Linker flags
# EE_LDFLAGS := $(EE_LDFLAGS)

# Assembler flags
EE_ASFLAGS := -call_shared $(EE_ASFLAGS)

EE_SAMPLES := $(EE_SAMPLES:%=$(EE_SAMPLE_DIR)%)

EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

EE_LINKFILE ?= $(PS2SDKSRC)/ee/startup/src/linkfile

EE_BIN_MAPFILE ?= $(shell basename $(CURDIR)).map

ifneq (x$(EE_COMPACT_EXECUTABLE), x0)
EE_LDFLAGS += -Wl,-zmax-page-size=128 -s -Wl,--gc-sections
EE_CFLAGS += -fdata-sections -ffunction-sections
endif

ifneq (x$(EE_NEWLIB_NANO), x0)
EE_LDFLAGS := $(EE_LDFLAGS) -nodefaultlibs -lm_nano -lgcc -Wl,--start-group -lc_nano $(EE_NANO_NEWLIB_LIBS) -Wl,--end-group
endif

# Externally defined variables: EE_BIN, EE_OBJS, EE_LIB

# These macros can be used to simplify certain build rules.
EE_C_COMPILE = $(EE_CC) $(EE_CFLAGS)
EE_CXX_COMPILE = $(EE_CXX) $(EE_CXXFLAGS)

# Command for ensuring the output directory for the rule exists.
DIR_GUARD = @$(MKDIR) -p $(@D)

PS2SDK_CRT0 = $(PS2SDKSRC)/ee/startup/obj/crt0.o

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) $(EE_FATLTOFLAGS) -c $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.cpp
	$(DIR_GUARD)
	$(EE_CXX_COMPILE) $(EE_FATLTOFLAGS) -c $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.S
	$(DIR_GUARD)
	$(EE_C_COMPILE) -c $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.s
	$(DIR_GUARD)
	$(EE_AS) $(EE_ASFLAGS) $< -o $@

$(EE_BIN): $(EE_OBJS) $(EE_LIB_ARCHIVES) $(EE_ADDITIONAL_DEPS) $(PS2SDK_CRT0)
	$(DIR_GUARD)
	$(EE_CC) -T$(EE_LINKFILE) -Wl,-Map,$(EE_BIN_MAPFILE) $(EE_OPTFLAGS) \
		-o $(EE_BIN) -nostartfiles $(PS2SDK_CRT0) $(EE_OBJS) $(EE_LDFLAGS) $(EE_LIB_ARCHIVES) $(EE_LIBS)

$(EE_LIB): $(EE_OBJS) $(EE_LIB:%.a=%.erl)
	$(DIR_GUARD)
	$(EE_AR) cru $(EE_LIB) $(EE_OBJS)

$(EE_LIB:%.a=%.erl): $(EE_OBJS)
	$(DIR_GUARD)
	$(EE_CC) -nostdlib $(EE_NO_CRT) -Wl,-r -Wl,-d -o $(EE_LIB:%.a=%.erl) $(EE_OBJS)
	$(EE_STRIP) --strip-unneeded -R .mdebug.eabi64 -R .reginfo -R .comment $(EE_LIB:%.a=%.erl)
