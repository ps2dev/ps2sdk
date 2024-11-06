# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

ifndef SUBDIRS
SUBDIRS =
SUBDIRS += debug/helloworld
SUBDIRS += debug/callstacktest
SUBDIRS += draw/cube
SUBDIRS += draw/teapot
SUBDIRS += draw/texture
SUBDIRS += draw/vu1
#SUBDIRS += erl/hello               #TODO: missing /ps2sdk/ee/lib/libc.erl
#SUBDIRS += erl/cpp-hello           #TODO: missing /ps2sdk/ee/lib/libc.erl
SUBDIRS += font
SUBDIRS += graph
SUBDIRS += kernel/systemTime
SUBDIRS += kernel/timer_alarm
SUBDIRS += kernel/timer_alarm_sleep
SUBDIRS += kernel/timer_alarm_stop
SUBDIRS += kernel/noPatchedHelloWorld
SUBDIRS += kernel/nanoHelloWorld
SUBDIRS += libcglue/regress
SUBDIRS += libcglue/mem_test
SUBDIRS += libcglue/nanosleep
SUBDIRS += libcglue/rewinddir
SUBDIRS += libcglue/timezone
SUBDIRS += libgs/doublebuffer
SUBDIRS += libgs/draw
SUBDIRS += libprofglue/basic
SUBDIRS += libprofglue/custom
#SUBDIRS += mpeg                #TODO: not modified for updated newlib
SUBDIRS += network/tcpip-basic
SUBDIRS += network/tcpip-dhcp
SUBDIRS += rpc/tcpips/ee-echo
SUBDIRS += rpc/audsrv/playadpcm
SUBDIRS += rpc/audsrv/playcdda
SUBDIRS += rpc/audsrv/playwav
SUBDIRS += rpc/audsrv/playwav2
SUBDIRS += rpc/audsrv/testcd
SUBDIRS += rpc/filexio
#SUBDIRS += rpc/camera          #TODO: not modified for updated newlib
#SUBDIRS += rpc/memorycard      #TODO: not modified for updated newlib
SUBDIRS += rpc/mtap
SUBDIRS += rpc/pad
SUBDIRS += rpc/padx
SUBDIRS += rpc/poweroff
SUBDIRS += rpc/ps2snd
SUBDIRS += rpc/remote
endif

all: $(patsubst %, _dir_%, $(SUBDIRS))

$(patsubst %, _dir_%, $(SUBDIRS)):
	@+$(MAKE) -r -C $(patsubst _dir_%, %, $@)

clean: $(patsubst %, _cleandir_%, $(SUBDIRS))

$(patsubst %, _cleandir_%, $(SUBDIRS)):
	+$(MAKE) -C $(patsubst _cleandir_%, %, $@) clean
