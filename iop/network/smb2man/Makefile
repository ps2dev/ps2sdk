# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

DEBUG ?= 0

IOP_CFLAGS += -DPS2_IOP_PLATFORM -DHAVE_CONFIG_H -DPS2SDK_IOP -DNEED_BE64TOH -DNEED_STRDUP -DNEED_READV -DNEED_WRITEV -DNEED_POLL -DNEED_RANDOM -DNEED_SRANDOM -DNEED_GETLOGIN_R -DNEED_GETPID -D_U_=/**/

ifeq ($(DEBUG),1)
	IOP_CFLAGS += -DDEBUG
endif

IOP_INCS += -I. -I$(PS2SDK)/iop/include -I$(PS2SDKSRC)/iop/tcpip/tcpip/include -Isrc/lib -Isrc/lib/ps2 -Isrc/lib/smb2

IOP_LDFLAGS += -L $(PS2SDK)/iop/lib -lgcc

SMB2_OBJS = lib/aes.o lib/aes128ccm.o lib/alloc.o lib/compat.o lib/dcerpc.o lib/dcerpc-lsa.o lib/dcerpc-srvsvc.o lib/errors.o lib/init.o lib/hmac.o lib/hmac-md5.o lib/krb5-wrapper.o lib/libsmb2.o lib/md4c.o lib/md5.o lib/ntlmssp.o lib/pdu.o lib/sha1.o lib/sha224-256.o \
lib/sha384-512.o lib/smb2-cmd-close.o lib/smb2-cmd-create.o lib/smb2-cmd-echo.o lib/smb2-cmd-error.o lib/smb2-cmd-flush.o lib/smb2-cmd-ioctl.o lib/smb2-cmd-logoff.o lib/smb2-cmd-negotiate.o lib/smb2-cmd-query-directory.o \
lib/smb2-cmd-query-info.o lib/smb2-cmd-read.o lib/smb2-cmd-session-setup.o lib/smb2-cmd-set-info.o lib/smb2-cmd-tree-connect.o lib/smb2-cmd-tree-disconnect.o lib/smb2-cmd-write.o lib/smb2-data-file-info.o lib/smb2-data-filesystem-info.o \
lib/smb2-data-security-descriptor.o lib/smb2-data-reparse-point.o lib/smb2-share-enum.o lib/smb3-seal.o lib/smb2-signing.o lib/socket.o lib/sync.o lib/timestamps.o lib/unicode.o lib/usha.o

IOP_OBJS = $(SMB2_OBJS) smb2man.o smb2_fio.o imports.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
