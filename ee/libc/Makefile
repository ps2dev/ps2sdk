# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_LIB = libc.a

# uncomment the following line to enable profile on libc
# EE_CFLAGS += -pg

EE_SAMPLES = hello prof regress

CORE_OBJS = sbrk.o memchr.o memcpy.o memset.o strcat.o \
	strncmp.o memcmp.o memmove.o strcpy.o strncat.o \
	strncpy.o terminate.o strlen.o strchr.o strcmp.o \
	setjmp.o qsort.o cxx.o

INIT_OBJS = init_libc.o init_args.o

ALLOC_OBJS = malloc.o realloc.o calloc.o memalign.o free.o __builtin_alloc.o __alloc_internals.o __mem_walk.o

STRING_C_OBJS = strdup.o strcasecmp.o strncasecmp.o strtok.o strrchr.o strstr.o \
	strupr.o strlwr.o _sjis_internals.o strcpy_ascii.o strcpy_sjis.o \
	strpbrk.o strspn.o strcspn.o \
	tolower.o toupper.o isupper.o islower.o isalpha.o isdigit.o isalnum.o iscntrl.o \
	isgraph.o isprint.o ispunct.o isspace.o isxdigit.o

WSTRING_C_OBJS = wcsdup.o wcscasecmp.o wcsncasecmp.o wcstok.o wcsrchr.o wcswcs.o wcsupr.o wcslwr.o towlower.o towupper.o iswupper.o iswlower.o iswalpha.o iswdigit.o iswalnum.o iswcntrl.o iswgraph.o iswprint.o iswpunct.o iswspace.o iswxdigit.o wcscpy.o wcsncpy.o wcspbrk.o wcsspn.o wcscspn.o wcscmp.o wcsncmp.o wcslen.o wcschr.o wcscat.o wcsncat.o

XPRINTF_OBJS = vxprintf.o xprintf.o __sout.o vsnprintf.o snprintf.o vsprintf.o sprintf.o \
	__mout.o mprintf.o vmprintf.o __fout.o fprintf.o vfprintf.o printf.o vprintf.o putchar.o \
	asprintf.o vasprintf.o sio_printf.o

WXPRINTF_OBJS = vxwprintf.o __swout.o vsnwprintf.o snwprintf.o vswprintf.o swprintf.o

XSCANF_OBJS = vxscanf.o xscanf.o scanf.o vscanf.o sscanf.o vsscanf.o fscanf.o vfscanf.o _xscanf_internals.o

STDIO_OBJS = clearerr.o fclose.o fcloseall.o feof.o ferror.o fflush.o fflushall.o fgetc.o \
	fgetpos.o fgets.o fopen.o fputc.o fputs.o fread.o fseek.o fsetpos.o ftell.o fwrite.o fileno.o \
	getc.o getchar.o getfdtype.o gets.o perror.o putc.o puts.o remove.o fdopen.o \
	rewind.o skipatoi.o stdio.o tmpfile.o tmpnam.o ungetc.o updatestdoutxy.o strerror.o \
	__stdio_internals.o __stdio_helper_internals.o chdir.o

STDLIB_OBJS = abs.o atexit.o atof.o bsearch.o div.o getenv.o _itoa.o labs.o \
	ldiv.o llabs.o lldiv.o _lltoa.o _ltoa.o rand.o setenv.o srand.o \
	strtod.o strtol.o strtoul.o __assert_fail.o \
	__stdlib_internals.o __stdlib_environmentals.o \
	mblen.o mbslen.o mbstowcs.o mbtowc.o wcstombs.o wctomb.o

WSTDLIB_OBJS = wcstod.o wcstol.o wcstoul.o

UNISTD_OBJS = stat.o fstat.o access.o unlink.o getcwd.o sleep.o

LIBGEN_OBJS = basename.o dirname.o

TIME_OBJS = clock.o

DIRENT_OBJS = opendir.o readdir.o rewinddir.o closedir.o

GPROF_OBJS = mcount.o mcount_internal.o

EE_OBJS = $(CORE_OBJS) $(XPRINTF_OBJS) $(WXPRINTF_OBJS) $(XSCANF_OBJS) $(ALLOC_OBJS) $(STRING_C_OBJS) \
	$(WSTRING_C_OBJS) $(STDIO_OBJS) $(STDLIB_OBJS) $(WSTDLIB_OBJS) $(UNISTD_OBJS) $(TIME_OBJS) \
	$(GPROF_OBJS) $(LIBGEN_OBJS) $(DIRENT_OBJS) $(INIT_OBJS)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(XPRINTF_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)xprintf.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(WXPRINTF_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)xwprintf.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(XSCANF_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)xscanf.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(ALLOC_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)alloc.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(STRING_C_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)string.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(WSTRING_C_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)wstring.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(STDIO_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)stdio.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(STDLIB_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)stdlib.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(WSTDLIB_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)wstdlib.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(UNISTD_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)unistd.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(TIME_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)time.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(LIBGEN_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)libgen.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(DIRENT_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)dirent.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

$(INIT_OBJS:%=$(EE_OBJS_DIR)%): $(EE_SRC_DIR)init.c
	$(EE_C_COMPILE) -DF_$(*:$(EE_OBJS_DIR)%=%) $< -c -o $@

