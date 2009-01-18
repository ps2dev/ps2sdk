# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$


IOP_CC_VERSION := $(shell $(IOP_CC) --version 2>&1 | sed -n 's/^.*(GCC) //p')

ASFLAGS_TARGET = -mcpu=r3000

ifeq ($(IOP_CC_VERSION),3.2.2)
ASFLAGS_TARGET = -march=r3000
endif

IOP_INCS := $(IOP_INCS) -I$(PS2SDKSRC)/iop/kernel/include -I$(PS2SDKSRC)/common/include -Iinclude

IOP_CFLAGS  := -fno-builtin -O2 -G0 -c $(IOP_INCS) $(IOP_CFLAGS)
IOP_ASFLAGS := $(ASFLAGS_TARGET) -EL -G0 $(IOP_ASFLAGS)
IOP_LDFLAGS := -fno-builtin -nostdlib $(IOP_LDFLAGS)

# Externally defined variables: IOP_BIN, IOP_OBJS, IOP_LIB

$(IOP_OBJS_DIR)%.o : $(IOP_SRC_DIR)%.c
	$(IOP_CC) $(IOP_CFLAGS) $< -o $@

$(IOP_OBJS_DIR)%.o : $(IOP_SRC_DIR)%.S
	$(IOP_CC) $(IOP_CFLAGS) $(IOP_INCS) -c $< -o $@
	
$(IOP_OBJS_DIR)%.o : $(IOP_SRC_DIR)%.s
	$(IOP_AS) $(IOP_ASFLAGS) $< -o $@

# A rule to build imports.lst.
$(IOP_OBJS_DIR)%.o : $(IOP_SRC_DIR)%.lst
	$(ECHO) "#include \"irx_imports.h\"" > $(IOP_OBJS_DIR)build-imports.c
	cat $< >> $(IOP_OBJS_DIR)build-imports.c
	$(IOP_CC) $(IOP_CFLAGS) -I$(IOP_SRC_DIR) $(IOP_OBJS_DIR)build-imports.c -o $@
	-rm -f $(IOP_OBJS_DIR)build-imports.c

# A rule to build exports.tab.
$(IOP_OBJS_DIR)%.o : $(IOP_SRC_DIR)%.tab
	$(ECHO) "#include \"irx.h\"" > $(IOP_OBJS_DIR)build-exports.c
	cat $< >> $(IOP_OBJS_DIR)build-exports.c
	$(IOP_CC) $(IOP_CFLAGS) -I$(IOP_SRC_DIR) $(IOP_OBJS_DIR)build-exports.c -o $@
	-rm -f $(IOP_OBJS_DIR)build-exports.c

$(IOP_OBJS_DIR):
	$(MKDIR) -p $(IOP_OBJS_DIR)

$(IOP_BIN_DIR):
	$(MKDIR) -p $(IOP_BIN_DIR)

$(IOP_LIB_DIR):
	$(MKDIR) -p $(IOP_LIB_DIR)

$(IOP_BIN) : $(IOP_OBJS)
	$(IOP_CC) $(IOP_LDFLAGS) -o $(IOP_BIN) $(IOP_OBJS) $(IOP_LIBS)

$(IOP_LIB) : $(IOP_OBJS)
	$(IOP_AR) cru $(IOP_LIB) $(IOP_OBJS)

