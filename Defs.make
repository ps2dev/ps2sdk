# _____     ___ ____     ___ ____ 
#  ____|   |    ____|   |        | |____|    
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project. 
#-----------------------------------------------------------------------
# Copyright 2001-2004.   
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details. 

 
# You can override the following options on the make command line, or manually
# edit them below.  Please see the file INSTALL for details on building
# ps2sdk.

#
# Definitions for the EE toolchain.
#

EE_TOOL_PREFIX = ee-
EE_CC = $(EE_TOOL_PREFIX)gcc
EE_AS = $(EE_TOOL_PREFIX)as
EE_LD = $(EE_TOOL_PREFIX)ld
EE_AR = $(EE_TOOL_PREFIX)ar
EE_OBJCOPY = $(EE_TOOL_PREFIX)objcopy
EE_STRIP = $(EE_TOOL_PREFIX)strip


#
# Defintions for the IOP toolchain.
#

IOP_TOOL_PREFIX = iop-
IOP_CC = $(IOP_TOOL_PREFIX)gcc
IOP_AS = $(IOP_TOOL_PREFIX)as
IOP_LD = $(IOP_TOOL_PREFIX)ld
IOP_AR = $(IOP_TOOL_PREFIX)ar
IOP_OBJCOPY = $(IOP_TOOL_PREFIX)objcopy
IOP_STRIP = $(IOP_TOOL_PREFIX)strip


