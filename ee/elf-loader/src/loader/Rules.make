# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_CC_VERSION := $(shell $(EE_CC) -dumpversion)

EE_SRC_DIR ?= src/
EE_INC_DIR ?= include/

# Include directories
EE_INCS := $(EE_INCS) -I$(EE_SRC_DIR) -I$(EE_SRC_DIR)include -I$(EE_INC_DIR) -I$(PS2SDKSRC)/ee/kernel/include -I$(PS2SDKSRC)/common/include -I$(PS2SDKSRC)/ee/libc/include -I$(PS2SDKSRC)/ee/erl/include

# C compiler flags
EE_CFLAGS := -D_EE -O2 -G0 -Wall -Werror $(EE_CFLAGS)

# C++ compiler flags
EE_CXXFLAGS := -D_EE -O2 -G0 -Wall -Werror $(EE_CXXFLAGS)

# Linker flags
EE_LDFLAGS := -L$(PS2SDKSRC)/ee/kernel/lib -L$(PS2SDKSRC)/ee/libc/lib $(EE_LDFLAGS)

# Assembler flags
EE_ASFLAGS := -G0 $(EE_ASFLAGS)

# Externally defined variables: EE_BIN, EE_OBJS, EE_LIB

# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# These macros can be used to simplify certain build rules.
EE_C_COMPILE = $(EE_CC) $(EE_CFLAGS) $(EE_INCS)
EE_CXX_COMPILE = $(EE_CXX) $(EE_CXXFLAGS) $(EE_INCS)

%.o: %.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

%.o: %.cc
	$(EE_CXX) $(EE_CXXFLAGS) $(EE_INCS) -c $< -o $@

%.o: %.cpp
	$(EE_CXX) $(EE_CXXFLAGS) $(EE_INCS) -c $< -o $@

%.o: %.S
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

%.o: %.s
	$(EE_AS) $(EE_ASFLAGS) $< -o $@

$(EE_BIN): $(EE_OBJS)
	$(EE_CC) $(EE_CFLAGS) -o $(EE_BIN) $(EE_OBJS) $(EE_LDFLAGS) $(EE_LIBS)

$(EE_ERL): $(EE_OBJS)
	$(EE_CC) -o $(EE_ERL) $(EE_OBJS) $(EE_CFLAGS) $(EE_LDFLAGS) -Wl,-r -Wl,-d
	$(EE_STRIP) --strip-unneeded -R .mdebug.eabi64 -R .reginfo -R .comment $(EE_ERL)

$(EE_LIB): $(EE_OBJS)
	$(EE_AR) cru $(EE_LIB) $(EE_OBJS)
