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


CORE_OBJS = sbrk.o memchr.o memcpy.o memset.o strcat.o \
	strncmp.o memcmp.o memmove.o strcpy.o strncat.o \
	strncpy.o terminate.o strlen.o strchr.o strcmp.o
CORE_OBJS := $(CORE_OBJS:%=$(EE_OBJS_DIR)%)

ALLOC_OBJS = malloc.o realloc.o calloc.o memalign.o free.o __builtin_alloc.o
ALLOC_OBJS := $(ALLOC_OBJS:%=$(EE_OBJS_DIR)%)

STRING_C_OBJS = strcasecmp.o strncasecmp.o strtok.o strrchr.o strstr.o strtol.o strtoul.o \
	strupr.o strlwr.o ctype.o
STRING_C_OBJS := $(STRING_C_OBJS:%=$(EE_OBJS_DIR)%)

XPRINTF_OBJS = vxprintf.o xprintf.o sout.o vsnprintf.o snprintf.o vsprintf.o sprintf.o \
	mout.o mprintf.o vmprintf.o fout.o fprintf.o vfprintf.o printf.o vprintf.o putchar.o \
	npmPuts.o nprintf.o vnprintf.o asprintf.o vasprintf.o
XPRINTF_OBJS := $(XPRINTF_OBJS:%=$(EE_OBJS_DIR)%)

STDIO_OBJS = clearerr.o fclose.o _fcloseall.o feof.o ferror.o fflush.o fflushall.o fgetc.o \
	fgetpos.o fgets.o fopen.o fputc.o fputs.o fread.o fseek.o fsetpos.o ftell.o fwrite.o \
	getc.o getchar.o getfdtype.o gets.o perror.o putc.o puts.o remove.o rename.o \
	rewind.o skipatoi.o sscanf.o stdio.o tmpfile.o tmpnam.o ungetc.o updatestdoutxy.o strerror.o
STDIO_OBJS := $(STDIO_OBJS:%=$(EE_OBJS_DIR)%)

MULT_OBJS = $(XPRINTF_OBJS) $(ALLOC_OBJS) $(STRING_C_OBJS) $(STDIO_OBJS)

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

$(STDIO_OBJS): src/stdio.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@


clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_LIB_DIR)


include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release
