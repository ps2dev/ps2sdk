#ifndef _SPB2_DISK_H_
#define _SPB2_DISK_H_


#include <tamtypes.h>
#include "scsi.h"


#define XFER_BLOCK_SIZE 65024 /* 65536-512. It seems like my Oxford 934 is capable of maximum transfers of less than 65536 bytes.     \
                      NOTE: I reduced the maximum block size here by 512, since transfers are done in (smallest) groups of 512 bytes! \
              */

#define READ_TRANSACTION  0
#define WRITE_TRANSACTION 1

/* Some macros */
#define CDB_PAGE_SIZE(v)          ((v) << 16) /* Not in use. */
#define CDB_PAGE_TABLE_PRESENT(v) ((v) << 19) /* Not in use. */
#define CDB_MAX_PAYLOAD(v)        ((v) << 20)
#define CDB_SPEED(v)              ((v) << 24)
#define CDB_DIRECTION(v)          ((v) << 27)
#define CDB_DATA_SIZE(v)          ((v))

/* Event flag bits. */
#define WRITE_REQ_INCOMING 1
#define READ_REQ_INCOMING  2
#define BUS_RESET_COMPLETE 4
#define ERROR_TIME_OUT     8

/* Management ORB opcodes */
#define SBP2_LOGIN_REQUEST        0x0
#define SBP2_QUERY_LOGINS_REQUEST 0x1
#define SBP2_RECONNECT_REQUEST    0x3
#define SBP2_SET_PASSWORD_REQUEST 0x4
#define SBP2_LOGOUT_REQUEST       0x7
#define SBP2_ABORT_TASK_REQUEST   0xb
#define SBP2_ABORT_TASK_SET       0xc
#define SBP2_LOGICAL_UNIT_RESET   0xe
#define SBP2_TARGET_RESET_REQUEST 0xf

/* Configuration ROM keys */
#define IEEE1394_CROM_LOGICAL_UNIT_NUM 0x14
#define IEEE1394_CROM_UNIT_CHARA       0x3A
#define IEEE1394_CROM_CSR_OFFSET       0x54
#define IEEE1394_CROM_UNIT_DIRECTORY   0xD1

/* Some macros. */
#define MANAGEMENT_ORB_LOGINID(v)   ((v))
#define MANAGEMENT_ORB_LUN(v)       ((v))
#define MANAGEMENT_ORB_FUNCTION(v)  ((v) << 16)
#define MANAGEMENT_ORB_RECONNECT(v) ((v) << 20)
#define MANAGEMENT_ORB_EXCLUSIVE(v) ((v) ? 1 << 28 : 0)

#define ORB_REQUEST_FORMAT(v) ((v) << 29)
#define ORB_NOTIFY            ((1) << 31)

#define MANAGEMENT_ORB_RESPONSE_LENGTH(v) ((v)&0x0000FFFF)
#define MANAGEMENT_ORB_PASSWORD_LENGTH(v) ((v) << 16)

#define NULL_POINTER (1 << 15)

#define RESP_SBP_STATUS(v) (((v) >> 16) & 0xFF)
#define RESP_LEN(v)        (((v) >> 24) & 0x07)
#define RESP_DEAD(v)       (((v) >> 27) & 0x01)
#define RESP_RESP(v)       (((v) >> 28) & 0x03)
#define RESP_SRC(v)        (((v) >> 30) & 0x03)

struct sbp2_status
{
    u32 status;
    u32 ORB_low;
    u8 data[24];
} __attribute__((packed));

struct sbp2_pointer
{
    u16 high;   /* Big endian. */
    u16 NodeID; /* Big endian. */
    u32 low;    /* Big endian. */
} __attribute__((packed));

struct sbp2_ORB_pointer
{
    u16 high;     /* Big endian. */
    u16 reserved; /* Big endian. */
    u32 low;      /* Big endian. */
} __attribute__((packed));

struct sbp2_login_response
{
    u16 login_ID; /* Big endian. */
    u16 length;   /* Big endian. */

    struct sbp2_pointer command_block_agent;
    u32 reconnect_hold; /* Big endian. */
} __attribute__((packed));

struct management_ORB
{
    union
    {
        u32 function_data[4];
        struct
        {
            struct sbp2_pointer password;
            struct sbp2_pointer response;
        } login;
    };
    u32 flags;
    union
    {
        u32 function_data2;
        u32 length;
    };
    struct sbp2_pointer status_FIFO;
} __attribute__((packed));

struct CommandDescriptorBlock
{
    struct sbp2_ORB_pointer NextOrb;
    struct sbp2_pointer DataDescriptor;

    u32 misc;
    unsigned char CDBs[12];
} __attribute__((packed));

struct SBP2Device
{
    int trContext;
    unsigned short int nodeID;
    unsigned short int InitiatorNodeID;
    unsigned short int max_payload;
    unsigned short int LUN;

    unsigned short int loginID; /* Stored in big endian. */

    unsigned char IsConnected;
    unsigned char mgt_ORB_timeout;
    unsigned char ORB_size;
    unsigned char speed;

    unsigned long int ManagementAgent_high;
    unsigned long int ManagementAgent_low;

    unsigned long int CommandBlockAgent_high;
    unsigned long int CommandBlockAgent_low;

    unsigned long int sectorSize;
    unsigned long int maxLBA;
    void *cache;

    struct scsi_interface scsi;
};

/* Function prototypes. */
void init_ieee1394DiskDriver(void);
int ieee1394_SendCommandBlockORB(struct SBP2Device *dev, struct CommandDescriptorBlock *firstCDB);
int ieee1394_Sync(void);
void DeinitIEEE1394(void);


#endif
