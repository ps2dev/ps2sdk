# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

.SUFFIXES:.c
VPATH = src:obj

LIBC_LIB  = lib/libc.a

# TODO: This is a hack ... it will be killed when new build system is
# installed.
OBJDIR = obj/
OBJEXIST = $(OBJDIR).created
LIBDIR = lib/
LIBEXIST = $(LIBDIR).created


ALLOC_OBJS = malloc.o realloc.o calloc.o memalign.o free.o __builtin_alloc.o
ALLOC_OBJS := $(ALLOC_OBJS:%=$(OBJDIR)%)

STRING_C_OBJS = strcasecmp.o strncasecmp.o strtok.o strrchr.o strstr.o strtol.o \
	strupr.o strlwr.o ctype.o
STRING_C_OBJS := $(STRING_C_OBJS:%=$(OBJDIR)%)

CORE_OBJS = sbrk.o memchr.o memcpy.o memset.o strcat.o \
	strncmp.o memcmp.o memmove.o strcpy.o strncat.o \
	strncpy.o terminate.o strlen.o strchr.o strcmp.o
CORE_OBJS := $(CORE_OBJS:%=$(OBJDIR)%)

MULT_OBJS = $(XPRINTF_OBJS) $(ALLOC_OBJS) $(STRING_C_OBJS) 

all: $(OBJEXIST) $(LIBEXIST) $(LIBC_LIB) 

$(LIBC_LIB): $(CORE_OBJS) $(MULT_OBJS) 
	$(EE_AR) cru $(LIBC_LIB) $(CORE_OBJS) $(MULT_OBJS) 


$(OBJEXIST):
	@echo; echo "Creating object directory"
	test -d $(dir $@) || mkdir $(dir $@)
	touch $@

$(LIBEXIST):
	@echo; echo "Creating library directory"
	test -d $(dir $@) || mkdir $(dir $@)
	touch $@


.c.o :
	$(EE_CC) $(EE_CFLAGS) $(EE_INC) -c $< -o obj/$@

$(XPRINTF_OBJS): src/xprintf.c
	$(EE_C_COMPILE) -DF_$(*:$(OBJDIR)%=%) $< -c -o $@

$(ALLOC_OBJS): src/alloc.c
	$(EE_C_COMPILE) -DF_$(*:$(OBJDIR)%=%) $< -c -o $@

$(STRING_C_OBJS): src/string.c
	$(EE_C_COMPILE) -DF_$(*:$(OBJDIR)%=%) $< -c -o $@


clean:
	$(RM) -rf $(OBJDIR)
	$(RM) -rf $(LIBDIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make

