# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$

CPU = ee

include $(PS2SDKSRC)/Defs.mak

# Variables that are specific to the build
#
DEFINES_debug   := DEBUG=1 $(DEFINES_debug)
DEFINES_release := NDEBUG=1 $(DEFINES_release)

CFLAGS_debug     := -g -O0 $(CFLAGS_debug)
CFLAGS_release   := -O2 -ffast-math $(CFLAGS_release)
CXXFLAGS_debug   := -g -O0 $(CXXFLAGS_debug)
CXXFLAGS_release := -O2 -ffast-math $(CXXFLAGS_release)

# EE-specific #defines, include paths, compiler flags, and linker flags
#
DEFINES  := _EE=1 PS2=1 _PS2=1 $(DEFINES)

SYSINCDIR := $(PS2SDKSRC)/ee/kernel/include $(PS2SDKSRC)/common/include $(PS2SDKSRC)/ee/libc/include
SYSLIBDIR := 

ASFLAGS  := -c -xassembler-with-cpp $(ASFLAGS)
SYSLIBS  := c kernel
LDFLAGS  := -mno-crt0 $(addprefix -L,$(LIBDIR)) $(LDFLAGS) $(addprefix -l,$(LIBS)) $(addprefix -l,$(SYSLIBS))

# We need to build the libraries with -G0 to disable GP-relative addressing
#
ifdef BUILDING_SYSTEM_LIBRARIES
  CFLAGS += -G0
endif

# Create the target file
#
ifdef TARGET
  ALLOBJS := $(addprefix $(OBJDIR)/,$(OBJS))

  # Libraries
  ifeq ($(findstring .a,$(TARGET)),.a)
    LINKSTEP = $(AR) cru $@ $(ALLOBJS)
  endif

  # Executables
  ifeq ($(findstring .elf,$(TARGET)),.elf)
  endif

  ifdef TARGET_DIR
    FINAL_TARGET = $(TARGET_DIR)/$(TARGET)
  else
    FINAL_TARGET = $(BUILDDIR)/$(TARGET)
  endif
endif

# System libraries always have their source files in src/ and headers in include/
#
ifdef BUILDING_SYSTEM_LIBRARIES
  VPATH := src
  INCDIR := include $(INCDIR)
endif

include $(PS2SDKSRC)/common/BuildRules.mak
