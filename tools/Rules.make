# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.


# C compiler flags
TOOLS_CFLAGS := -O2 -Wall $(TOOLS_CFLAGS)

# C++ compiler flags
TOOLS_CXXFLAGS := -O2 -Wall $(TOOLS_CXXFLAGS)

# Linker flags
#TOOLS_LDFLAGS := $(TOOLS_LDFLAGS)

# Assembler flags
#TOOLS_ASFLAGS := $(TOOLS_ASFLAGS)

# Externally defined variables: TOOLS_BIN, TOOLS_OBJS, TOOLS_LIB

# These macros can be used to simplify certain build rules.
TOOLS_C_COMPILE = $(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_INCS)
TOOLS_CXX_COMPILE = $(TOOLS_CC) $(TOOLS_CXXFLAGS) $(TOOLS_INCS)


$(TOOLS_OBJS_DIR)%.o : $(TOOLS_SRC_DIR)%.c
	$(CC) $(TOOLS_CFLAGS) $(TOOLS_INCS) -c $< -o $@

$(TOOLS_OBJS_DIR)%.o : $(TOOLS_SRC_DIR)%.cpp
	$(CXX) $(TOOLS_CXXFLAGS) $(TOOLS_INCS) -c $< -o $@

$(TOOLS_OBJS_DIR)%.o : $(TOOLS_SRC_DIR)%.S
	$(CC) $(TOOLS_CFLAGS) $(TOOLS_INCS) -c $< -o $@

$(TOOLS_OBJS_DIR)%.o : $(TOOLS_SRC_DIR)%.s
	$(AS) $(TOOLS_ASFLAGS) $< -o $@

$(TOOLS_BIN_DIR):
	mkdir $(TOOLS_BIN_DIR)

$(TOOLS_OBJS_DIR):
	mkdir $(TOOLS_OBJS_DIR)

$(TOOLS_BIN) : $(TOOLS_OBJS) 
	$(CC) $(TOOLS_LDFLAGS) -o $(TOOLS_BIN) $(TOOLS_OBJS) $(TOOLS_LIBS) 

$(TOOLS_LIB) : $(TOOLS_OBJS)
	$(AR) cru $(TOOLS_LIB) $<

