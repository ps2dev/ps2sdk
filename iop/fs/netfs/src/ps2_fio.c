/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# iop tcp fs driver.
*/

#include <types.h>
#include <defs.h>
#include <irx.h>

#include <loadcore.h>
#include <ioman_mod.h>
#include <iomanX.h>
#include <thbase.h>
#include <thsemap.h>
#include <stdio.h>
#include <sysclib.h>
#include <errno.h>
#include <ps2ip.h>
#include <iopmgr.h>

#include <sys/stat.h>

#include "ps2_fio.h"
#include "ps2fs.h"
#include "byteorder.h"
#include "devscan.h"

#define ntohl(x) htonl(x)
#define ntohs(x) htons(x)

//#define DEBUG
//#define DEBUG
#ifdef DEBUG
#define dbgprintf(args...) printf(args)
#else
#define dbgprintf(args...) do { } while(0)
#endif

//////////////////////////////////////////////////////////////////////////
// How about a header file?..
extern int fsysUnmount(void);

//////////////////////////////////////////////////////////////////////////
#define PACKET_MAXSIZE 4096

static char ps2netfs_send_packet[PACKET_MAXSIZE] __attribute__((aligned(16)));
static char ps2netfs_recv_packet[PACKET_MAXSIZE] __attribute__((aligned(16)));

static int ps2netfs_sock = -1;
static int ps2netfs_active = 0;

static int ps2netfs_sema;
static int ps2netfs_pid = 0;

#define FIOTRAN_MAXSIZE 65535
static char ps2netfs_fiobuffer[FIOTRAN_MAXSIZE+1];

//////////////////////////////////////////////////////////////////////////

/*
 *  \brief File Descriptor Handler functions
 *  \ingroup ps2netfs 
 *
 *        Handles upto FDH_MAXFD open files concurrently.
 */
typedef struct {
  int used;
  int realfd;
  int devtype;
} fd_table_t;

#define FDH_MAX 50
static fd_table_t fd_info_list[FDH_MAX+1]; /* one for padding */

/*! \brief Initialise the file descriptor table.
 *  \ingroup ps2netfs 
 *
 *  This initialises the file descriptor table that we
 *  use to translate to numbers passed to client, and to
 *  enable quick lookups on handler type.
 */
static inline void fdh_setup()
{
  memset(&fd_info_list,0,sizeof(fd_info_list));
}

/*! \brief Get file descriptor for client.
 *  \ingroup ps2netfs 
 *
 *  \param devtype Device Manager type.
 *  \param fd      PS2 file descriptor.
 *  \return FD to send to client.
 *
 *  return values:
 *    -1 if no space in list.
 *    FD if returning valid entry.
 */
static inline int fdh_getfd(int devtype,int fd)
{
  int count = 1;
  while ((count < FDH_MAX))
  {
    if (fd_info_list[count].used == 0)
    { 
      fd_info_list[count].used = 1;
      fd_info_list[count].realfd = fd;
      fd_info_list[count].devtype = devtype;
      return count;
    }
    count++;
  }
  return -1;
}

/*! \brief Free file descriptor from client table.
 *  \ingroup ps2netfs 
 *
 *  \param fd      client file descriptor.
 *
 *  no return value.
 */
static inline void fdh_freefd(int fd)
{
  if (fd < FDH_MAX)
  {
    fd_info_list[fd].used = 0;
    fd_info_list[fd].realfd = 0;
    fd_info_list[fd].devtype = 0;
  }
}

/*! \brief Get fd list entry. */
#define fdh_get(a) (&fd_info_list[a])
/*! \brief Get fd list entry used value. */
#define fdh_getused(a) (fd_info_list[a].used)
/*! \brief Get fd list entry type value. */
#define fdh_getfdtype(a) (fd_info_list[a].devtype)
/*! \brief Get fd list entry ps2 file descriptor value. */
#define fdh_getrealfd(a) (fd_info_list[a].realfd)


/*! \brief changes mode from ioman to iomanx format
 *  \ingroup ps2netfs
 *
 */
static inline int convmode_to_iomanx(int stat)
{
  int mode = 0;
  
  if FIO_SO_ISLNK(stat) mode |= FIO_S_IFLNK; // Symbolic link
  if FIO_SO_ISREG(stat) mode |= FIO_S_IFREG; // regular file 
  if FIO_SO_ISDIR(stat) mode |= FIO_S_IFDIR; // directory

  if (((stat) & FIO_SO_IROTH) == FIO_SO_IROTH) // read
    mode |= FIO_S_IRUSR | FIO_S_IRGRP | FIO_S_IROTH;
  if (((stat) & FIO_SO_IWOTH) == FIO_SO_IWOTH) // write
    mode |= FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH;
  if (((stat) & FIO_SO_IXOTH) == FIO_SO_IXOTH) // execute
    mode |= FIO_S_IXUSR | FIO_S_IXGRP | FIO_S_IXOTH;

  return mode;
}

/*! \brief changes mode from iomanx to ioman format
 *  \ingroup ps2netfs
 *
 */
static inline int convmode_from_iomanx(int stat)
{
  return stat;
}

/*! \brief Shortcut to close the socket and cleanup.
 *  \ingroup ps2netfs 
 *
 *  \return disconnect return value.
 */
int ps2netfs_close_socket(void)
{
  int ret;

  ret = disconnect(ps2netfs_sock);
  if (ret < 0) 
    printf("ps2netfs: disconnect returned error %d\n", ret);
  ps2netfs_sock = -1;
  return ret;
}

/*! \brief Shortcut to close the socket forcibly and set to exit (active=0).
 *  \ingroup ps2netfs 
 *
 *  This will close down the socket, and cause the server thread to exit.
 *
 *  \return disconnect return value.
 */
void ps2netfs_close_fsys(void)
{
  if (ps2netfs_sock > 0) 
    disconnect(ps2netfs_sock);
  ps2netfs_active = 0;
  return;
}

