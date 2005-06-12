# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#

EE_OBJS_DIR = obj/
EE_LIB_DIR = lib/
EE_SRC_DIR = src/
EE_INC_DIR = include/
EE_CFLAGS = -fshort-double -mlong64 -mhard-float -fno-builtin

EE_OBJS = acoshf.o airyf.o asinf.o asinhf.o atanf.o \
atanhf.o bdtrf.o betaf.o cbrtf.o chbevlf.o chdtrf.o \
clogf.o cmplxf.o constf.o coshf.o dawsnf.o ellief.o \
ellikf.o ellpef.o ellpkf.o ellpjf.o expf.o exp2f.o \
exp10f.o expnf.o expx2f.o fabsf.o facf.o fdtrf.o floorf.o fresnlf.o \
gammaf.o gdtrf.o hypergf.o hyp2f1f.o igamf.o igamif.o \
incbetf.o incbif.o i0f.o i1f.o ivf.o j0f.o j1f.o \
jnf.o jvf.o k0f.o k1f.o knf.o logf.o log2f.o \
log10f.o nbdtrf.o ndtrf.o ndtrif.o pdtrf.o polynf.o \
powif.o powf.o psif.o rgammaf.o shichif.o sicif.o \
sindgf.o sinf.o sinhf.o spencef.o sqrtf.o stdtrf.o \
struvef.o tandgf.o tanf.o tanhf.o ynf.o zetaf.o \
zetacf.o polevlf.o setprec.o mtherr.o erl-support.o

EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

EE_LIB = $(EE_LIB_DIR)libmf.a

all: $(EE_OBJS_DIR) $(EE_LIB_DIR) $(EE_LIB)

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_LIB_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release
