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
#include <ps2ipc.h>

_libcglue_fdman_socket_ops_t __ps2ipc_fdman_socket_ops;
_libcglue_fdman_fd_ops_t __ps2ipc_fdman_ops_socket;

extern void __ps2ipcOpsInitializeImpl(void);

__attribute__((constructor))
static void __ps2ipcOpsInitialize(void)
{
    __ps2ipcOpsInitializeImpl();
}

int __ps2ipcSocketHelper(_libcglue_fdman_fd_info_t *info, int domain, int type, int protocol)
{
    int iop_fd;

    iop_fd = ps2ipc_socket(domain, type, protocol);
    if (iop_fd < 0)
    {
        return -ENFILE;
    }
    info->userdata = (void *)(uiptr)(u32)iop_fd;
    info->ops = &__ps2ipc_fdman_ops_socket;
    return 0;
}

int __ps2ipcGetFdHelper(void *userdata)
{
    return (int)(u32)(uiptr)userdata;
}

int __ps2ipcFcntlfsetflHelper(void *userdata, int newfl)
{
    int fd, res;
    u32 val;

    fd = __ps2ipcGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    val = (newfl & O_NONBLOCK) ? 1 : 0;
    res = ps2ipc_ioctl(fd, FIONBIO, &val);
    if (res < 0)
    {
        return -ENFILE;
    }
    return res;
}

int __ps2ipcAcceptHelper(void *userdata, _libcglue_fdman_fd_info_t *info, struct sockaddr *addr, int *addrlen)
{
    int fd, new_fd;

    fd = __ps2ipcGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    new_fd = ps2ipc_accept(fd, addr, addrlen);
    if (new_fd < 0)
    {
        return -ENFILE;
    }
    info->userdata = (void *)(uiptr)(u32)new_fd;
    info->ops = &__ps2ipc_fdman_ops_socket;
    return 0;
}

int __ps2ipcBindHelper(void *userdata, const struct sockaddr *name, int namelen)
{
    int fd, res;

    fd = __ps2ipcGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = ps2ipc_bind(fd, name, namelen);
    if (res < 0)
    {
        return -ENFILE;
    }
    return res;
}

int __ps2ipcCloseHelper(void *userdata)
{
    int fd, res;

    fd = __ps2ipcGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = ps2ipc_disconnect(fd);
    if (res < 0)
    {
        return -ENFILE;
    }
    return res;
}

int __ps2ipcConnectHelper(void *userdata, const struct sockaddr *name, int namelen)
{
    int fd, res;

    fd = __ps2ipcGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = ps2ipc_connect(fd, name, namelen);
    if (res < 0)
    {
        return -ENFILE;
    }
    return res;
}

int __ps2ipcListenHelper(void *userdata, int backlog)
{
    int fd, res;

    fd = __ps2ipcGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = ps2ipc_listen(fd, backlog);
    if (res < 0)
    {
        return -ENFILE;
    }
    return res;
}

int __ps2ipcRecvHelper(void *userdata, void *mem, size_t len, int flags)
{
    int fd, res;

    fd = __ps2ipcGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = ps2ipc_recv(fd, mem, len, flags);
    if (res < 0)
    {
        return -ENFILE;
    }
    return res;
}

int __ps2ipcRecvfromHelper(void *userdata, void *mem, size_t len, int flags, struct sockaddr *from, int *fromlen)
{
    int fd, res;

    fd = __ps2ipcGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = ps2ipc_recvfrom(fd, mem, len, flags, from, fromlen);
    if (res < 0)
    {
        return -ENFILE;
    }
    return res;
}

int __ps2ipcSendHelper(void *userdata, const void *dataptr, size_t len, int flags)
{
    int fd, res;

    fd = __ps2ipcGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = ps2ipc_send(fd, dataptr, len, flags);
    if (res < 0)
    {
        return -ENFILE;
    }
    return res;
}

int __ps2ipcSendtoHelper(void *userdata, const void *dataptr, size_t len, int flags, const struct sockaddr *to, int tolen)
{
    int fd, res;

    fd = __ps2ipcGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = ps2ipc_sendto(fd, dataptr, len, flags, to, tolen);
    if (res < 0)
    {
        return -ENFILE;
    }
    return res;
}

int __ps2ipcIoctlHelper(void *userdata, int cmd, void *argp)
{
    int fd, res;

    fd = __ps2ipcGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = ps2ipc_ioctl(fd, cmd, argp);
    if (res < 0)
    {
        return -ENFILE;
    }
    return res;
}

