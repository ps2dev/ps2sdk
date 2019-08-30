/*	If changes are made to this file, please update the defined
	values at the bottom of common/include/tcpip.h too.	*/

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

/**
 * NO_SYS==1: Provides VERY minimal functionality. Otherwise,
 * use lwIP facilities.
 */
#define NO_SYS		0

#define LWIP_TIMEVAL_PRIVATE 0

/* ---------- Thread options ---------- */
/**
 * DEFAULT_THREAD_STACKSIZE: The stack size used by any other lwIP thread.
 * The stack size value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#define DEFAULT_THREAD_STACKSIZE	0x1000

/**
 * DEFAULT_THREAD_PRIO: The priority assigned to any other lwIP thread.
 * The priority value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#define DEFAULT_THREAD_PRIO		0x58

/**
 * TCPIP_THREAD_STACKSIZE: The stack size used by the main tcpip thread.
 * The stack size value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#define TCPIP_THREAD_STACKSIZE		DEFAULT_THREAD_STACKSIZE

/**
 * TCPIP_THREAD_PRIO: The priority assigned to the main tcpip thread.
 * The priority value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#define TCPIP_THREAD_PRIO		DEFAULT_THREAD_PRIO

/**
 * SLIP_THREAD_STACKSIZE: The stack size used by the slipif_loop thread.
 * The stack size value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#define SLIPIF_THREAD_STACKSIZE		DEFAULT_THREAD_STACKSIZE

/**
 * SLIPIF_THREAD_PRIO: The priority assigned to the slipif_loop thread.
 * The priority value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#define SLIPIF_THREAD_PRIO		DEFAULT_THREAD_PRIO

/**
 * PPP_THREAD_STACKSIZE: The stack size used by the pppInputThread.
 * The stack size value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#define PPP_THREAD_STACKSIZE		DEFAULT_THREAD_STACKSIZE

/**
 * PPP_THREAD_PRIO: The priority assigned to the pppInputThread.
 * The priority value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#define PPP_THREAD_PRIO			DEFAULT_THREAD_PRIO

/*
   ------------------------------------
   ---------- Memory options ----------
   ------------------------------------
*/
/**
 * MEM_LIBC_MALLOC==1: Use malloc/free/realloc provided by your C-library
 * instead of the lwip internal allocator. Can save code size if you
 * already use it.
 */
#define MEM_LIBC_MALLOC		1

/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT		64		//SP193: must be 64, to deal with the EE cache design.

/**
 * MEM_SIZE: the size of the heap memory. If the application will send
 * a lot of data that needs to be copied, this should be set high.
 */
/*	SP193: setting this too low may cause tcp_write() to fail when it tries to allocate from PBUF_RAM!
	Up to TCP_SND_BUF * 2 segments may be transmitted at once, thanks to Nagle and Delayed Ack. */
#define MEM_SIZE		(TCP_SND_BUF * 2)

/*
   ------------------------------------------------
   ---------- Internal Memory Pool Sizes ----------
   ------------------------------------------------
*/
/**
 * MEMP_NUM_NETCONN: the number of struct netconns.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#define MEMP_NUM_NETCONN       (MEMP_NUM_TCP_PCB+MEMP_NUM_UDP_PCB)

/**
 * PBUF_POOL_SIZE: the number of buffers in the pbuf pool.
 */
//SP193: should be at least ((TCP_WND/PBUF_POOL_BUFSIZE)+1). But that is too small to accommodate data not accepted by the application layer and multiple connections.
//NETMAN will also always have PBUFs reserved for incoming frames (currently 64).
#define PBUF_POOL_SIZE			128

/**
 * MEMP_NUM_TCPIP_MSG_INPKT: the number of struct tcpip_msg, which are used
 * for incoming packets.
 * (only needed if you use tcpip.c)
 */
//SP193: this should be around the size of the TCP window because the TCPIP thread may take a while to execute (non-preemptive multitasking), otherwise incoming frames may get dropped.
#ifndef LWIP_TCPIP_CORE_LOCKING_INPUT
#define MEMP_NUM_TCPIP_MSG_INPKT	50
#endif

/**
 * MEMP_NUM_TCPIP_MSG_API: the number of struct tcpip_msg, which are used
 * for callback/timeout API communication.
 * (only needed if you use tcpip.c)
 */
//SP193: this should be around the size of MEM_SIZE (in PBUFs), to prevent transmissions from being potentially being dropped.
#define MEMP_NUM_TCPIP_MSG_API		50

/**
 * MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP segments.
 * (requires the LWIP_TCP option)
 */
#define MEMP_NUM_TCP_SEG		TCP_SND_QUEUELEN

