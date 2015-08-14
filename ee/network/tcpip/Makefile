# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------

EE_OBJS_DIR = obj/
EE_LIB_DIR = lib/
EE_SRC_DIR = src/
EE_INC_DIR = include/

#DEBUG = 1
PS2IP_DHCP=1
PS2IP_DNS=1

ifeq ($(DEBUG),1)
DEBUG_FLAGS = -DDEBUG -DLWIP_DEBUG
endif

# Change this to point to the version of LWIP this is being compiled with.
LWIP=$(PS2SDKSRC)/common/tcpip/lwip-1.4.1

ps2api_OBJECTS = api_lib.o api_msg.o api_netbuf.o err.o sockets.o tcpip.o
ps2api_IPV4 = icmp.o ip.o ip_addr.o ip_frag.o inet.o inet_chksum.o
ps2ip_OBJECTS = sys.o lwip_init.o mem.o netif.o pbuf.o stats.o tcp_in.o tcp_out.o udp.o memp.o tcp.o etharp.o raw.o def.o timers.o $(ps2api_IPV4) $(ps2api_OBJECTS)

ifdef PS2IP_DHCP
ps2ip_OBJECTS+= dhcp.o
endif

ifdef PS2IP_DNS
ps2ip_OBJECTS+= dns.o netdb.o
endif

# Turns DHCP on.

ifdef PS2IP_DHCP
EE_CFLAGS += -DPS2IP_DHCP -DLWIP_DHCP
endif

# Turns DNS on.
ifdef PS2IP_DNS
EE_CFLAGS += -DPS2IP_DNS
endif

EE_INCS += -I$(LWIP)/src/include -I$(LWIP)/src/include/ipv4 -Isrc/include
EE_CFLAGS += -Wall -fno-builtin -DLWIP_NOASSERT -DLWIP_COMPAT_MUTEX $(DEBUG_FLAGS) -mno-gpopt -O2 -G0 -mno-check-zero-division

EE_LIB = $(EE_LIB_DIR)libps2ip.a
EE_OBJS  = ps2ip.o sys_arch.o $(ps2ip_OBJECTS) erl-support.o
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

all: $(EE_OBJS_DIR) $(EE_LIB_DIR) $(EE_LIB)

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_LIB_DIR)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/ee/Rules.make
include $(PS2SDKSRC)/ee/Rules.release

obj/def.o: $(LWIP)/src/core/def.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/lwip_init.o: $(LWIP)/src/core/init.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/loopif.o: $(LWIP)/src/netif/loopif.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/api_lib.o: $(LWIP)/src/api/api_lib.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/api_msg.o: $(LWIP)/src/api/api_msg.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/api_netbuf.o: $(LWIP)/src/api/netbuf.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/icmp.o: $(LWIP)/src/core/ipv4/icmp.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/ip.o: $(LWIP)/src/core/ipv4/ip.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/ip_addr.o: $(LWIP)/src/core/ipv4/ip_addr.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/ip_frag.o: $(LWIP)/src/core/ipv4/ip_frag.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/err.o: $(LWIP)/src/api/err.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/sockets.o: $(LWIP)/src/api/sockets.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/tcpip.o: $(LWIP)/src/api/tcpip.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/inet.o: $(LWIP)/src/core/ipv4/inet.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/inet_chksum.o: $(LWIP)/src/core/ipv4/inet_chksum.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/mem.o: $(LWIP)/src/core/mem.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/memp.o: $(LWIP)/src/core/memp.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/netif.o: $(LWIP)/src/core/netif.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/pbuf.o: $(LWIP)/src/core/pbuf.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/stats.o: $(LWIP)/src/core/stats.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/tcp.o: $(LWIP)/src/core/tcp.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/tcp_in.o: $(LWIP)/src/core/tcp_in.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/tcp_out.o: $(LWIP)/src/core/tcp_out.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/sys.o: $(LWIP)/src/core/sys.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/raw.o: $(LWIP)/src/core/raw.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/timers.o: $(LWIP)/src/core/timers.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/etharp.o: $(LWIP)/src/netif/etharp.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

ifdef PS2IP_DHCP
obj/dhcp.o: $(LWIP)/src/core/dhcp.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
endif

ifdef PS2IP_DNS
obj/dns.o: $(LWIP)/src/core/dns.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

obj/netdb.o: $(LWIP)/src/api/netdb.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
endif

obj/udp.o: $(LWIP)/src/core/udp.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

