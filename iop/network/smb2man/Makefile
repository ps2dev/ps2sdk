# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

DEBUG ?= 0

LIBSMB2 := $(PS2SDKSRC)/common/external_deps/libsmb2

SMB2_CFLAGS += -D__ps2sdk_iop__ -D__PS2__ -DHAVE_CONFIG_H -DNEED_BE64TOH -DNEED_STRDUP -DNEED_READV -DNEED_WRITEV -DNEED_POLL -DNEED_GETPID -DNEED_RANDOM -DNEED_SRANDOM -DNEED_GETLOGIN_R -D_U_=/**/ -Wall -Os -I. -I$(LIBSMB2)/include -I$(LIBSMB2)/include/ps2 -I$(LIBSMB2)/include/smb2

SMB2_OBJS = aes.o aes128ccm.o alloc.o compat.o dcerpc.o dcerpc-lsa.o dcerpc-srvsvc.o errors.o init.o hmac.o hmac-md5.o krb5-wrapper.o libsmb2.o md4c.o md5.o ntlmssp.o pdu.o sha1.o sha224-256.o \
sha384-512.o smb2-cmd-close.o smb2-cmd-create.o smb2-cmd-echo.o smb2-cmd-error.o smb2-cmd-flush.o smb2-cmd-ioctl.o smb2-cmd-logoff.o smb2-cmd-negotiate.o smb2-cmd-query-directory.o \
smb2-cmd-query-info.o smb2-cmd-read.o smb2-cmd-session-setup.o smb2-cmd-set-info.o smb2-cmd-tree-connect.o smb2-cmd-tree-disconnect.o smb2-cmd-write.o smb2-data-file-info.o smb2-data-filesystem-info.o \
smb2-data-security-descriptor.o smb2-data-reparse-point.o smb2-share-enum.o smb3-seal.o smb2-signing.o socket.o sync.o timestamps.o unicode.o usha.o

IOP_CFLAGS += $(SMB2_CFLAGS)

ifeq ($(DEBUG),1)
	IOP_CFLAGS += -DDEBUG
endif

IOP_INCS += -I. -I$(PS2SDK)/iop/include -I$(PS2SDKSRC)/iop/tcpip/tcpip/include 

IOP_LDFLAGS += -lgcc

IOP_OBJS = smb2man.o smb2_fio.o imports.o $(SMB2_OBJS)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

$(LIBSMB2):
	$(MAKEREC) $(PS2SDKSRC)/common/external_deps all

vpath %.c $(LIBSMB2)/lib/

$(IOP_OBJS_DIR)%.o: %.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

.NOTPARALLEL:: \
	$(LIBSMB2)
