/*	If changes are made to this file, please update the defined
	values at the bottom of common/include/tcpip.h too.	*/

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

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

/*
   ------------------------------------
   ---------- Memory options ----------
   ------------------------------------
*/
/**
 * MEM_LIBC_MALLOC==1: use libc's malloc/free for lwIP's mem_malloc /
 * mem_free (which serve PBUF_RAM allocations and a handful of other
 * sites — DHCP options, ARP, DNS, slip/ppp/zepif which we don't build).
 * Internally lwIP's pbuf_alloc(PBUF_RAM) computes the payload pointer
 * with `LWIP_MEM_ALIGN(p + SIZEOF_STRUCT_PBUF + offset)`, which only
 * stays inside the allocated chunk when 'p' is already MEM_ALIGNMENT-
 * aligned. That is why MEM_ALIGNMENT must match the underlying
 * allocator's alignment — see MEM_ALIGNMENT below.
 */
#define MEM_LIBC_MALLOC		1

/* MEM_ALIGNMENT: must equal the alignment libc's malloc returns,
   because pbuf_alloc(PBUF_RAM) computes
     payload = LWIP_MEM_ALIGN(p + SIZEOF_STRUCT_PBUF + offset)
   inside an allocation sized assuming p is already MEM_ALIGNMENT-
   aligned. If MEM_ALIGNMENT > newlib's alignment the payload pointer
   overruns the chunk and corrupts the next-chunk header on the
   freelist (TLB misses inside _malloc_r / _free_r).

   16 is the contract baked into the toolchain configuration. See
   newlib/newlib/configure.host:

       mips64r5900*)
           machine_dir=r5900
           newlib_cflags="${newlib_cflags} -DMALLOC_ALIGNMENT=16"
           ;;

   so for the mips64r5900el-ps2-elf target newlib is built with
   -DMALLOC_ALIGNMENT=16, which sets _mallocr.c's MALLOC_ALIGNMENT
   directly (overriding the SIZE_SZ-derived default of 8). 16 is also
   what samples/malloc_stress observes empirically.

   The historical value here was 64 with a comment about "the EE cache
   design" (SP193). That was over-cautious: PBUF_POOL pbufs (the only
   ones touched by IOP->EE DMA + cache invalidate) come from memp's
   static pools and are always 64-byte aligned regardless of
   MEM_ALIGNMENT; PBUF_RAM pbufs (TX, ARP, DNS, ...) only see DMA
   writeback, where misalignment harmlessly over-flushes extra cache
   lines. The IOP-side lwipopts has used MEM_ALIGNMENT=4 forever for
   the same reason. */
#define MEM_ALIGNMENT		16

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
 * MEMP_NUM_TCP_PCB: the number of simultaneously active TCP connections.
 * The default of 5 is too small for an HTTP server: each completed request
 * leaves the closing-side pcb in TIME_WAIT for 2*MSL (~60 s) holding a slot,
 * so after a handful of fast back-to-back requests the pool fills up and
 * accept() / connect() start failing with EHOSTUNREACH until slots free.
 * 32 gives enough headroom for sustained traffic plus the in-flight SYN_RECV
 * state for a 20-burst.
 */
#define MEMP_NUM_TCP_PCB       32

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
 * SP193: this should be around the size of the TCP window because the
 * TCPIP thread may take a while to execute (non-preemptive multitasking),
 * otherwise incoming frames may get dropped.
 */
#define MEMP_NUM_TCPIP_MSG_INPKT	50

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
 * LWIP_TCPIP_CORE_LOCKING==1: matches lwIP 2.2.1's upstream default. With
 * LWIP_COMPAT_MUTEX in arch/cc.h the core lock is a binary semaphore taken
 * directly on the calling app thread for socket/netconn API calls; lwIP
 * releases it before any blocking I/O wait so the tcpip thread + netif
 * input continue to make progress. Saves a context switch + sem wait per
 * API call versus the message-passing alternative.
 */
#define LWIP_TCPIP_CORE_LOCKING		1

/**
 * LWIP_TCPIP_CORE_LOCKING_INPUT==1: tcpip_input() takes the core mutex
 * directly instead of allocating a message. Safe here because the netif
 * input callback runs in a regular thread, not interrupt context.
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

/* LWIP_DHCP_DOES_ACD_CHECK / LWIP_ACD left at upstream defaults (=1 when
 * LWIP_DHCP=1) so the RFC 5227 Address Conflict Detection probe runs on
 * any DHCP-offered IP. EE applications run on user home networks where
 * IP collisions are a real (if uncommon) failure mode; the few KB of
 * code and ~1-2 s extra DHCP-bind delay are worth the robustness. The
 * IOP lwIP build forces both off because ps2link runs in controlled
 * bench environments where the IRX size + tick savings matter more. */

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

#define LWIP_NETIF_LOOPBACK                   1

/**
 * LWIP_HAVE_LOOPIF: lwIP defaults this to (LWIP_NETIF_LOOPBACK && !LWIP_SINGLE_NETIF)
 * which is 1 once we enable LWIP_NETIF_LOOPBACK. That auto-creates a 127.0.0.1
 * loopback netif and may make it the default route during init, which breaks
 * DHCP because DHCP DISCOVER ends up routed through the loop netif and never
 * hits the real SMAP wire (PCSX2 / a real router never sees it). Force it to 0
 * so loopback traffic is handled in-place by the real netif via
 * netif_loop_output, while DHCP / wire traffic still uses the SMAP path.
 */
#define LWIP_HAVE_LOOPIF                      0

#endif /* __LWIPOPTS_H__ */
