# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_INCS := $(EE_INCS) -I$(PS2SDKSRC)/ee/kernel/include -I$(PS2SDKSRC)/common/include -Iinclude

# C compiler flags
EE_CFLAGS := -D_EE -O2 -G0 -Wall $(EE_CFLAGS)

# C++ compiler flags
EE_CXXFLAGS := -D_EE -O2 -G0 -Wall $(EE_CXXFLAGS)

# Linker flags
#EE_LDFLAGS := $(EE_LDFLAGS)

# Assembler flags
EE_ASFLAGS := -G0 $(EE_ASFLAGS)

# Externally defined variables: EE_BIN, EE_OBJS, EE_LIB

# These macros can be used to simplify certain build rules.
EE_C_COMPILE = $(EE_CC) $(EE_CFLAGS) $(EE_INCS)
EE_CXX_COMPILE = $(EE_CC) $(EE_CXXFLAGS) $(EE_INCS)


$(EE_OBJS_DIR)%.o : $(EE_SRC_DIR)%.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)%.o : $(EE_SRC_DIR)%.cpp
	$(EE_CXX) $(EE_CXXFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)%.o : $(EE_SRC_DIR)%.S
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)%.o : $(EE_SRC_DIR)%.s
	$(EE_AS) $(EE_ASFLAGS) $< -o $@

$(EE_LIB_DIR):
	mkdir $(EE_LIB_DIR)

$(EE_BIN_DIR):
	mkdir $(EE_BIN_DIR)

$(EE_OBJS_DIR):
	mkdir $(EE_OBJS_DIR)

$(EE_BIN) : $(EE_OBJS) $(PS2SDKSRC)/ee/startup/obj/crt0.o
	$(EE_CC) -nostartfiles -T$(PS2SDKSRC)/ee/startup/src/linkfile $(EE_LDFLAGS) \
		-o $(EE_BIN) $(PS2SDKSRC)/ee/startup/obj/crt0.o $(EE_OBJS) $(EE_LIBS)

$(EE_LIB) : $(EE_OBJS)
	$(EE_AR) cru $(EE_LIB) $(EE_OBJS)
