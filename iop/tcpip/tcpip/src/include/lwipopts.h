
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#define LWIP_CALLBACK_API 1

/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           4

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE                0x3000

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF          40
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        15
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB        15 
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 15
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG        40
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    5


/* The following four are used only with the sequential API and can be
   set to 0 if the application only will use the raw API. */
/* MEMP_NUM_NETBUF: the number of struct netbufs. */
#define MEMP_NUM_NETBUF         15
/* MEMP_NUM_NETCONN: the number of struct netconns. */
#define MEMP_NUM_NETCONN        15
/* MEMP_NUM_APIMSG: the number of struct api_msg, used for
   communication between the TCP/IP stack and the sequential
   programs. */
#define MEMP_NUM_API_MSG        40
/* MEMP_NUM_TCPIPMSG: the number of struct tcpip_msg, which is used
   for sequential API communication and incoming packets. Used in
   src/api/tcpip.c. */
#define MEMP_NUM_TCPIP_MSG      40

/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE         25

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
//Boman666: Should be atleast 1518 to be compatible with ps2smap
#define PBUF_POOL_BUFSIZE       1540

/* PBUF_LINK_HLEN: the number of bytes that should be allocated for a
   link level header. */
/* MRB: This needs to be the actual size so that pbuf's are aligned properly.  */
#define PBUF_LINK_HLEN          14

/** SYS_LIGHTWEIGHT_PROT
 * define SYS_LIGHTWEIGHT_PROT in lwipopts.h if you want inter-task protection
 * for certain critical regions during buffer allocation, deallocation and 
 * memory allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT 1

/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         1

/* TCP Maximum segment size. */
#define TCP_MSS                 1460

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             (TCP_MSS*4)

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN        (4*TCP_SND_BUF/TCP_MSS)

/* TCP receive window. */
#define TCP_WND                 (16384*2)

/* Maximum number of retransmissions of data segments. */
#define TCP_MAXRTX              12

/* Maximum number of retransmissions of SYN segments. */
#define TCP_SYNMAXRTX           4

/* TCP writable space (bytes).  This must be less than or equal
   to TCP_SND_BUF.  It is the amount of space which must be
   available in the tcp snd_buf for select to return writable */
#define TCP_SNDLOWAT		(TCP_SND_BUF/2)

/* ---------- ARP options ---------- */
#define ARP_TABLE_SIZE 10
#define ARP_QUEUEING 1

/* ---------- IP options ---------- */
/* Define IP_FORWARD to 1 if you wish to have the ability to forward
   IP packets across network interfaces. If you are going to run lwIP
   on a device with only one network interface, define this to 0. */
#define IP_FORWARD              0

/* IP reassembly and segmentation. These are orthogonal even
   if they both deal with IP fragments */
#define IP_REASSEMBLY		1
#define IP_FRAG			1

/* ---------- ICMP options ---------- */
#define ICMP_TTL                255


/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
   interfaces. DHCP is not implemented in lwIP 0.5.1, however, so
   turning this on does currently not work. */

#ifdef PS2IP_DHCP

#define LWIP_DHCP               1

/* 1 if you want to do an ARP check on the offered address
   (recommended). */
#define DHCP_DOES_ARP_CHECK     1

#else

#define LWIP_DHCP		0
#define DHCP_DOES_ARP_CHECK	0

#endif

/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define UDP_TTL                 255


/* ---------- Statistics options ---------- */
#define LWIP_STATS		0

#if		LWIP_STATS
#define LINK_STATS		1
#define IP_STATS			1
#define IPFRAG_STATS		1
#define ICMP_STATS		1
#define UDP_STATS			1
#define TCP_STATS			1
#define MEM_STATS			1
#define MEMP_STATS		1
#define PBUF_STATS		1
#define SYS_STATS			1
#define RAW_STATS			1
#endif	/*LWIP_STATS*/

//Boman666: This define will force the TX-data to be splitted in an even number of TCP-segments. This will significantly increase
//the upload speed, atleast on my configuration (PC - WinXP).
#define	PS2IP_EVEN_TCP_SEG

#endif /* __LWIPOPTS_H__ */
