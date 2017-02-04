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

# Change this to point to the version of LWIP this is being compiled with.
LWIP = $(PS2SDKSRC)/common/tcpip/lwip-2.0.0

EE_LIB = libps2ip.a

EE_CFLAGS += -Wall -fno-builtin -DLWIP_NOASSERT -DLWIP_COMPAT_MUTEX -DLWIP_COMPAT_MUTEX_ALLOWED -mno-gpopt -O2 -G0 -mno-check-zero-division

ifeq ($(DEBUG),1)
EE_CFLAGS += -DDEBUG -DLWIP_DEBUG
endif

# Turns DHCP on.
ifdef PS2IP_DHCP
EE_CFLAGS += -DPS2IP_DHCP -DLWIP_DHCP
endif

# Turns DNS on.
ifdef PS2IP_DNS
EE_CFLAGS += -DPS2IP_DNS
endif

EE_INCS += -I$(LWIP)/src/include -I$(LWIP)/src/include/ipv4

ps2api_OBJECTS = api_lib.o api_msg.o api_netbuf.o err.o sockets.o tcpip.o
ps2api_IPV4 = icmp.o ip.o ip4.o ip4_addr.o ip4_frag.o inet_chksum.o
ps2ip_OBJECTS = sys.o lwip_init.o mem.o netif.o pbuf.o stats.o tcp_in.o tcp_out.o udp.o memp.o tcp.o ethernet.o etharp.o raw.o def.o timeouts.o $(ps2api_IPV4) $(ps2api_OBJECTS)

ifdef PS2IP_DHCP
ps2ip_OBJECTS += dhcp.o
endif

ifdef PS2IP_DNS
ps2ip_OBJECTS += dns.o netdb.o
endif

EE_OBJS = ps2ip.o sys_arch.o $(ps2ip_OBJECTS) erl-support.o

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.lib.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

$(EE_OBJS_DIR)def.o: $(LWIP)/src/core/def.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)lwip_init.o: $(LWIP)/src/core/init.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)api_lib.o: $(LWIP)/src/api/api_lib.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)api_msg.o: $(LWIP)/src/api/api_msg.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)api_netbuf.o: $(LWIP)/src/api/netbuf.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)icmp.o: $(LWIP)/src/core/ipv4/icmp.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)ip.o: $(LWIP)/src/core/ip.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)ip4.o: $(LWIP)/src/core/ipv4/ip4.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)ip4_addr.o: $(LWIP)/src/core/ipv4/ip4_addr.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)ip4_frag.o: $(LWIP)/src/core/ipv4/ip4_frag.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)err.o: $(LWIP)/src/api/err.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)sockets.o: $(LWIP)/src/api/sockets.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)tcpip.o: $(LWIP)/src/api/tcpip.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)inet_chksum.o: $(LWIP)/src/core/inet_chksum.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)mem.o: $(LWIP)/src/core/mem.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)memp.o: $(LWIP)/src/core/memp.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)netif.o: $(LWIP)/src/core/netif.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)pbuf.o: $(LWIP)/src/core/pbuf.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)stats.o: $(LWIP)/src/core/stats.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)tcp.o: $(LWIP)/src/core/tcp.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)tcp_in.o: $(LWIP)/src/core/tcp_in.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)tcp_out.o: $(LWIP)/src/core/tcp_out.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)sys.o: $(LWIP)/src/core/sys.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)raw.o: $(LWIP)/src/core/raw.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)timeouts.o: $(LWIP)/src/core/timeouts.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)etharp.o: $(LWIP)/src/core/ipv4/etharp.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)ethernet.o: $(LWIP)/src/netif/ethernet.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

ifdef PS2IP_DHCP
$(EE_OBJS_DIR)dhcp.o: $(LWIP)/src/core/ipv4/dhcp.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
endif

ifdef PS2IP_DNS
$(EE_OBJS_DIR)dns.o: $(LWIP)/src/core/dns.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)netdb.o: $(LWIP)/src/api/netdb.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
endif

$(EE_OBJS_DIR)udp.o: $(LWIP)/src/core/udp.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

