# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$

# This file must be included after the CPU-specific Rules.mak.

ifndef NO_LISTINGS
  LSTFILE = > $(LSTDIR)/$*.lst
  CFLAGS  += -Wa,-al
  ASFLAGS += -Wa,-al
endif

# Generate the final set of build flags and switches
#
DEFINES  := $(DEFINES_$(BUILD)) $(DEFINES)
INCDIR   := . $(INCDIR) $(SYSINCDIR)
CFLAGS   := -Wall -W $(CFLAGS_$(BUILD)) $(CFLAGS)
CXXFLAGS := -Wall -W $(CXXFLAGS_$(BUILD)) $(CXXFLAGS)
LIBDIR   := . $(LIBDIR) $(SYSLIBDIR)

# If a target was specified, then generate generic rules for it
#
ifdef TARGET
  CLEANSTEP = -rm -rf $(BUILDDIR)
endif

all: $(TARGET_DIR) $(FINAL_TARGET) $(EXTRATARGETS)

$(FINAL_TARGET): $(BUILD_TARGETS) $(OBJS)
	@$(ECHO) $(TARGET):$@
	@$(LINKSTEP)

.PHONY: clean
clean: $(EXTRACLEAN)
	$(CLEANSTEP)

# Create the output build directories if they don't exist
#
Makefile: $(BUILDDIR) $(OBJDIR) $(DEPDIR) $(LSTDIR)

$(TARGET_DIR):
	$(MKDIR) -p $(TARGET_DIR)

$(BUILDDIR):
	@$(MKDIR) -p $(BUILDDIR)
$(OBJDIR):
	@$(MKDIR) -p $(OBJDIR)
$(DEPDIR):
	@$(MKDIR) -p $(DEPDIR)
$(LSTDIR):
	@$(MKDIR) -p $(LSTDIR)

# Make sure 'make' knows where to look for object files
#
vpath %.o $(OBJDIR)

# Rebuild all object files if the Makefile changes
#
$(OBJS): Makefile

# Include auto-generated dependency files
#
DEPFILES := $(wildcard $(DEPDIR)/*.d)
ifneq ($(DEPFILES),)
  -include $(DEPFILES)
endif

# Global build rules for source files
#
.SUFFIXES:
.SUFFIXES: .c .cc .cpp .s .S .dsm .o

%.o: %.s
	@$(ECHO) $(TARGET):$(OBJDIR)/$@
	@$(AS_COMPILE) -o $(OBJDIR)/$@ $< $(LSTFILE)

%.o: %.S
	@$(ECHO) $(TARGET):$(OBJDIR)/$@
	@$(AS_COMPILE) -o $(OBJDIR)/$@ $< $(LSTFILE)

ifeq ($(CPU),ee)
%.o: %.dsm
	@$(ECHO) $(TARGET):$(OBJDIR)/$@
	@$(DVPAS_COMPILE) -o $(OBJDIR)/$@ $< $(LSTFILE)
endif

# These rules will also generate source dependencies
#
%.o: %.c
	@$(ECHO) $(TARGET):$(OBJDIR)/$@
	@if $(C_COMPILE) -MD -MP -MF "$(DEPDIR)/$*.Td" \
	  -c $< -o $(OBJDIR)/$*.o $(LSTFILE); \
	then sed 's/$(subst /,\/,$(OBJDIR))\/\($*\)\.o[ :]*/\1.o : /g' < "$(DEPDIR)/$*.Td" > "$(DEPDIR)/$*.d"; \
	  rm -f "$(DEPDIR)/$*.Td"; \
	else rm -f "$(DEPDIR)/$*.Td"; exit 1; \
	fi

%.o: %.cc
	@$(ECHO) $(TARGET):$(OBJDIR)/$@
	@if $(CXX_COMPILE) -MD -MP -MF "$(DEPDIR)/$*.Td" \
	  -c $< -o $(OBJDIR)/$*.o $(LSTFILE); \
	then sed 's/$(subst /,\/,$(OBJDIR))\/\($*\)\.o[ :]*/\1.o : /g' < "$(DEPDIR)/$*.Td" > "$(DEPDIR)/$*.d"; \
	  rm -f "$(DEPDIR)/$*.Td"; \
	else rm -f "$(DEPDIR)/$*.Td"; exit 1; \
	fi

%.o: %.cpp
	@$(ECHO) $(TARGET):$(OBJDIR)/$@
	@if $(CXX_COMPILE) -MD -MP -MF "$(DEPDIR)/$*.Td" \
	  -c $< -o $(OBJDIR)/$*.o $(LSTFILE); \
	then sed 's/$(subst /,\/,$(OBJDIR))\/\($*\)\.o[ :]*/\1.o : /g' < "$(DEPDIR)/$*.Td" > "$(DEPDIR)/$*.d"; \
	  rm -f "$(DEPDIR)/$*.Td"; \
	else rm -f "$(DEPDIR)/$*.Td"; exit 1; \
	fi
