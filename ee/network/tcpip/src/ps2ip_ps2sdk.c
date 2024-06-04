/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#define LIBCGLUE_SYS_SOCKET_NO_ALIASES
#define LIBCGLUE_ARPA_INET_NO_ALIASES
#include <ps2sdkapi.h>
#include <string.h>
#include <errno.h>
#include <ps2ipee.h>

_libcglue_fdman_socket_ops_t __ps2ipee_fdman_socket_ops;
_libcglue_fdman_inet_ops_t __ps2ipee_fdman_inet_ops;
_libcglue_fdman_fd_ops_t __ps2ipee_fdman_ops_socket;

extern void __ps2ipeeOpsInitializeImpl(void);

__attribute__((constructor))
static void __ps2ipeeOpsInitialize(void)
{
    __ps2ipeeOpsInitializeImpl();
}

int __ps2ipeeSocketHelper(_libcglue_fdman_fd_info_t *info, int domain, int type, int protocol)
{
    int iop_fd;

    iop_fd = lwip_socket(domain, type, protocol);
    if (iop_fd < 0)
    {
        return -errno;
    }
    info->userdata = (void *)(uiptr)(u32)iop_fd;
    info->ops = &__ps2ipee_fdman_ops_socket;
    return 0;
}

int __ps2ipeeGetFdHelper(void *userdata)
{
    return (int)(u32)(uiptr)userdata;
}

int __ps2ipeeFcntlfsetflHelper(void *userdata, int newfl)
{
    int fd, res;
    u32 val;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    val = (newfl & O_NONBLOCK) ? 1 : 0;
    res = lwip_ioctl(fd, FIONBIO, &val);
    if (res < 0)
    {
        return -ENFILE;
    }
    return res;
}

int __ps2ipeeAcceptHelper(void *userdata, _libcglue_fdman_fd_info_t *info, struct sockaddr *addr, int *addrlen)
{
    int fd, new_fd;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    new_fd = lwip_accept(fd, addr, addrlen);
    if (new_fd < 0)
    {
        return -errno;
    }
    info->userdata = (void *)(uiptr)(u32)new_fd;
    info->ops = &__ps2ipee_fdman_ops_socket;
    return 0;
}

int __ps2ipeeBindHelper(void *userdata, const struct sockaddr *name, int namelen)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_bind(fd, name, namelen);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

int __ps2ipeeCloseHelper(void *userdata)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_close(fd);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

int __ps2ipeeConnectHelper(void *userdata, const struct sockaddr *name, int namelen)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_connect(fd, name, namelen);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

int __ps2ipeeListenHelper(void *userdata, int backlog)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_listen(fd, backlog);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

int __ps2ipeeRecvHelper(void *userdata, void *mem, size_t len, int flags)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_recv(fd, mem, len, flags);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

int __ps2ipeeRecvfromHelper(void *userdata, void *mem, size_t len, int flags, struct sockaddr *from, int *fromlen)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_recvfrom(fd, mem, len, flags, from, fromlen);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

int __ps2ipeeSendHelper(void *userdata, const void *dataptr, size_t size, int flags)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_send(fd, dataptr, size, flags);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

int __ps2ipeeSendtoHelper(void *userdata, const void *dataptr, size_t size, int flags, const struct sockaddr *to, int tolen)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_sendto(fd, dataptr, size, flags, to, tolen);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

int __ps2ipeeIoctlHelper(void *userdata, int cmd, void *argp)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_ioctl(fd, cmd, argp);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

int __ps2ipeeGetsocknameHelper(void *userdata, struct sockaddr* name, int* namelen)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_getsockname(fd, name, namelen);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

int __ps2ipeeGetpeernameHelper(void *userdata, struct sockaddr *name, int *namelen)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_getpeername(fd, name, namelen);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

int __ps2ipeeGetsockoptHelper(void *userdata, int level, int optname, void* optval, socklen_t* optlen)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_getsockopt(fd, level, optname, optval, optlen);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

int __ps2ipeeSetsockoptHelper(void *userdata, int level, int optname, const void *optval, socklen_t optlen)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_setsockopt(fd, level, optname, optval, optlen);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

int __ps2ipeeShutdownHelper(void *userdata, int how)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_shutdown(fd, how);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

int __ps2ipeeReadHelper(void *userdata, void *mem, int len)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_read(fd, mem, len);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

int __ps2ipeeWriteHelper(void *userdata, const void *mem, int len)
{
    int fd, res;

    fd = __ps2ipeeGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = lwip_write(fd, mem, len);
    if (res < 0)
    {
        return -errno;
    }
    return res;
}