//---------------------------------------------------------------------- 
// XXX: Hm, this func should behave sorta like pko_recv_bytes imho..
// i.e. check if it was able to send just a part of the packet etc..
static inline int ps2netfs_lwip_send(int sock, void *buf, int len, int flag)
{
  int ret;
  ret = send(sock, buf, len, flag);
  if (ret < 0) 
  {
    dbgprintf("ps2netfs: lwip_send() error %d\n", ret);
    ps2netfs_close_socket();
    return -1;
  }
  else if (len == 0)
  { // send is blocking, so 0 means error
    dbgprintf("ps2netfs: lwip_send() - disconnected\n");
    return -2;
  }
  else return ret;
}

//---------------------------------------------------------------------- 
// Do repetetive recv() calls until 'bytes' bytes are received
// or error returned
int ps2netfs_recv_bytes(int sock, char *buf, int bytes)
{
  int left;
  int len;

  left = bytes;

  while (left > 0) {
    len = recv(sock, &buf[bytes - left], left, 0);
    if (len < 0) {
      dbgprintf("ps2netfs: recv_bytes error!! (%d)\n",len);
      return -1;
    }
    if (len == 0)
    { // recv is blocking, so 0 means error
      dbgprintf("ps2netfs: recv_bytes - disconnected\n");
      return -2;
    }
    left -= len;
  }
  return bytes;
}

//---------------------------------------------------------------------- 
// Receive a 'packet' of unknown type, max size 4096 bytes
int ps2netfs_accept_pktunknown(int sock, char *buf)
{
  int length;
  ps2netfs_pkt_hdr *hdr;
  unsigned int hcmd;
  unsigned short hlen;

  dbgprintf("ps2netfs: accept_pkt\n");
  length = ps2netfs_recv_bytes(sock, buf, sizeof(ps2netfs_pkt_hdr));
  if (length == 0) return 0;
  if (length < 0) {
    dbgprintf("ps2netfs: accept_pkt recv error\n");
    return -1;
  }

  if (length < sizeof(ps2netfs_pkt_hdr)) {
    dbgprintf("ps2netfs: XXX: did not receive a full header!!!! "
              "Fix this! (%d)\n", length);
    return -1;
  }
    
  hdr = (ps2netfs_pkt_hdr *)buf;
  hcmd = ntohl(hdr->cmd);
  hlen = ntohs(hdr->len);

  dbgprintf("ps2netfs: accept_pkt: got 0x%x , hlen: %d, length: %d\n", hcmd,hlen,length);

  if ((length > PACKET_MAXSIZE) || (hlen > PACKET_MAXSIZE)) 
  {
    dbgprintf("ps2netfs: accept_pkt: hlen is too large!! "
              "(%d,%d can only receive %d)\n", length,hlen, PACKET_MAXSIZE);
    return -1;
  }

  // get the actual packet data
  length = ps2netfs_recv_bytes(sock, buf + sizeof(ps2netfs_pkt_hdr), 
                               hlen - sizeof(ps2netfs_pkt_hdr));

  if (length < 0) {
    dbgprintf("ps2netfs: accept recv2 error!!\n");
    return -1;
  }

  if (length < (hlen - sizeof(ps2netfs_pkt_hdr))) {
    dbgprintf("ps2netfs: Did not receive full packet!!! "
              "Fix this! (%d)\n", length);
  }

  return hlen;
}

/*! \brief Handles a PS2NETFS_INFO_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -1 if error.
 *    0 if handled ok.
 */