/**
 * LWIP_TCPIP_CORE_LOCKING_INPUT: when LWIP_TCPIP_CORE_LOCKING is enabled,
 * this lets tcpip_input() grab the mutex for input packets as well,
 * instead of allocating a message and passing it to tcpip_thread.
 *
 * ATTENTION: this does not work when tcpip_input() is called from
 * interrupt context!
 */
#define LWIP_TCPIP_CORE_LOCKING_INPUT	1

/** SYS_LIGHTWEIGHT_PROT
 * define SYS_LIGHTWEIGHT_PROT in lwipopts.h if you want inter-task protection
 * for certain critical regions during buffer allocation, deallocation and
 * memory allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT	1

/*
   ---------------------------------
   ---------- TCP options ----------
   ---------------------------------
*/
/* TCP Maximum segment size. */
#define TCP_MSS                 1460

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             (TCP_MSS*32)

/* TCP receive window. */
#define TCP_WND                 65535

/* ---------- DHCP options ---------- */
#ifdef PS2IP_DHCP
/**
 * LWIP_DHCP==1: Enable DHCP module.
 */
#define LWIP_DHCP		1
#endif

/**
 * DHCP_DOES_ARP_CHECK==1: Do an ARP check on the offered address.
 */
#define DHCP_DOES_ARP_CHECK	0	//Don't do the ARP check because an IP address would be first required.

/**
 * LWIP_DHCP_CHECK_LINK_UP==1: dhcp_start() only really starts if the netif has
 * NETIF_FLAG_LINK_UP set in its flags. As this is only an optimization and
 * netif drivers might not set this flag, the default is off. If enabled,
 * netif_set_link_up() must be called to continue dhcp starting.
 */
#define LWIP_DHCP_CHECK_LINK_UP	1

/*
   ----------------------------------
   ---------- DNS options -----------
   ----------------------------------
*/
/**
 * LWIP_DNS==1: Turn on DNS module. UDP must be available for DNS
 * transport.
 */
#ifdef PS2IP_DNS
#define LWIP_DNS	1
#endif

/* ---------- Statistics options ---------- */
/**
 * LWIP_STATS==1: Enable statistics collection in lwip_stats.
 */
#define LWIP_STATS	0

/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/
//Rely on the Ethernet checksums to reduce the number of checksum compuations. If you require any of these, re-enable them.
/**
 * CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.
 */
#define CHECKSUM_CHECK_IP	0

/**
 * CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.
 */
#define CHECKSUM_CHECK_UDP	0

/**
 * CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.
 */
#define CHECKSUM_CHECK_TCP	0

/**
 * CHECKSUM_CHECK_ICMP==1: Check checksums in software for incoming ICMP packets.
 */
#define CHECKSUM_CHECK_ICMP	0

/**
 * CHECKSUM_CHECK_ICMP6==1: Check checksums in software for incoming ICMPv6 packets
 */
#define CHECKSUM_CHECK_ICMP6	0

/**
 * LWIP_CHECKSUM_ON_COPY==1: Calculate checksum when copying data from
 * application buffers to pbufs.
 */
#define LWIP_CHECKSUM_ON_COPY	1

/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/
/* LWIP_SOCKET_SET_ERRNO==1: Set errno when socket functions cannot complete
 * successfully, as required by POSIX. Default is POSIX-compliant.
 */
#define LWIP_SOCKET_SET_ERRNO	0
/**
 * LWIP_POSIX_SOCKETS_IO_NAMES==1: Enable POSIX-style sockets functions names.
 * Disable this option if you use a POSIX operating system that uses the same
 * names (read, write & close). (only used if you use sockets.c)
 */
#define LWIP_POSIX_SOCKETS_IO_NAMES	0

/*
   ----------------------------------
   ---------- DNS options -----------
   ----------------------------------
*/
/** LWIP_DNS_SECURE: controls the security level of the DNS implementation
 * Use all DNS security features by default.
 * This is overridable but should only be needed by very small targets
 * or when using against non standard DNS servers. */
#define LWIP_DNS_SECURE	0

/*
   ------------------------------------------------
   ---------- Network Interfaces options ----------
   ------------------------------------------------
*/
/**
 * LWIP_NETIF_TX_SINGLE_PBUF: if this is set to 1, lwIP tries to put all data
 * to be sent into one single pbuf. This is for compatibility with DMA-enabled
 * MACs that do not support scatter-gather.
 * Beware that this might involve CPU-memcpy before transmitting that would not
 * be needed without this flag! Use this only if you need to!
 *
 * @todo: TCP and IP-frag do not work with this, yet:
 */
#define LWIP_NETIF_TX_SINGLE_PBUF             1

#endif /* __LWIPOPTS_H__ */