struct hostent *ps2ip_gethostbyaddr(const void *addr, int len, int type) {
    // TODO
    return NULL;
}

void __ps2ipeeOpsInitializeImpl(void)
{
    memset(&__ps2ipee_fdman_socket_ops, 0, sizeof(__ps2ipee_fdman_socket_ops));
    __ps2ipee_fdman_socket_ops.setconfig = ps2ip_setconfig;
    __ps2ipee_fdman_socket_ops.getconfig = ps2ip_getconfig;
    __ps2ipee_fdman_socket_ops.dns_setserver = dns_setserver;
    __ps2ipee_fdman_socket_ops.dns_getserver = dns_getserver;
    __ps2ipee_fdman_socket_ops.socket = __ps2ipeeSocketHelper;
    __ps2ipee_fdman_socket_ops.select = lwip_select;
    __ps2ipee_fdman_socket_ops.gethostbyaddr = ps2ip_gethostbyaddr;
    __ps2ipee_fdman_socket_ops.gethostbyname = lwip_gethostbyname;
    __ps2ipee_fdman_socket_ops.gethostbyname_r = lwip_gethostbyname_r;
    __ps2ipee_fdman_socket_ops.freeaddrinfo = lwip_freeaddrinfo;
    __ps2ipee_fdman_socket_ops.getaddrinfo = lwip_getaddrinfo;

    memset(&__ps2ipee_fdman_inet_ops, 0, sizeof(__ps2ipee_fdman_inet_ops));
    __ps2ipee_fdman_inet_ops.inet_addr = ipaddr_addr;
    __ps2ipee_fdman_inet_ops.inet_ntoa = ip4addr_ntoa;
    __ps2ipee_fdman_inet_ops.inet_ntoa_r = ip4addr_ntoa_r;
    __ps2ipee_fdman_inet_ops.inet_aton = ip4addr_aton;

    memset(&__ps2ipee_fdman_ops_socket, 0, sizeof(__ps2ipee_fdman_ops_socket));
    __ps2ipee_fdman_ops_socket.getfd = __ps2ipeeGetFdHelper;
    __ps2ipee_fdman_ops_socket.fcntl_f_setfl = __ps2ipeeFcntlfsetflHelper;

    __ps2ipee_fdman_ops_socket.accept = __ps2ipeeAcceptHelper;
    __ps2ipee_fdman_ops_socket.bind = __ps2ipeeBindHelper;
    __ps2ipee_fdman_ops_socket.close = __ps2ipeeCloseHelper;
    __ps2ipee_fdman_ops_socket.connect = __ps2ipeeConnectHelper;
    __ps2ipee_fdman_ops_socket.listen = __ps2ipeeListenHelper;
    __ps2ipee_fdman_ops_socket.recv = __ps2ipeeRecvHelper;
    __ps2ipee_fdman_ops_socket.recvfrom = __ps2ipeeRecvfromHelper;
    __ps2ipee_fdman_ops_socket.send = __ps2ipeeSendHelper;
    __ps2ipee_fdman_ops_socket.sendto = __ps2ipeeSendtoHelper;
    __ps2ipee_fdman_ops_socket.ioctl = __ps2ipeeIoctlHelper;
    __ps2ipee_fdman_ops_socket.getsockname = __ps2ipeeGetsocknameHelper;
    __ps2ipee_fdman_ops_socket.getpeername = __ps2ipeeGetpeernameHelper;
    __ps2ipee_fdman_ops_socket.getsockopt = __ps2ipeeGetsockoptHelper;
    __ps2ipee_fdman_ops_socket.setsockopt = __ps2ipeeSetsockoptHelper;
    __ps2ipee_fdman_ops_socket.shutdown = __ps2ipeeShutdownHelper;
    __ps2ipee_fdman_ops_socket.read = __ps2ipeeReadHelper;
    __ps2ipee_fdman_ops_socket.write = __ps2ipeeWriteHelper;
}

/* Backup pointer functions to restore after exit ps2ipee */
static _libcglue_fdman_socket_ops_t *_backup_libcglue_fdman_socket_ops;

void _ps2sdk_ps2ipee_init(void)
{
    _backup_libcglue_fdman_socket_ops = _libcglue_fdman_socket_ops;
    _libcglue_fdman_socket_ops = &__ps2ipee_fdman_socket_ops;
}

void _ps2sdk_ps2ipee_deinit(void)
{
    _libcglue_fdman_socket_ops = _backup_libcglue_fdman_socket_ops;
}
