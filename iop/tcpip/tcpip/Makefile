# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

# DEBUG = 1
PS2IP_DHCP = 1
PS2IP_DNS = 1

LWIP = $(PS2SDKSRC)/common/tcpip/lwip-2.0.0
PS2IP_BASE = $(PS2SDKSRC)/iop/tcpip/tcpip-base

IOP_BIN ?= ps2ip.irx

IOP_CFLAGS += -DLWIP_NOASSERT -DLWIP_COMPAT_MUTEX -DLWIP_COMPAT_MUTEX_ALLOWED

ifeq ($(DEBUG),1)
IOP_CFLAGS += -DDEBUG -DLWIP_DEBUG
endif

# Turns DHCP on.
ifdef PS2IP_DHCP
IOP_CFLAGS += -DPS2IP_DHCP
endif

# Turns DNS on.
ifdef PS2IP_DNS
IOP_CFLAGS += -DPS2IP_DNS
endif

IOP_INCS += -I$(LWIP)/src/include -I$(LWIP)/src/include/ipv4 -I$(PS2IP_BASE)/include

ps2ip_OBJECTS = ps2ip.o sys.o sys_arch.o init.o exports.o imports.o mem.o netif.o pbuf.o stats.o tcp_in.o tcp_out.o udp.o memp.o tcp.o etharp.o ethernet.o raw.o def.o timeouts.o

ifdef PS2IP_DHCP
ps2ip_OBJECTS += dhcp.o
endif

ifdef PS2IP_DNS
ps2ip_OBJECTS += dns.o netdb.o
endif

ps2api_OBJECTS = api_lib.o api_msg.o err.o sockets.o tcpip.o netbuf.o

ps2api_IPV4 = icmp.o ip.o ip4.o ip4_addr.o ip4_frag.o inet_chksum.o

IOP_OBJS = $(ps2ip_OBJECTS) $(ps2api_OBJECTS) $(ps2api_IPV4)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.bin.make
include $(PS2SDKSRC)/iop/Rules.make
include $(PS2SDKSRC)/iop/Rules.release

$(IOP_OBJS_DIR)def.o: $(LWIP)/src/core/def.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)init.o: $(LWIP)/src/core/init.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)api_lib.o: $(LWIP)/src/api/api_lib.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)api_msg.o: $(LWIP)/src/api/api_msg.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)icmp.o: $(LWIP)/src/core/ipv4/icmp.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)ip.o: $(LWIP)/src/core/ip.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)ip4.o: $(LWIP)/src/core/ipv4/ip4.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)ip4_addr.o: $(LWIP)/src/core/ipv4/ip4_addr.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)ip4_frag.o: $(LWIP)/src/core/ipv4/ip4_frag.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)err.o: $(LWIP)/src/api/err.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)sockets.o: $(LWIP)/src/api/sockets.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)tcpip.o: $(LWIP)/src/api/tcpip.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)inet_chksum.o: $(LWIP)/src/core/inet_chksum.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)mem.o: $(LWIP)/src/core/mem.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)memp.o: $(LWIP)/src/core/memp.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)netbuf.o: $(LWIP)/src/api/netbuf.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)netif.o: $(LWIP)/src/core/netif.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)pbuf.o: $(LWIP)/src/core/pbuf.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)stats.o: $(LWIP)/src/core/stats.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)tcp.o: $(LWIP)/src/core/tcp.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)tcp_in.o: $(LWIP)/src/core/tcp_in.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)tcp_out.o: $(LWIP)/src/core/tcp_out.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)sys.o: $(LWIP)/src/core/sys.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)raw.o: $(LWIP)/src/core/raw.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)timeouts.o: $(LWIP)/src/core/timeouts.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)etharp.o: $(LWIP)/src/core/ipv4/etharp.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)ethernet.o: $(LWIP)/src/netif/ethernet.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

ifdef PS2IP_DHCP
$(IOP_OBJS_DIR)dhcp.o: $(LWIP)/src/core/ipv4/dhcp.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@
endif

ifdef PS2IP_DNS
$(IOP_OBJS_DIR)dns.o: $(LWIP)/src/core/dns.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)netdb.o: $(LWIP)/src/api/netdb.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@
endif

$(IOP_OBJS_DIR)udp.o: $(LWIP)/src/core/udp.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@

$(IOP_OBJS_DIR)sys_arch.o: $(PS2IP_BASE)/sys_arch.c
	$(IOP_CC) $(IOP_CFLAGS) -c $< -o $@
