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

EE_OBJS_DIR = obj/
EE_LIB_DIR = lib/
EE_SRC_DIR = src/
EE_INC_DIR = include/
EE_SAMPLE_DIR = samples/

EE_LIB = $(LIBC_LIB)
EE_SAMPLES = hello

EE_SAMPLES := $(EE_SAMPLES:%=$(EE_SAMPLE_DIR)%)


ALLOC_OBJS = malloc.o realloc.o calloc.o memalign.o free.o __builtin_alloc.o
ALLOC_OBJS := $(ALLOC_OBJS:%=$(EE_OBJS_DIR)%)

STRING_C_OBJS = strcasecmp.o strncasecmp.o strtok.o strrchr.o strstr.o strtol.o strtoul.o \
	strupr.o strlwr.o ctype.o
STRING_C_OBJS := $(STRING_C_OBJS:%=$(EE_OBJS_DIR)%)

CORE_OBJS = sbrk.o memchr.o memcpy.o memset.o strcat.o \
	strncmp.o memcmp.o memmove.o strcpy.o strncat.o \
	strncpy.o terminate.o strlen.o strchr.o strcmp.o
CORE_OBJS := $(CORE_OBJS:%=$(EE_OBJS_DIR)%)

XPRINTF_OBJS = vsnprintf.o vsprintf.o vprintf.o printf.o sprintf.o snprintf.o \
	npmPuts.o nprintf.o putchar.o puts.o
XPRINTF_OBJS := $(XPRINTF_OBJS:%=$(EE_OBJS_DIR)%)

MULT_OBJS = $(XPRINTF_OBJS) $(ALLOC_OBJS) $(STRING_C_OBJS) 

EE_OBJS = $(CORE_OBJS) $(MULT_OBJS)

all: $(EE_OBJS_DIR) $(EE_LIB_DIR) $(LIBC_LIB) 


.c.o :
	$(EE_CC) $(EE_CFLAGS) $(EE_INC) -c $< -o obj/$@

$(XPRINTF_OBJS): src/xprintf.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(ALLOC_OBJS): src/alloc.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(STRING_C_OBJS): src/string.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@


clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_LIB_DIR)


include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release