int __ps2ipcGetsocknameHelper(void *userdata, struct sockaddr* name, int* namelen)
{
    int fd, res;

    fd = __ps2ipcGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = ps2ipc_getsockname(fd, name, namelen);
    if (res < 0)
    {
        return -ENFILE;
    }
    return res;
}

int __ps2ipcGetpeernameHelper(void *userdata, struct sockaddr *name, int *namelen)
{
    int fd, res;

    fd = __ps2ipcGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = ps2ipc_getpeername(fd, name, namelen);
    if (res < 0)
    {
        return -ENFILE;
    }
    return res;
}

int __ps2ipcGetsockoptHelper(void *userdata, int level, int optname, void* optval, socklen_t* optlen)
{
    int fd, res;

    fd = __ps2ipcGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = ps2ipc_getsockopt(fd, level, optname, optval, optlen);
    if (res < 0)
    {
        return -ENFILE;
    }
    return res;
}

int __ps2ipcSetsockoptHelper(void *userdata, int level, int optname, const void *optval, socklen_t optlen)
{
    int fd, res;

    fd = __ps2ipcGetFdHelper(userdata);

    if (fd < 0)
    {
        return fd;
    }

    res = ps2ipc_setsockopt(fd, level, optname, optval, optlen);
    if (res < 0)
    {
        return -ENFILE;
    }
    return res;
}

void __ps2ipcOpsInitializeImpl(void)
{
    memset(&__ps2ipc_fdman_socket_ops, 0, sizeof(__ps2ipc_fdman_socket_ops));
    __ps2ipc_fdman_socket_ops.setconfig = ps2ipc_ps2ip_setconfig;
    __ps2ipc_fdman_socket_ops.getconfig = ps2ipc_ps2ip_getconfig;
    __ps2ipc_fdman_socket_ops.dns_setserver = ps2ipc_dns_setserver;
    __ps2ipc_fdman_socket_ops.dns_getserver = ps2ipc_dns_getserver;
    __ps2ipc_fdman_socket_ops.socket = __ps2ipcSocketHelper;
    __ps2ipc_fdman_socket_ops.select = ps2ipc_select;
    __ps2ipc_fdman_socket_ops.gethostbyname = ps2ipc_gethostbyname;

    memset(&__ps2ipc_fdman_ops_socket, 0, sizeof(__ps2ipc_fdman_ops_socket));
    __ps2ipc_fdman_ops_socket.getfd = __ps2ipcGetFdHelper;
    __ps2ipc_fdman_ops_socket.fcntl_f_setfl = __ps2ipcFcntlfsetflHelper;
    __ps2ipc_fdman_ops_socket.accept = __ps2ipcAcceptHelper;
    __ps2ipc_fdman_ops_socket.bind = __ps2ipcBindHelper;
    __ps2ipc_fdman_ops_socket.close = __ps2ipcCloseHelper;
    __ps2ipc_fdman_ops_socket.connect = __ps2ipcConnectHelper;
    __ps2ipc_fdman_ops_socket.listen = __ps2ipcListenHelper;
    __ps2ipc_fdman_ops_socket.recv = __ps2ipcRecvHelper;
    __ps2ipc_fdman_ops_socket.recvfrom = __ps2ipcRecvfromHelper;
    __ps2ipc_fdman_ops_socket.send = __ps2ipcSendHelper;
    __ps2ipc_fdman_ops_socket.sendto = __ps2ipcSendtoHelper;
    __ps2ipc_fdman_ops_socket.ioctl = __ps2ipcIoctlHelper;
    __ps2ipc_fdman_ops_socket.getsockname = __ps2ipcGetsocknameHelper;
    __ps2ipc_fdman_ops_socket.getpeername = __ps2ipcGetpeernameHelper;
    __ps2ipc_fdman_ops_socket.getsockopt = __ps2ipcGetsockoptHelper;
    __ps2ipc_fdman_ops_socket.setsockopt = __ps2ipcSetsockoptHelper;
}

/* Backup pointer functions to restore after exit ps2ipc */
static _libcglue_fdman_socket_ops_t *_backup_libcglue_fdman_socket_ops;

void _ps2sdk_ps2ipc_init()
{
    _backup_libcglue_fdman_socket_ops = _libcglue_fdman_socket_ops;
    _libcglue_fdman_socket_ops = &__ps2ipc_fdman_socket_ops;
}

void _ps2sdk_ps2ipc_deinit()
{
    _libcglue_fdman_socket_ops = _backup_libcglue_fdman_socket_ops;
}
