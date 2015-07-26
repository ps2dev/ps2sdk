/*	If changes are made to this file, please update the defined
	values at the bottom of common/include/tcpip.h too.	*/

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#ifdef DEBUG
#define LWIP_PLATFORM_DIAG(args...) printf(args)
#define LWIP_PLATFORM_ASSERT(args...) printf(args)
#else
#define LWIP_PLATFORM_DIAG(args...)
#define LWIP_PLATFORM_ASSERT(args...)
#endif

/**
 * NO_SYS==1: Provides VERY minimal functionality. Otherwise,
 * use lwIP facilities.
 */
#define NO_SYS		0

/* ---------- Thread options ---------- */
/**
 * DEFAULT_THREAD_STACKSIZE: The stack size used by any other lwIP thread.
 * The stack size value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#define DEFAULT_THREAD_STACKSIZE	0x600

/**
 * DEFAULT_THREAD_PRIO: The priority assigned to any other lwIP thread.
 * The priority value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#define DEFAULT_THREAD_PRIO		0x18

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
#define MEM_ALIGNMENT		4

/*
   ------------------------------------------------
   ---------- Internal Memory Pool Sizes ----------
   ------------------------------------------------
*/
/**
 * MEMP_NUM_NETCONN: the number of struct netconns.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#define MEMP_NUM_NETCONN                10	//SP193: find it weird that the default is only 4, while the maximum number of simultaneous TCP and UDP connections in total is 9.

/**
 * PBUF_POOL_SIZE: the number of buffers in the pbuf pool.
 */
#define PBUF_POOL_SIZE		25	//SP193: should be at least ((TCP_WND/PBUF_POOL_BUFSIZE)+1). But that is too small to handle simultaneous connections.

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
#define TCP_SND_BUF             (TCP_MSS*4)

/* TCP receive window. */
#define TCP_WND                 32768

/* ---------- ARP options ---------- */
/**
 * ETHARP_TRUST_IP_MAC==1: Incoming IP packets cause the ARP table to be
 * updated with the source MAC and IP addresses supplied in the packet.
 * You may want to disable this if you do not trust LAN peers to have the
 * correct addresses, or as a limited approach to attempt to handle
 * spoofing. If disabled, lwIP will need to make a new ARP request if
 * the peer is not already in the ARP table, adding a little latency.
 * The peer *is* in the ARP table if it requested our address before.
 * Also notice that this slows down input processing of every IP packet!
 */
#define ETHARP_TRUST_IP_MAC	1

/** ETH_PAD_SIZE: number of bytes added before the ethernet header to ensure
 * alignment of payload after that header. Since the header is 14 bytes long,
 * without this padding e.g. addresses in the IP header will not be aligned
 * on a 32-bit boundary, so setting this to 2 can speed up 32-bit-platforms.
 */
//#define ETH_PAD_SIZE	2	//SP193: Why doesn't this work correctly?

/* ---------- DHCP options ---------- */
#ifdef PS2IP_DHCP
/**
 * LWIP_DHCP==1: Enable DHCP module.
 */
#define LWIP_DHCP		1
#endif

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
   ---------------------------------
   ---------- RAW options ----------
   ---------------------------------
*/
/**
 * LWIP_RAW==1: Enable application layer to hook into the IP layer itself.
 */
#define LWIP_RAW	0

/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/
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
/**
 * LWIP_POSIX_SOCKETS_IO_NAMES==1: Enable POSIX-style sockets functions names.
 * Disable this option if you use a POSIX operating system that uses the same
 * names (read, write & close). (only used if you use sockets.c)
 */
#define LWIP_POSIX_SOCKETS_IO_NAMES	0

#endif /* __LWIPOPTS_H__ */
