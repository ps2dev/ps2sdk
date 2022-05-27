/**
 * @file
 * PS2IP RPC definitions
 * This header conflicts with fileXio.h
 */

#ifndef __PS2IP_RPC_H__
#define __PS2IP_RPC_H__

#include <tamtypes.h>
#include <tcpip.h>
#include <sys/time.h>

#define PS2IP_IRX 0xB0125F2

enum PS2IPS_RPC_ID {
    PS2IPS_ID_ACCEPT = 1,
    PS2IPS_ID_BIND,
    PS2IPS_ID_DISCONNECT,
    PS2IPS_ID_CONNECT,
    PS2IPS_ID_LISTEN,
    PS2IPS_ID_RECV,
    PS2IPS_ID_RECVFROM,
    PS2IPS_ID_SEND,
    PS2IPS_ID_SENDTO,
    PS2IPS_ID_SOCKET,
    PS2IPS_ID_SETCONFIG,
    PS2IPS_ID_GETCONFIG,
    PS2IPS_ID_SELECT,
    PS2IPS_ID_IOCTL,
    PS2IPS_ID_GETSOCKNAME,
    PS2IPS_ID_GETPEERNAME,
    PS2IPS_ID_GETSOCKOPT,
    PS2IPS_ID_SETSOCKOPT,
    PS2IPS_ID_GETHOSTBYNAME,
#ifdef PS2IP_DNS
    /*    Not implemented:
            gethostbyname_r -> Redudant because it gets called over the RPC.
            freeaddrinfo    -> too complicated and probably nobody uses it?
            getaddrinfo     -> too complicated and probably nobody uses it?    */

    PS2IPS_ID_DNS_SETSERVER,
    PS2IPS_ID_DNS_GETSERVER,
#endif

    PS2IPS_ID_COUNT
};

typedef struct
{
    s32 domain;
    s32 type;
    s32 protocol;
} socket_pkt;

typedef struct
{
    s32 ssize;
    s32 esize;
    u8 *sbuf;
    u8 *ebuf;
    u8 sbuffer[64];
    u8 ebuffer[64];
} rests_pkt;

typedef struct
{
    s32 socket;
    s32 length;
    s32 flags;
    void *ee_addr;
    struct sockaddr sockaddr; // sizeof = 16
    s32 malign;
    /** buffer for sending misaligned portion */
    u8 malign_buff[64];
} send_pkt;

typedef struct
{
    s32 socket;
    s32 length;
    s32 flags;
    void *ee_addr;
    void *intr_data;
} s_recv_pkt;

typedef struct
{
    s32 ret;
    struct sockaddr sockaddr;
} r_recv_pkt;

typedef struct
{
    s32 socket;
    struct sockaddr sockaddr;
    s32 len;
} cmd_pkt;

typedef struct
{
    s32 retval;
    struct sockaddr sockaddr;
} ret_pkt;

typedef struct
{
    s32 s;
    s32 backlog;
} listen_pkt;

typedef struct
{
    s32 s;
    s32 level;
    s32 optname;
} getsockopt_pkt;

typedef struct
{
    s32 result;
    s32 optlen;
    u8 buffer[128];
} getsockopt_res_pkt;

typedef struct
{
    s32 s;
    s32 level;
    s32 optname;
    s32 optlen;
    u8 buffer[128];
} setsockopt_pkt;

typedef struct
{
    union
    {
        s32 maxfdp1;
        s32 result;
    };
    struct timeval *timeout_p;
    struct timeval timeout;
    struct fd_set *readset_p;
    struct fd_set *writeset_p;
    struct fd_set *exceptset_p;
    struct fd_set readset;
    struct fd_set writeset;
    struct fd_set exceptset;
} select_pkt;

typedef struct
{
    union
    {
        s32 s;
        s32 result;
    };
    u32 cmd;
    void *argp;
    u32 value;
} ioctl_pkt;

#ifdef PS2IP_DNS
struct hostent_res
{
    s16 h_addrtype;
    s16 h_length;
    ip_addr_t h_addr;
};

typedef struct
{
    s32 result;
    struct hostent_res hostent;
} gethostbyname_res_pkt;

typedef struct
{
    ip_addr_t dnsserver;
    u8 numdns;
} dns_setserver_pkt;

typedef struct
{
    ip_addr_t dnsserver;
} dns_getserver_res_pkt;

#endif

#endif /* __PS2IP_RPC_H__ */
