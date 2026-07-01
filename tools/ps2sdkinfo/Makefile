# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

TOOLS_INCS += -I$(TOOLS_OBJS_DIR)
TOOLS_CFLAGS += -std=c99 -D_DEFAULT_SOURCE=1
TOOLS_LDFLAGS +=

TOOLS_OBJS = main.o

PS2SDK_GIT_REMOTE_URL := $(shell git -C $(CURDIR) config --get remote.origin.url)
PS2SDK_GIT_HASH := $(shell git -C $(CURDIR) log -1 '--format=%H')
PS2SDK_GIT_TAG := $(shell git -C $(CURDIR) log -1 '--format=%(describe:tags,abbrev=0)')
PS2SDK_GIT_TIME := $(shell git -C $(CURDIR) log -1 '--format=%ct')
PS2SDK_GIT_TIME_ISO8601 := $(shell git -C $(CURDIR) log -1 '--format=%cd' --date="format:%Y-%m-%dT%H:%M:%S%z")
PS2SDK_GIT_TIME_RFC2822 := $(shell git -C $(CURDIR) log -1 '--format=%cd' --date="format:%a, %d %b %Y %H:%M:%S %z")

PS2SDK_BUILD_PATH := $(PS2SDKSRC)
PS2SDK_BUILD_USER := $(shell whoami)
PS2SDK_BUILD_MACHINE := $(shell hostname)
PS2SDK_BUILD_TIME := $(shell date +"%s")
PS2SDK_BUILD_TIME_ISO8601 := $(shell date -d'@'$(PS2SDK_BUILD_TIME) +"%Y-%m-%dT%H:%M:%S%z")
PS2SDK_BUILD_TIME_RFC2822 := $(shell date -d'@'$(PS2SDK_BUILD_TIME) +"%a, %d %b %Y %H:%M:%S %z")

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/tools/Rules.bin.make
include $(PS2SDKSRC)/tools/Rules.make
include $(PS2SDKSRC)/tools/Rules.release

$(TOOLS_OBJS_DIR)gitinfo.h:
	$(MKDIR) -p $(TOOLS_OBJS_DIR) && \
	printf '/* Git info for ps2sdk */\n' > $@ && \
	printf '#ifndef %s\n#define %s "%s"\n#endif\n' PS2SDK_GIT_REMOTE_URL PS2SDK_GIT_REMOTE_URL "$(PS2SDK_GIT_REMOTE_URL)" >> $@ && \
	printf '#ifndef %s\n#define %s "%s"\n#endif\n' PS2SDK_GIT_HASH PS2SDK_GIT_HASH "$(PS2SDK_GIT_HASH)" >> $@ && \
	printf '#ifndef %s\n#define %s "%s"\n#endif\n' PS2SDK_GIT_TIME PS2SDK_GIT_TIME "$(PS2SDK_GIT_TIME)" >> $@ && \
	printf '#ifndef %s\n#define %s "%s"\n#endif\n' PS2SDK_GIT_TIME_ISO8601 PS2SDK_GIT_TIME_ISO8601 "$(PS2SDK_GIT_TIME_ISO8601)" >> $@ && \
	printf '#ifndef %s\n#define %s "%s"\n#endif\n' PS2SDK_GIT_TIME_RFC2822 PS2SDK_GIT_TIME_RFC2822 "$(PS2SDK_GIT_TIME_RFC2822)" >> $@ && \
	printf '#ifndef %s\n#define %s "%s"\n#endif\n' PS2SDK_GIT_TAG PS2SDK_GIT_TAG "$(PS2SDK_GIT_TAG)" >> $@ && \
	printf '/* Git info for ps2sdk end */\n' >> $@

$(TOOLS_OBJS_DIR)buildinfo.h:
	$(MKDIR) -p $(TOOLS_OBJS_DIR) && \
	printf '/* Build info for ps2sdk */\n' > $@ && \
	printf '#ifndef %s\n#define %s "%s"\n#endif\n' PS2SDK_BUILD_PATH PS2SDK_BUILD_PATH "$(PS2SDK_BUILD_PATH)" >> $@ && \
	printf '#ifndef %s\n#define %s "%s"\n#endif\n' PS2SDK_BUILD_USER PS2SDK_BUILD_USER "$(PS2SDK_BUILD_USER)" >> $@ && \
	printf '#ifndef %s\n#define %s "%s"\n#endif\n' PS2SDK_BUILD_MACHINE PS2SDK_BUILD_MACHINE "$(PS2SDK_BUILD_MACHINE)" >> $@ && \
	printf '#ifndef %s\n#define %s "%s"\n#endif\n' PS2SDK_BUILD_TIME PS2SDK_BUILD_TIME "$(PS2SDK_BUILD_TIME)" >> $@ && \
	printf '#ifndef %s\n#define %s "%s"\n#endif\n' PS2SDK_BUILD_TIME_ISO8601 PS2SDK_BUILD_TIME_ISO8601 "$(PS2SDK_BUILD_TIME_ISO8601)" >> $@ && \
	printf '#ifndef %s\n#define %s "%s"\n#endif\n' PS2SDK_BUILD_TIME_RFC2822 PS2SDK_BUILD_TIME_RFC2822 "$(PS2SDK_BUILD_TIME_RFC2822)" >> $@ && \
	printf '/* Build info for ps2sdk end */\n' >> $@

$(TOOLS_OBJS_DIR)main.o: $(TOOLS_OBJS_DIR)gitinfo.h $(TOOLS_OBJS_DIR)buildinfo.h