static int ps2netfs_op_info(char *buf, int len)
{
  ps2netfs_pkt_open_req *cmd;
  ps2netfs_pkt_info_rly *inforly;
  int count;

  cmd = (ps2netfs_pkt_open_req *)buf;

  dbgprintf("ps2netfs: info\n");

  if (len != sizeof(ps2netfs_pkt_open_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }
  inforly = (ps2netfs_pkt_info_rly *)&ps2netfs_send_packet[0];

  // do the stuff here
  count = 0;

  // now build the response

  // Build packet
  inforly->cmd = htonl(PS2NETFS_INFO_RLY);
  inforly->len = htons((unsigned short)sizeof(ps2netfs_pkt_info_rly));
  inforly->retval = htonl(count);
  inforly->count = htonl(count);

  if (ps2netfs_lwip_send(ps2netfs_sock, inforly, sizeof(ps2netfs_pkt_info_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_FSTYPE_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -1 if error.
 *    0 if handled ok.
 */
static int ps2netfs_op_fstype(char *buf, int len)
{
  ps2netfs_pkt_open_req *cmd;
  ps2netfs_pkt_file_rly *openrly;
  int devtype = -1;

  cmd = (ps2netfs_pkt_open_req *)buf;

  dbgprintf("ps2netfs: fstype\n");

  if (len != sizeof(ps2netfs_pkt_open_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }
  // do the stuff here
  devtype = devscan_gettype(cmd->path);
    
  // now build the response
  openrly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  openrly->cmd = htonl(PS2NETFS_OPEN_RLY);
  openrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  openrly->retval = htonl(devtype);

  if (ps2netfs_lwip_send(ps2netfs_sock, openrly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_DEVLIST_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -1 if error.
 *    0 if handled ok.
 */
static int ps2netfs_op_devlist(char *buf, int len)
{
  ps2netfs_pkt_open_req *cmd;
  ps2netfs_pkt_devlist_rly *devlistrly;
  int count;

  cmd = (ps2netfs_pkt_open_req *)buf;

  dbgprintf("ps2netfs: devlist\n");

  if (len != sizeof(ps2netfs_pkt_open_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }
  devlistrly = (ps2netfs_pkt_devlist_rly *)&ps2netfs_send_packet[0];

  // do the stuff here
  count = devscan_getdevlist(devlistrly->list);

  // first char of the path passed in is the delimiter
  { // so fudge in the delimiters
    int cnt; char *ptr = &devlistrly->list[0]; int len;
    for(cnt=0;cnt<(count-1);cnt++)
    {  
      len = strlen(ptr);
      // ok, got a string, so replace the final 0, with delimiter
      // move onto next string
      *(ptr+len) = cmd->path[0];
      ptr += len + 1;
    }
  }

  // now build the response

  // Build packet
  devlistrly->cmd = htonl(PS2NETFS_DEVLIST_RLY);
  devlistrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_devlist_rly));
  devlistrly->retval = htonl(count);
  devlistrly->count = htonl(count);

  if (ps2netfs_lwip_send(ps2netfs_sock, devlistrly, sizeof(ps2netfs_pkt_devlist_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}


/*! \brief Handles a PS2NETFS_OPEN_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    +X if handled ok, valid FD returned.
 *    -X if error.
 */
// int open(const char *name, int flags, ...);
static int ps2netfs_op_open(char *buf, int len)
{
  ps2netfs_pkt_open_req *cmd;
  ps2netfs_pkt_file_rly *openrly;
  int retval = -ENODEV;
  int devtype = 0;

  cmd = (ps2netfs_pkt_open_req *)buf;

  dbgprintf("ps2netfs: open\n");

  if (len != sizeof(ps2netfs_pkt_open_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }
  // do the stuff here
  devtype = devscan_gettype(cmd->path);
  if (devtype != IOPMGR_DEVTYPE_INVALID)
  { 
    if (devtype == IOPMGR_DEVTYPE_IOMAN)
      retval = io_open(cmd->path,ntohl(cmd->flags));
    else if (devtype == IOPMGR_DEVTYPE_IOMANX)
      retval = open(cmd->path,ntohl(cmd->flags),0644);
    if (retval > 0)
      retval = fdh_getfd(devtype,retval);
  }

  // now build the response
  openrly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  openrly->cmd = htonl(PS2NETFS_OPEN_RLY);
  openrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  openrly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, openrly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_CLOSE_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    >=0 if handled ok.
 *    -X if error.
 */
// int close(int fd);
static int ps2netfs_op_close(char *buf, int len)
{
  ps2netfs_pkt_close_req *cmd;
  ps2netfs_pkt_file_rly *closerly;
  int retval = -1;
  fd_table_t *fdptr;
  cmd = (ps2netfs_pkt_close_req *)buf;

  dbgprintf("ps2netfs: close\n");

  if (len != sizeof(ps2netfs_pkt_close_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }

  // do the stuff here
  fdptr = fdh_get(ntohl(cmd->fd));
  if (fdptr != 0)
  { 
    if (fdptr->devtype == IOPMGR_DEVTYPE_IOMAN)
      retval = io_close(fdptr->realfd);
    else if (fdptr->devtype == IOPMGR_DEVTYPE_IOMANX)
      retval = close(fdptr->realfd);
    fdh_freefd(ntohl(cmd->fd));
  }

  // now build the response
  closerly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  closerly->cmd = htonl(PS2NETFS_CLOSE_RLY);
  closerly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  closerly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, closerly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_READ_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    >=0 if handled ok , bytes read.
 *    -X if error.
 */
// int read(int fd, void *ptr, size_t size);
static int ps2netfs_op_read(char *buf, int len)
{
  ps2netfs_pkt_read_req *cmd;
  ps2netfs_pkt_read_rly *readrly;
  int retval = -1;
  int nbytes;
  fd_table_t *fdptr;
  cmd = (ps2netfs_pkt_read_req *)buf;

  dbgprintf("ps2netfs: read\n");

  if (len != sizeof(ps2netfs_pkt_read_req))  
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }
  // limit read request to 64k
  nbytes = ntohl(cmd->nbytes);
  if (nbytes > FIOTRAN_MAXSIZE) nbytes = FIOTRAN_MAXSIZE;

  // do the stuff here
  fdptr = fdh_get(ntohl(cmd->fd));
  if (fdptr != 0)
  { 
    if (fdptr->devtype == IOPMGR_DEVTYPE_IOMAN)
      retval = io_read(fdptr->realfd,ps2netfs_fiobuffer,nbytes);
    else if (fdptr->devtype == IOPMGR_DEVTYPE_IOMANX)
      retval = read(fdptr->realfd,ps2netfs_fiobuffer,nbytes);
  }

  // now build the response
  readrly = (ps2netfs_pkt_read_rly *)&ps2netfs_send_packet[0];

  // Build packet
  readrly->cmd = htonl(PS2NETFS_READ_RLY);
  readrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_read_rly));
  readrly->retval = htonl(retval);
  readrly->nbytes = readrly->retval;

  // send the response
  if (ps2netfs_lwip_send(ps2netfs_sock, readrly, sizeof(ps2netfs_pkt_read_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  // now send the data
  if (retval > 0)
    if (ps2netfs_lwip_send(ps2netfs_sock, ps2netfs_fiobuffer, ntohl(readrly->retval), 0) < 0) 
    {
      dbgprintf("ps2netfs: error sending data!\n");
      return -1;
    }
  return 0;
}

/*! \brief Handles a PS2NETFS_WRITE_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    >=0 if handled ok , bytes written.
 *    -X if error.
 */
// int write(int fd, void *ptr, size_t size);
static int ps2netfs_op_write(char *buf, int len)
{
  ps2netfs_pkt_write_req *cmd;
  ps2netfs_pkt_file_rly *writerly;
  int retval = -1;
  fd_table_t *fdptr;
  cmd = (ps2netfs_pkt_write_req *)buf;

  dbgprintf("ps2netfs: write\n");
 
  // do the stuff here
  fdptr = fdh_get(ntohl(cmd->fd));
  if (fdptr != 0)
  { 
    int left = ntohl(cmd->nbytes);
    int towrite, written;
    written = retval = 0;
    while ((retval >= 0) && (left > 0))
    {
      towrite = left; if (towrite > FIOTRAN_MAXSIZE) towrite = FIOTRAN_MAXSIZE;
      if (ps2netfs_recv_bytes(ps2netfs_sock, ps2netfs_fiobuffer, towrite) <=0 )
      {
        dbgprintf("ps2netfs: error reading data!\n");
        return -1;
      }

      if (fdptr->devtype == IOPMGR_DEVTYPE_IOMAN)
        retval = io_write(fdptr->realfd,ps2netfs_fiobuffer,towrite);
      else if (fdptr->devtype == IOPMGR_DEVTYPE_IOMANX)
        retval = write(fdptr->realfd,ps2netfs_fiobuffer,towrite);

      if (retval > 0) { written += retval; left -= retval; }
    }
    if (retval > 0)
        retval = written;
  }

  // now build the response
  writerly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  writerly->cmd = htonl(PS2NETFS_WRITE_RLY);
  writerly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  writerly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, writerly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_LSEEK_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    >=0 if handled ok , position.
 *    -X if error.
 */
// int lseek(int fd, int offset, int mode);
static int ps2netfs_op_lseek(char *buf, int len)
{
  ps2netfs_pkt_lseek_req *cmd;
  ps2netfs_pkt_file_rly *lseekrly;
  fd_table_t *fdptr;
  int retval = -ENODEV; 
  cmd = (ps2netfs_pkt_lseek_req *)buf;

  dbgprintf("ps2netfs: lseek\n");

  if (len != sizeof(ps2netfs_pkt_lseek_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }
  // do the stuff here
  fdptr = fdh_get(ntohl(cmd->fd));
  if (fdptr != 0)
  { 
    if (fdptr->devtype == IOPMGR_DEVTYPE_IOMAN)
      retval = io_lseek(fdptr->realfd,ntohl(cmd->offset),ntohl(cmd->whence));
    else if (fdptr->devtype == IOPMGR_DEVTYPE_IOMANX)
      retval = lseek(fdptr->realfd,ntohl(cmd->offset),ntohl(cmd->whence));
  }

  // now build the response
  lseekrly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  lseekrly->cmd = htonl(PS2NETFS_LSEEK_RLY);
  lseekrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  lseekrly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, lseekrly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_IOCTL_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    >=0 if handled ok.
 *    -X if error.
 */
// int ioctl(int fd, unsigned long cmd, void *param);
static int ps2netfs_op_ioctl(char *buf, int len)
{
  ps2netfs_pkt_ioctl_req *cmd;
  ps2netfs_pkt_ioctl_rly *ioctlrly;
  fd_table_t *fdptr;
  int retval = -ENODEV; 
  cmd = (ps2netfs_pkt_ioctl_req *)buf;

  dbgprintf("ps2netfs: ioctl\n");

  if (len != sizeof(ps2netfs_pkt_ioctl_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }
  // now build the response
  ioctlrly = (ps2netfs_pkt_ioctl_rly *)&ps2netfs_send_packet[0];

  // do the stuff here
  fdptr = fdh_get(ntohl(cmd->fd));
  if (fdptr != 0)
  { 
    if (fdptr->devtype == IOPMGR_DEVTYPE_IOMAN)
      retval = io_ioctl(fdptr->realfd,ntohl(cmd->command),(void *)ioctlrly->buf);
    else if (fdptr->devtype == IOPMGR_DEVTYPE_IOMANX)
      retval = ioctl(fdptr->realfd,ntohl(cmd->command),(void *)ioctlrly->buf);
  }

  // Build packet
  ioctlrly->cmd = htonl(PS2NETFS_IOCTL_RLY);
  ioctlrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_ioctl_rly));
  ioctlrly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, ioctlrly, sizeof(ps2netfs_pkt_ioctl_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_REMOVE_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    >=0 if handled ok.
 *    -X if error.
 */
// int remove(const char *name);
static int ps2netfs_op_remove(char *buf, int len)
{
  ps2netfs_pkt_open_req *cmd;
  ps2netfs_pkt_file_rly *removerly;
  int retval = -ENODEV; 
  int devtype;
  cmd = (ps2netfs_pkt_open_req *)buf;

  dbgprintf("ps2netfs: remove\n");

  if (len != sizeof(ps2netfs_pkt_open_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }

  // do the stuff here
  devtype = devscan_gettype(cmd->path);
  if (devtype != IOPMGR_DEVTYPE_INVALID)
  { 
    if (devtype == IOPMGR_DEVTYPE_IOMAN)
      retval = io_remove(cmd->path);
    else if (devtype == IOPMGR_DEVTYPE_IOMANX)
      retval = remove(cmd->path);
  }

  // now build the response
  removerly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  removerly->cmd = htonl(PS2NETFS_REMOVE_RLY);
  removerly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  removerly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, removerly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_MKDIR_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    >=0 if handled ok.
 *    -X if error.
 */
// int mkdir(const char *path, ...);
static int ps2netfs_op_mkdir(char *buf, int len)
{
  ps2netfs_pkt_open_req *cmd;
  ps2netfs_pkt_file_rly *mkdirrly;
  int retval = -19; 
  int devtype;
  cmd = (ps2netfs_pkt_open_req *)buf;

  dbgprintf("ps2netfs: mkdir\n");

  if (len != sizeof(ps2netfs_pkt_open_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }

  // do the stuff here
  devtype = devscan_gettype(cmd->path);
  if (devtype != IOPMGR_DEVTYPE_INVALID)
  { 
    if (devtype == IOPMGR_DEVTYPE_IOMAN)
      retval = io_mkdir(cmd->path);
    else if (devtype == IOPMGR_DEVTYPE_IOMANX)
      retval = mkdir(cmd->path,0644);
  }

  // now build the response
  mkdirrly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  mkdirrly->cmd = htonl(PS2NETFS_MKDIR_RLY);
  mkdirrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  mkdirrly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, mkdirrly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_RMDIR_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    >=0 if handled ok.
 *    -X if error.
 */
// int rmdir(const char *path);
static int ps2netfs_op_rmdir(char *buf, int len)
{
  ps2netfs_pkt_open_req *cmd;
  ps2netfs_pkt_file_rly *rmdirrly;
  int retval = -ENODEV; 
  int devtype;
  cmd = (ps2netfs_pkt_open_req *)buf;

  dbgprintf("ps2netfs: remove\n");

  if (len != sizeof(ps2netfs_pkt_open_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }

  // do the stuff here
  devtype = devscan_gettype(cmd->path);
  if (devtype != IOPMGR_DEVTYPE_INVALID)
  { 
    if (devtype == IOPMGR_DEVTYPE_IOMAN)
      retval = io_rmdir(cmd->path);
    else if (devtype == IOPMGR_DEVTYPE_IOMANX)
      retval = rmdir(cmd->path);
  }

  // now build the response
  rmdirrly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  rmdirrly->cmd = htonl(PS2NETFS_RMDIR_RLY);
  rmdirrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  rmdirrly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, rmdirrly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_DOPEN_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    +X if handled ok, valid FD returned.
 *    -X if error.
 */
// int dopen(const char *path);
static int ps2netfs_op_dopen(char *buf, int len)
{
  ps2netfs_pkt_open_req *cmd;
  ps2netfs_pkt_file_rly *dopenrly;
  int retval = -ENODEV; 
  int devtype;
  cmd = (ps2netfs_pkt_open_req *)buf;

  dbgprintf("ps2netfs: dopen\n");

  if (len != sizeof(ps2netfs_pkt_open_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }

  // do the stuff here
  devtype = devscan_gettype(cmd->path);
  if (devtype != IOPMGR_DEVTYPE_INVALID)
  { 
    if (devtype == IOPMGR_DEVTYPE_IOMAN)
      retval = io_dopen(cmd->path,0);
    else if (devtype == IOPMGR_DEVTYPE_IOMANX)
      retval = dopen(cmd->path);
    if (retval > 0)
      retval = fdh_getfd(devtype,retval);
  }

  // now build the response
  dopenrly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  dopenrly->cmd = htonl(PS2NETFS_DOPEN_RLY);
  dopenrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  dopenrly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, dopenrly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_DCLOSE_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    >=0 if handled ok.
 *    -X if error.
 */
// int dclose(int fd);
static int ps2netfs_op_dclose(char *buf, int len)
{
  ps2netfs_pkt_close_req *cmd;
  ps2netfs_pkt_file_rly *closerly;
  fd_table_t *fdptr;
  int retval = -ENODEV;
  cmd = (ps2netfs_pkt_close_req *)buf;

  dbgprintf("ps2netfs: dclose\n");

  if (len != sizeof(ps2netfs_pkt_close_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }

  // do the stuff here
  fdptr = fdh_get(ntohl(cmd->fd));
  if (fdptr != 0)
  { 
    if (fdptr->devtype == IOPMGR_DEVTYPE_IOMAN)
      retval = io_dclose(fdptr->realfd);
    else if (fdptr->devtype == IOPMGR_DEVTYPE_IOMANX)
      retval = dclose(fdptr->realfd);
    if (retval == 0) fdh_freefd(ntohl(cmd->fd));
  }

  // now build the response
  closerly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  closerly->cmd = htonl(PS2NETFS_DCLOSE_RLY);
  closerly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  closerly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, closerly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_DREAD_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    >=0 if handled ok.
 *    -X if error.
 */
// int dread(int fd, void *buf);
static int ps2netfs_op_dread(char *buf, int len)
{
  ps2netfs_pkt_dread_req *cmd;
  ps2netfs_pkt_dread_rly *dreadrly;
  fd_table_t *fdptr;
  int retval = -ENODEV;
  cmd = (ps2netfs_pkt_dread_req *)buf;

  dbgprintf("ps2netfs: dread\n");

  if (len != sizeof(ps2netfs_pkt_dread_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }

  // get response structure ready for filling
  dreadrly = (ps2netfs_pkt_dread_rly *)&ps2netfs_send_packet[0];
  memset(dreadrly,0,sizeof(ps2netfs_pkt_dread_rly));

  // do the stuff here
  fdptr = fdh_get(ntohl(cmd->fd));
  if (fdptr != 0)
  { 
    if (fdptr->devtype == IOPMGR_DEVTYPE_IOMAN)
    {
      io_dirent_t dirent;
      retval = io_dread(fdptr->realfd,&dirent);
      if (retval > 0)
      {
        dreadrly->mode   = htonl(convmode_to_iomanx(dirent.stat.mode));
        dreadrly->attr   = htonl(dirent.stat.attr);
        dreadrly->size   = htonl(dirent.stat.size);
        dreadrly->hisize = htonl(0);
        memcpy(dreadrly->ctime,dirent.stat.ctime,8*3);
        strncpy(dreadrly->name,dirent.name,255);
        dreadrly->name[255] = '\0';
      }
    }
    else if (fdptr->devtype == IOPMGR_DEVTYPE_IOMANX)
    {
      iox_dirent_t dirent;
      retval = dread(fdptr->realfd,&dirent);
      if (retval > 0)
      {
        dreadrly->mode   = htonl(dirent.stat.mode);
        dreadrly->attr   = htonl(dirent.stat.attr);
        dreadrly->size   = htonl(dirent.stat.size);
        dreadrly->hisize = htonl(0);
        memcpy(dreadrly->ctime,dirent.stat.ctime,8*3);
        strncpy(dreadrly->name,dirent.name,255);
        dreadrly->name[255] = '\0';
      }
    }
  }
  dbgprintf("ps2netfs: dread '%s' %d\n",dreadrly->name,dreadrly->size);
  // Build packet
  dreadrly->cmd = htonl(PS2NETFS_DREAD_RLY);
  dreadrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_dread_rly));
  dreadrly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, dreadrly, sizeof(ps2netfs_pkt_dread_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_GETSTAT_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    +X if handled ok, valid FD returned.
 *    -X if error.
 */
// int getstat(const char *name, void *stat);
static int ps2netfs_op_getstat(char *buf, int len)
{
  return -1;
}

/*! \brief Handles a PS2NETFS_CHSTAT_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    +X if handled ok, valid FD returned.
 *    -X if error.
 */
// int chstat(const char *name, void *stat, unsigned int statmask);
static int ps2netfs_op_chstat(char *buf, int len)
{
  return -1;
}

/*! \brief Handles a PS2NETFS_FORMAT_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    +X if handled ok, valid FD returned.
 *    -X if error.
 */
// int format(const char *dev, const char *blockdev, void *arg, size_t arglen);
// #define PFS_ZONE_SIZE	8192
// #define PFS_FRAGMENT	0x00000000
// int Arg[3] = { PFS_ZONE_SIZE, 0x2d66, PFS_FRAGMENT };
// retVal = fileXioFormat("pfs:", "hdd0:partition", (const char*)&Arg, sizeof(Arg));

#define PFS_ZONE_SIZE	8192
#define PFS_FRAGMENT	0x00000000
static int ps2netfs_op_format(char *buf, int len)
{
  ps2netfs_pkt_format_req *cmd;
  ps2netfs_pkt_file_rly *formatrly;
  int retval = -ENODEV;
  int devtype = 0;

  cmd = (ps2netfs_pkt_format_req *)buf;

  dbgprintf("ps2netfs: format\n");

  if (len != sizeof(ps2netfs_pkt_format_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }
  // do the stuff here
  devtype = devscan_gettype(cmd->dev);
  if (devtype == IOPMGR_DEVTYPE_IOMANX)
  { // if other end has not set args, add some standard ones (hdd use only for mo)
    if (ntohl(cmd->arglen) == 0)
    {
      int Arg[3] = { PFS_ZONE_SIZE, 0x2d66, PFS_FRAGMENT };
      retval = format(cmd->dev,cmd->blockdev,(char*)&Arg,sizeof(Arg));
    }
    else retval = format(cmd->dev,cmd->blockdev,cmd->arg,ntohl(cmd->arglen));
  }

  // now build the response
  formatrly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  formatrly->cmd = htonl(PS2NETFS_FORMAT_RLY);
  formatrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  formatrly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, formatrly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

// EXTENDED CALLS FROM HERE ON IN

/*! \brief Handles a PS2NETFS_RENAME_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    +X if handled ok, valid FD returned.
 *    -X if error.
 */
// int rename(const char *old, const char *new);
static int ps2netfs_op_rename(char *buf, int len)
{
  ps2netfs_pkt_symlink_req *cmd;
  ps2netfs_pkt_file_rly *renamerly;
  int retval = -ENODEV; 
  int devtype;
  cmd = (ps2netfs_pkt_symlink_req *)buf;

  dbgprintf("ps2netfs: rename\n");

  if (len != sizeof(ps2netfs_pkt_symlink_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }

  // check that both are iomanx devices
  devtype = devscan_gettype(cmd->oldpath);
  if (devtype == IOPMGR_DEVTYPE_IOMANX)
  {
    devtype = devscan_gettype(cmd->newpath);
    if (devtype == IOPMGR_DEVTYPE_IOMANX)
      retval = rename(cmd->oldpath,cmd->newpath);
    else retval = -1;
  }
  // now build the response
  renamerly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  renamerly->cmd = htonl(PS2NETFS_SYMLINK_RLY);
  renamerly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  renamerly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, renamerly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1 ; 
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_CHDIR_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    +X if handled ok, valid FD returned.
 *    -X if error.
 */
// int chdir(const char *name);
static int ps2netfs_op_chdir(char *buf, int len)
{
  ps2netfs_pkt_open_req *cmd;
  ps2netfs_pkt_file_rly *chdirrly;
  int retval = -ENODEV; 
  int devtype;
  cmd = (ps2netfs_pkt_open_req *)buf;

  dbgprintf("ps2netfs: chdir\n");

  if (len != sizeof(ps2netfs_pkt_open_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }

  // do the stuff here
  devtype = devscan_gettype(cmd->path);
  if (devtype == IOPMGR_DEVTYPE_IOMANX)
    retval = chdir(cmd->path);

  // now build the response
  chdirrly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  chdirrly->cmd = htonl(PS2NETFS_CHDIR_RLY);
  chdirrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  chdirrly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, chdirrly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_SYNC_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    +X if handled ok, valid FD returned.
 *    -X if error.
 */
// int sync(const char *dev, int flag);
static int ps2netfs_op_sync(char *buf, int len)
{
  ps2netfs_pkt_open_req *cmd;
  ps2netfs_pkt_file_rly *syncrly;
  int retval = -ENODEV; 
  int devtype;
  cmd = (ps2netfs_pkt_open_req *)buf;

  dbgprintf("ps2netfs: sync\n");

  if (len != sizeof(ps2netfs_pkt_open_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }

  // do the stuff here
  devtype = devscan_gettype(cmd->path);
  if (devtype == IOPMGR_DEVTYPE_IOMANX)
    retval = sync(cmd->path,ntohl(cmd->flags));

  // now build the response
  syncrly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  syncrly->cmd = htonl(PS2NETFS_SYNC_RLY);
  syncrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  syncrly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, syncrly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1;
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_MOUNT_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    +X if handled ok, valid FD returned.
 *    -X if error.
 */
// int mount(const char *fsname, const char *devname, int flag, void *arg, size_t arglen);
static int ps2netfs_op_mount(char *buf, int len)
{
  ps2netfs_pkt_mount_req *cmd;
  ps2netfs_pkt_file_rly *mountrly;
  int retval = -ENODEV; 
  int devtype;
  cmd = (ps2netfs_pkt_mount_req *)buf;

  dbgprintf("ps2netfs: mount\n");

  if (len != sizeof(ps2netfs_pkt_mount_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }

  // do the stuff here
  devtype = devscan_gettype(cmd->fsname);
  if (devtype == IOPMGR_DEVTYPE_IOMANX)
    retval = mount(cmd->fsname,cmd->devname,ntohl(cmd->flag),cmd->arg,ntohl(cmd->arglen));

  // now build the response
  mountrly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  mountrly->cmd = htonl(PS2NETFS_MOUNT_RLY);
  mountrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  mountrly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, mountrly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1 ; 
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_UMOUNT_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    +X if handled ok, valid FD returned.
 *    -X if error.
 */
// int umount(const char *fsname);
static int ps2netfs_op_umount(char *buf, int len)
{
  ps2netfs_pkt_open_req *cmd;
  ps2netfs_pkt_file_rly *umountrly;
  int retval = -ENODEV; 
  int devtype;
  cmd = (ps2netfs_pkt_open_req *)buf;

  dbgprintf("ps2netfs: umount\n");

  if (len != sizeof(ps2netfs_pkt_open_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }

  // do the stuff here
  devtype = devscan_gettype(cmd->path);
  if (devtype == IOPMGR_DEVTYPE_IOMANX)
    retval = umount(cmd->path);

  // now build the response
  umountrly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  umountrly->cmd = htonl(PS2NETFS_UMOUNT_RLY);
  umountrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  umountrly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, umountrly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1 ; 
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_LSEEK64_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    +X if handled ok, valid FD returned.
 *    -X if error.
 */
// int lseek64(int fd, long long offset, int whence);
static int ps2netfs_op_lseek64(char *buf, int len)
{
  return -1;
}

/*! \brief Handles a PS2NETFS_DEVCTL_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    +X if handled ok, valid FD returned.
 *    -X if error.
 */
// int devctl(const char *name, int cmd, void *arg, size_t arglen, void *buf, size_t buflen);
static int ps2netfs_op_devctl(char *buf, int len)
{
  return -1;
}

/*! \brief Handles a PS2NETFS_SYMLINK_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    +X if handled ok, valid FD returned.
 *    -X if error.
 */
// int symlink(const char *old, const char *new);
static int ps2netfs_op_symlink(char *buf, int len)
{
  ps2netfs_pkt_symlink_req *cmd;
  ps2netfs_pkt_file_rly *symlinkrly;
  int retval = -ENODEV; 
  int devtype;
  cmd = (ps2netfs_pkt_symlink_req *)buf;

  dbgprintf("ps2netfs: symlink\n");

  if (len != sizeof(ps2netfs_pkt_symlink_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }

  // check that both are iomanx devices
  devtype = devscan_gettype(cmd->oldpath);
  if (devtype == IOPMGR_DEVTYPE_IOMANX)
  {
    devtype = devscan_gettype(cmd->newpath);
    if (devtype == IOPMGR_DEVTYPE_IOMANX)
      retval = symlink(cmd->oldpath,cmd->newpath);
    else retval = -1;
  }
  // now build the response
  symlinkrly = (ps2netfs_pkt_file_rly *)&ps2netfs_send_packet[0];

  // Build packet
  symlinkrly->cmd = htonl(PS2NETFS_SYMLINK_RLY);
  symlinkrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_file_rly));
  symlinkrly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, symlinkrly, sizeof(ps2netfs_pkt_file_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1 ; 
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_READLINK_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    +X if handled ok, valid FD returned.
 *    -X if error.
 */
// int readlink(const char *path, char *buf, size_t buflen);
static int ps2netfs_op_readlink(char *buf, int len)
{
  ps2netfs_pkt_open_req *cmd;
  ps2netfs_pkt_readlink_rly *readlinkrly;
  int retval = -ENODEV; 
  int devtype;
  cmd = (ps2netfs_pkt_open_req *)buf;

  dbgprintf("ps2netfs: readlink\n");

  if (len != sizeof(ps2netfs_pkt_open_req)) 
  {
    dbgprintf("ps2netfs: got a broken packet (%d)!\n", len);
    return -1;
  }

  // now build the response
  readlinkrly = (ps2netfs_pkt_readlink_rly *)&ps2netfs_send_packet[0];

  // do the stuff here
  devtype = devscan_gettype(cmd->path);
  if (devtype == IOPMGR_DEVTYPE_IOMANX)
    retval = readlink(cmd->path,readlinkrly->path,cmd->flags);


  // Build packet
  readlinkrly->cmd = htonl(PS2NETFS_READLINK_RLY);
  readlinkrly->len = htons((unsigned short)sizeof(ps2netfs_pkt_readlink_rly));
  readlinkrly->retval = htonl(retval);

  if (ps2netfs_lwip_send(ps2netfs_sock, readlinkrly, sizeof(ps2netfs_pkt_readlink_rly), 0) < 0) 
  {
    dbgprintf("ps2netfs: error sending reply!\n");
    return -1 ; 
  }
  return 0;
}

/*! \brief Handles a PS2NETFS_IOCTL2_CMD request.
 *  \ingroup ps2netfs 
 *
 *  \param buf Pointer to packet data.
 *  \param len Length of packet.
 *  \return Status.
 *
 *  status returns:
 *    0 if all request and response handled ok.
 *   -X if error.
 *
 *  return values for client:
 *    -19 (ENODEV) if device not found
 *    +X if handled ok, valid FD returned.
 *    -X if error.
 */
// int ioctl2(int fd, int cmd, void *arg, size_t arglen, void *buf, size_t buflen);
static int ps2netfs_op_ioctl2(char *buf, int len)
{
  return -1;
}


/*! \brief Listen for packets and handle them.
 *  \ingroup ps2netfs 
 *
 *  \param sock Socket to receive on.
 *
 *  This function listens on the given socket for command packets.
 *  It loops until a socket error, or protocol error is found, then
 *  exits to the calling function, which should clean the socket up.
 */
static void ps2netfs_Listener(int sock)
{
 int done;
 int len;
 unsigned int cmd;
 ps2netfs_pkt_hdr *header;
 int retval;

 done = 0;

 while(!done) {
   len = ps2netfs_accept_pktunknown(sock, &ps2netfs_recv_packet[0]);

   if (len > 0)
     dbgprintf("ps2netfs: received packet (%d)\n", len);

   if (len < 0) {
     dbgprintf("ps2netfs_Listener: recvfrom error (%d)\n", len);
     return;
   }
   if (len >= sizeof(ps2netfs_pkt_hdr)) 
   {
     header = (ps2netfs_pkt_hdr *)ps2netfs_recv_packet;
     cmd = ntohl(header->cmd);
     switch (cmd) 
     {
       case PS2NETFS_OPEN_CMD:
         retval = ps2netfs_op_open(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_CLOSE_CMD:
         retval = ps2netfs_op_close(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_READ_CMD:
         retval = ps2netfs_op_read(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_WRITE_CMD:
         retval = ps2netfs_op_write(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_LSEEK_CMD:
         retval = ps2netfs_op_lseek(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_IOCTL_CMD:
         retval = ps2netfs_op_ioctl(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_REMOVE_CMD:
         retval = ps2netfs_op_remove(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_MKDIR_CMD:
         retval = ps2netfs_op_mkdir(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_RMDIR_CMD:
         retval = ps2netfs_op_rmdir(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_DOPEN_CMD:
         retval = ps2netfs_op_dopen(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_DCLOSE_CMD:
         retval = ps2netfs_op_dclose(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_DREAD_CMD:
         retval = ps2netfs_op_dread(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_GETSTAT_CMD:
         retval = ps2netfs_op_getstat(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_CHSTAT_CMD:
         retval = ps2netfs_op_chstat(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_FORMAT_CMD:
         retval = ps2netfs_op_format(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_RENAME_CMD:
         retval = ps2netfs_op_rename(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_CHDIR_CMD:
         retval = ps2netfs_op_chdir(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_SYNC_CMD:
         retval = ps2netfs_op_sync(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_MOUNT_CMD:
         retval = ps2netfs_op_mount(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_UMOUNT_CMD:
         retval = ps2netfs_op_umount(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_LSEEK64_CMD:
         retval = ps2netfs_op_lseek64(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_DEVCTL_CMD:
         retval = ps2netfs_op_devctl(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_SYMLINK_CMD:
         retval = ps2netfs_op_symlink(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_READLINK_CMD:
         retval = ps2netfs_op_readlink(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_IOCTL2_CMD:
         retval = ps2netfs_op_ioctl2(ps2netfs_recv_packet, len);
         break;

       case PS2NETFS_INFO_CMD:
         retval = ps2netfs_op_info(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_FSTYPE_CMD:
         retval = ps2netfs_op_fstype(ps2netfs_recv_packet, len);
         break;
       case PS2NETFS_DEVLIST_CMD:
         retval = ps2netfs_op_devlist(ps2netfs_recv_packet, len);
         break;

       default: 
         dbgprintf("ps2netfs: Unknown cmd received\n");
         retval = 0;
         break;
      }
      if (retval == -1)
        return;
    } 
    else 
    {
      dbgprintf("ps2netfs: packet too small (%d)\n", len);
      return;
    }
    dbgprintf("ps2netfs: waiting for next pkt\n");
  }
}

/*! \brief Main ps2netfs Thread.
 *  \ingroup ps2netfs 
 *
 *  \param argv Parameters to thread.
 *  \return Status.
 *
 *  This is the main thread function for ps2netfs. It runs until 
 *  'ps2netfs_active' is set to 0.
 *  Handles accepting connections and passing them onto the listener
 *  function to handle them.
 *
 *  status returns:
 *    0 if exiting and finished.
 *   -1 if error starting thread.
 */
int
ps2netfs_serv(void *argv)
{
 struct sockaddr_in server_addr;
 struct sockaddr_in client_addr;
 int sock;
 int client_sock;
 int client_len;
 int ret;

 dbgprintf(" - ps2netfs TCP Server -\n");
    
 devscan_setup(DEVSCAN_MASK);
 fdh_setup();
  
 memset((void *)&server_addr, 0, sizeof(server_addr));
 // Should perhaps specify PC side ip..
 server_addr.sin_family = AF_INET;
 server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
 server_addr.sin_port = htons(PS2NETFS_LISTEN_PORT);

 while ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
 {
   dbgprintf("ps2netfs: socket creation error (%d)\n", sock);
   return -1;
 }

 ret = bind(sock, (struct sockaddr *)&server_addr, 
         sizeof(server_addr));
 if (ret < 0)  
 {
   dbgprintf("ps2netfs: bind error (%d)\n", ret);
   ps2netfs_close_socket();
   return -1;
 }

 ret = listen(sock, 5);

 if (ret < 0) 
 {
   dbgprintf("ps2netfs: listen error (%d)\n", ret);
   disconnect(sock);
   return -1;
 }

 // Active flag kinda sux, cause it wont be checked until a new client has
 // connected.. But it's better than nothing and good for now at least
 ps2netfs_active = 1;

 // Connection loop
 while(ps2netfs_active)
 {
   dbgprintf("ps2netfs: Waiting for connection\n");

   client_len = sizeof(client_addr);
   client_sock = accept(sock, (struct sockaddr *)&client_addr, 
                        &client_len);
   if (client_sock < 0) 
   {
     dbgprintf("ps2netfs: accept error (%d)", client_sock);
     continue;
   }

   dbgprintf("Client connected from %x\n", 
             client_addr.sin_addr.s_addr);

   if (ps2netfs_sock > 0) 
   {
     dbgprintf("ps2netfs: Client reconnected\n");
     ret = ps2netfs_close_socket();
     dbgprintf("ps2netfs: close ret %d\n", ret);
     continue;
   }
   ps2netfs_sock = client_sock;

   if (ps2netfs_sock > 0) 
   {
     ps2netfs_Listener(ps2netfs_sock);
     ret = ps2netfs_close_socket();
     dbgprintf("ps2netfs: close2 ret %d\n", ret);
     continue;
   }
 }

 if (ps2netfs_sock > 0)
   disconnect(ps2netfs_sock);

 disconnect(sock);

 ExitDeleteThread();
 return 0;
}

/*! \brief Init ps2netfs.
 *  \ingroup ps2netfs 
 *
 *  \return Status (0=success, -1 = failure). 
 *
 *  This handles setting up the Thread, and starting it running.
 *
 */
int ps2netfs_Init(void)
{
  iop_thread_t mythread;
  int pid;
  int i;

  dbgprintf("initializing ps2netfs\n");

  // Start socket server thread

  mythread.attr = 0x02000000; // attr
  mythread.option = 0; // option
  mythread.thread = (void *)ps2netfs_serv; // entry
  mythread.stacksize = 0x800;
  mythread.priority = 0x43; // just above ps2link

  pid = CreateThread(&mythread);

  if (pid > 0) 
  {
    if ((i=StartThread(pid, NULL)) < 0) 
    {
      printf("StartThread failed (%d)\n", i);
      return -1;
    }
  } 
  else 
  {
    printf("ps2netfs: CreateThread failed (%d)\n", pid);
    return -1;
  }
    
  ps2netfs_pid = pid;
  dbgprintf("ps2netfs: Thread id: %x\n", pid);
  return 0;
}

/*! \brief Shutdown ps2netfs.
 *  \ingroup ps2netfs 
 *
 *  This handles Stopping the thread and disconnecting. All shutdown
 *  for ps2netfs.
 *
 */
int ps2fs_Destroy(void)
{
  WaitSema(ps2netfs_sema);
  //    ExitDeleteThread(ps2netfs_pid);
  SignalSema(ps2netfs_sema);
  DeleteSema(ps2netfs_sema);
  return 0;
}


