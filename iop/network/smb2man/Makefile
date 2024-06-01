# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

DEBUG ?= 0

LIBSMB2 := $(PS2SDKSRC)/common/external_deps/libsmb2

SMB2_CFLAGS += -D__PS2SDK_IOP__ -D__PS2__ -DHAVE_CONFIG_H -DNEED_BE64TOH -DNEED_STRDUP -DNEED_READV -DNEED_WRITEV -DNEED_POLL -DNEED_GETPID -DNEED_RANDOM -DNEED_SRANDOM -DNEED_GETLOGIN_R -D_U_=/**/ -Wall -Os -I. -I$(LIBSMB2)/include -I$(LIBSMB2)/include/ps2 -I$(LIBSMB2)/include/smb2

SMB2_OBJS = aes.o aes128ccm.o alloc.o compat.o dcerpc.o dcerpc-lsa.o dcerpc-srvsvc.o errors.o init.o hmac.o hmac-md5.o krb5-wrapper.o libsmb2.o md4c.o md5.o ntlmssp.o pdu.o sha1.o sha224-256.o \
sha384-512.o smb2-cmd-close.o smb2-cmd-create.o smb2-cmd-echo.o smb2-cmd-error.o smb2-cmd-flush.o smb2-cmd-ioctl.o smb2-cmd-logoff.o smb2-cmd-negotiate.o smb2-cmd-query-directory.o \
smb2-cmd-query-info.o smb2-cmd-read.o smb2-cmd-session-setup.o smb2-cmd-set-info.o smb2-cmd-tree-connect.o smb2-cmd-tree-disconnect.o smb2-cmd-write.o smb2-data-file-info.o smb2-data-filesystem-info.o \
smb2-data-security-descriptor.o smb2-data-reparse-point.o smb2-share-enum.o smb3-seal.o smb2-signing.o socket.o sync.o timestamps.o unicode.o usha.o

IOP_CFLAGS += $(SMB2_CFLAGS) -Wno-error=address

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

$(IOP_OBJS_DIR)aes.o: $(LIBSMB2)/lib/aes.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)aes128ccm.o: $(LIBSMB2)/lib/aes128ccm.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)alloc.o: $(LIBSMB2)/lib/alloc.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)compat.o: $(LIBSMB2)/lib/compat.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)dcerpc.o: $(LIBSMB2)/lib/dcerpc.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)dcerpc-lsa.o: $(LIBSMB2)/lib/dcerpc-lsa.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)dcerpc-srvsvc.o: $(LIBSMB2)/lib/dcerpc-srvsvc.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)errors.o: $(LIBSMB2)/lib/errors.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)init.o: $(LIBSMB2)/lib/init.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)hmac.o: $(LIBSMB2)/lib/hmac.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)hmac-md5.o: $(LIBSMB2)/lib/hmac-md5.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)krb5-wrapper.o: $(LIBSMB2)/lib/krb5-wrapper.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)libsmb2.o: $(LIBSMB2)/lib/libsmb2.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)md4c.o: $(LIBSMB2)/lib/md4c.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)md5.o: $(LIBSMB2)/lib/md5.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)ntlmssp.o: $(LIBSMB2)/lib/ntlmssp.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)pdu.o: $(LIBSMB2)/lib/pdu.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)sha1.o: $(LIBSMB2)/lib/sha1.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)sha224-256.o: $(LIBSMB2)/lib/sha224-256.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)sha384-512.o: $(LIBSMB2)/lib/sha384-512.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-close.o: $(LIBSMB2)/lib/smb2-cmd-close.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-create.o: $(LIBSMB2)/lib/smb2-cmd-create.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-echo.o: $(LIBSMB2)/lib/smb2-cmd-echo.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-error.o: $(LIBSMB2)/lib/smb2-cmd-error.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-flush.o: $(LIBSMB2)/lib/smb2-cmd-flush.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-ioctl.o: $(LIBSMB2)/lib/smb2-cmd-ioctl.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-logoff.o: $(LIBSMB2)/lib/smb2-cmd-logoff.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-negotiate.o: $(LIBSMB2)/lib/smb2-cmd-negotiate.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-query-directory.o: $(LIBSMB2)/lib/smb2-cmd-query-directory.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-query-info.o: $(LIBSMB2)/lib/smb2-cmd-query-info.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-read.o: $(LIBSMB2)/lib/smb2-cmd-read.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-session-setup.o: $(LIBSMB2)/lib/smb2-cmd-session-setup.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-set-info.o: $(LIBSMB2)/lib/smb2-cmd-set-info.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-tree-connect.o: $(LIBSMB2)/lib/smb2-cmd-tree-connect.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-tree-disconnect.o: $(LIBSMB2)/lib/smb2-cmd-tree-disconnect.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-cmd-write.o: $(LIBSMB2)/lib/smb2-cmd-write.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-data-file-info.o: $(LIBSMB2)/lib/smb2-data-file-info.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-data-filesystem-info.o: $(LIBSMB2)/lib/smb2-data-filesystem-info.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-data-security-descriptor.o: $(LIBSMB2)/lib/smb2-data-security-descriptor.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-data-reparse-point.o: $(LIBSMB2)/lib/smb2-data-reparse-point.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-share-enum.o: $(LIBSMB2)/lib/smb2-share-enum.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb3-seal.o: $(LIBSMB2)/lib/smb3-seal.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)smb2-signing.o: $(LIBSMB2)/lib/smb2-signing.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)socket.o: $(LIBSMB2)/lib/socket.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)sync.o: $(LIBSMB2)/lib/sync.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)timestamps.o: $(LIBSMB2)/lib/timestamps.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)unicode.o: $(LIBSMB2)/lib/unicode.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)usha.o: $(LIBSMB2)/lib/usha.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

.NOTPARALLEL:: \
	$(LIBSMB2)
