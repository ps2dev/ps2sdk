/*	If changes are made to this file, please update the defined
	values at the bottom of common/include/tcpip.h too.	*/

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#define LWIP_TIMEVAL_PRIVATE 0

/* ---------- Thread options ---------- */
/**
 * DEFAULT_THREAD_STACKSIZE: The stack size used by any other lwIP thread
 * spawned via sys_thread_new(). In our build that's just the tcpip thread.
 *
 * With LWIP_TCPIP_CORE_LOCKING=1 the deep socket-API call chains run on
 * the calling app thread, not on the tcpip thread; the tcpip thread itself
 * only dispatches timer callbacks and the occasional tcpip_callback (e.g.
 * link up/down). Worst-case chain on the tcpip thread is roughly
 *
 *   tcpip_thread (56) -> sys_check_timeouts (32) -> lwip_cyclic_timer (32)
 *     -> tcp_slowtmr (72) or dhcp_fine_tmr -> ~150-200 of inner work
 *   ~= 350-450 bytes + register-save overhead.
 *
 * 0x600 (1.5 KB) is the historical 2.0.3 value and matches the call-chain
 * profile under LWIP_TCPIP_CORE_LOCKING=1. ~2x margin over the measured
 * worst case.
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

/*
   ------------------------------------
   ---------- Memory options ----------
   ------------------------------------
*/
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT		4

/**
 * LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT==1: make mem_free() callable from
 * any context (ISR/disabled-interrupt) by using SYS_ARCH_PROTECT instead of
 * a mutex for critical regions. Required on IOP because SMAP RX interrupts
 * invoke pbuf_free() in interrupt-disabled context; taking a mutex there
 * would violate the critical section (same effect as ps2dev/lwip's mem.c
 * patch, but achieved via upstream lwipopts instead of patching lwIP).
 */
#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT	1

/**
 * MEM_SIZE: the size of the heap memory. If the application will send
 * a lot of data that needs to be copied, this should be set high.
 */
/*	SP193: setting this too low may cause tcp_write() to fail when it tries to allocate from PBUF_RAM!
	Up to TCP_SND_BUF * 2 segments may be transmitted at once, thanks to Nagle and Delayed Ack. */
#define MEM_SIZE		(TCP_SND_BUF * 2)

/*
   -----------------------------------------------
   ---------- IP options -------------------------
   -----------------------------------------------
*/
/**
 * IP_REASSEMBLY==1: Reassemble incoming fragmented IP packets.
 * Disabled: PS2 networking targets a local LAN with MTU=1500, and
 * TCP negotiates MSS=1460 so fragments never arise in the common
 * case. Frees MEMP_NUM_REASSDATA / MEMP_NUM_FRAG_PBUF pool entries.
 */
#define IP_REASSEMBLY		0

/**
 * IP_FRAG==1: Fragment outgoing IP packets if their size exceeds MTU.
 * Disabled: TCP_MSS=1460 ensures we never exceed Ethernet MTU=1500.
 */
#define IP_FRAG			0

/*
   ------------------------------------------------
   ---------- Internal Memory Pool Sizes ----------
   ------------------------------------------------
*/
/**
 * MEMP_NUM_TCPIP_MSG_INPKT: the number of struct tcpip_msg, which are used
 * for incoming packets.
 * SP193: this should be around the size of the TCP window because the
 * TCPIP thread may take a while to execute (non-preemptive multitasking),
 * otherwise incoming frames may get dropped.
 */
#define MEMP_NUM_TCPIP_MSG_INPKT	24

/**
 * MEMP_NUM_NETCONN: the number of struct netconns.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#define MEMP_NUM_NETCONN       (MEMP_NUM_TCP_PCB+MEMP_NUM_UDP_PCB)

/**
 * PBUF_POOL_SIZE: the number of buffers in the pbuf pool.
 */
#define PBUF_POOL_SIZE		32	//SP193: should be at least ((TCP_WND/PBUF_POOL_BUFSIZE)+1). But that is too small to handle simultaneous connections.

/**
 * LWIP_TCPIP_CORE_LOCKING==1: matches lwIP 2.2.1's upstream default. Socket
 * and netconn API calls take the core mutex on the calling app thread and
 * run synchronously, instead of round-tripping through the tcpip thread's
 * mailbox. Saves a context switch + sem wait per API call. lwIP releases
 * the core lock before any blocking I/O wait (mbox_fetch on connection
 * mboxes), so the tcpip thread and SMAP RX can still run to deliver data.
 */
#define LWIP_TCPIP_CORE_LOCKING		1

/**
 * LWIP_TCPIP_CORE_LOCKING_INPUT==1: tcpip_input() takes the core mutex
 * directly instead of allocating a message. Safe here because the netif
 * input callback runs in IntrHandlerThread (smap.c) — a normal thread,
 * not interrupt context — so the lock acquire is allowed.
 */
#define LWIP_TCPIP_CORE_LOCKING_INPUT	1

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

/* ---------- DHCP options ---------- */
#ifdef PS2IP_DHCP
/**
 * LWIP_DHCP==1: Enable DHCP module.
 */
#define LWIP_DHCP		1
#endif

/**
 * LWIP_DHCP_DOES_ACD_CHECK==0: skip RFC 5227 Address Conflict Detection on
 * the DHCP-offered address (replaces the pre-2.2.0 DHCP_DOES_ARP_CHECK).
 * PS2 networking targets a controlled LAN; the saved code+timer/RAM beats
 * guarding against a vanishingly unlikely IP collision. Combined with
 * LWIP_AUTOIP=0 (default) this lets LWIP_ACD default to 0 too.
 */
#define LWIP_DHCP_DOES_ACD_CHECK	0
#define LWIP_ACD			0

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
