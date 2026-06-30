#ifndef _SCSI_H
#define _SCSI_H

struct scsi_interface
{
    void *priv;
    char *name;
    // cppcheck-suppress unusedStructMember
    unsigned int devNr;
    unsigned int max_sectors;

    int (*get_max_lun)(struct scsi_interface *scsi);
    int (*queue_cmd)(struct scsi_interface *scsi, const unsigned char *cmd, unsigned int cmd_len, unsigned char *data, unsigned int data_len, unsigned int data_wr);
};

extern int scsi_init(void);
extern void scsi_connect(struct scsi_interface *scsi);
extern void scsi_disconnect(struct scsi_interface *scsi);

enum {
    TEST_UNIT_READY   = 0x00,
    REQUEST_SENSE     = 0x03,
    INQUIRY           = 0x12,
    START_STOP_UNIT   = 0x1b,
    READ_CAPACITY_10  = 0x25,
    READ_10           = 0x28,
    WRITE_10          = 0x2a,
    READ_16           = 0x88,
    WRITE_16          = 0x8a,
    SERVICE_ACTION_IN = 0x9e, // READ_CAPACITY_16 and READ_LONG_16
};

#endif
