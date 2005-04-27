# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$

# This is the default type of build, it can be overridden on the command line.
# Set it to "debug" for debug builds, and "release" for release builds.
export BUILD = debug

# Build directories
#
# I've decided to use the MSVC naming scheme (Debug, Release) so that it's
# easier to distinguish build directories from source directories.
#
ifeq ($(findstring debug,$(BUILD)),debug)
  export BUILDDIR = Debug
endif
ifeq ($(findstring release,$(BUILD)),release)
  export BUILDDIR = Release
endif

OBJDIR = $(BUILDDIR)/obj
DEPDIR = $(BUILDDIR)/dep
LSTDIR = $(BUILDDIR)/lst

# Toolchain definitions
#
ifeq ($(CPU),ee)
  ifndef EE_TOOL_PREFIX
    EE_TOOL_PREFIX := $(CPU)-
  endif

  CC      = $(EE_TOOL_PREFIX)gcc
  CXX     = $(EE_TOOL_PREFIX)g++
  AS      = $(EE_TOOL_PREFIX)gcc
  DVPAS   = $(EE_TOOL_PREFIX)dvp-as
  LD      = $(EE_TOOL_PREFIX)gcc
  AR      = $(EE_TOOL_PREFIX)ar
  OBJCOPY = $(EE_TOOL_PREFIX)objcopy
  STRIP   = $(EE_TOOL_PREFIX)strip
else
  ifeq ($(CPU),iop)
    ifndef IOP_TOOL_PREFIX
      IOP_TOOL_PREFIX := $(CPU)-
    endif

    CC      = $(IOP_TOOL_PREFIX)gcc
    CXX     = $(IOP_TOOL_PREFIX)g++
    AS      = $(IOP_TOOL_PREFIX)gcc
    LD      = $(IOP_TOOL_PREFIX)gcc
    AR      = $(IOP_TOOL_PREFIX)ar
    OBJCOPY = $(IOP_TOOL_PREFIX)objcopy
    STRIP   = $(IOP_TOOL_PREFIX)strip
  else
    CC      = gcc
    CXX     = g++
    AS      = gcc
    LD      = gcc
    AR      = ar
    OBJCOPY = objcopy
    STRIP   = strip
  endif
endif

# Other utilities
#
TOUCH = touch

SYSTEM := $(shell uname)
ifeq ($(SYSTEM),CYGWIN_NT-5.1)
  # these versions are used for the cygwin toolchain in a dos environment
  # since they need to overwrite the standard dos versions of each command
  MKDIR = cyg-mkdir
  RMDIR = cyg-rmdir
  ECHO  = cyg-echo
  GNUMAKE = make
else
  MKDIR = mkdir
  RMDIR = rmdir
  ECHO  = echo
  GNUMAKE = make
endif

# Aliases used to build source files
#
C_COMPILE   = $(CC)  $(addprefix -D,$(DEFINES)) $(addprefix -I,$(INCDIR)) $(CFLAGS)
CXX_COMPILE = $(CXX) $(addprefix -D,$(DEFINES)) $(addprefix -I,$(INCDIR)) $(CXXFLAGS)
AS_COMPILE  = $(AS)  $(addprefix -D,$(DEFINES)) $(addprefix -I,$(INCDIR)) $(ASFLAGS)
ifeq ($(CPU),ee)
  DVPAS_COMPILE = $(DVPASM) $(addprefix -I,$(INCDIR)) $(INCLUDES) $(DVPASMFLAGS)
endif
