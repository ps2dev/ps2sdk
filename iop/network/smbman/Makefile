#  _____     ___ ____ 
#   ____|   |    ____|      PS2 Open Source Project
#  |     ___|   |____       
#  
#--------------------------------------------------------------------------
#
#    makefile
#
#--------------------------------------------------------------------------

IOP_OBJS_DIR = obj/
IOP_BIN_DIR = bin/
IOP_SRC_DIR = src/
IOP_INC_DIR = 

IOP_BIN  = bin/smbman.irx
IOP_OBJS = obj/smbman.o obj/smb_fio.o obj/smb.o obj/poll.o obj/auth.o \
	obj/des.o obj/md4.o obj/imports.o obj/exports.o

IOP_CFLAGS  += -Wall -fno-builtin -D_IOP #-DDEBUG
IOP_LDFLAGS += -s
IOP_INCS += -I$(PS2SDKSRC)/iop/tcpip/tcpip/include

all: $(IOP_OBJS_DIR) $(IOP_BIN_DIR) $(IOP_BIN)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_BIN_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release
