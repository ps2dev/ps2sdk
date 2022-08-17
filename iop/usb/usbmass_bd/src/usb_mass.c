/*
 * usb_driver.c - USB Mass Storage Driver
 * See usbmass-ufi10.pdf and usbmassbulk_10.pdf
 */

#include <errno.h>
#include <stdio.h>
#include <sysclib.h>
#include <thbase.h>
#include <thsemap.h>
#include <usbd.h>
#include <usbd_macro.h>

#include "scsi.h"
#include <usbhdfsd-common.h>

// #define ASYNC
// #define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

#define USB_SUBCLASS_MASS_RBC       0x01
#define USB_SUBCLASS_MASS_ATAPI     0x02
#define USB_SUBCLASS_MASS_QIC       0x03
#define USB_SUBCLASS_MASS_UFI       0x04
#define USB_SUBCLASS_MASS_SFF_8070I 0x05
#define USB_SUBCLASS_MASS_SCSI      0x06

#define USB_PROTOCOL_MASS_CBI        0x00
#define USB_PROTOCOL_MASS_CBI_NO_CCI 0x01
#define USB_PROTOCOL_MASS_BULK_ONLY  0x50

#define USB_BLK_EP_IN  0
#define USB_BLK_EP_OUT 1

#define USB_XFER_MAX_RETRIES 8

#define CBW_TAG 0x43425355
#define CSW_TAG 0x53425355

typedef struct _mass_dev
{
    int controlEp;          // config endpoint id
    int bulkEpI;            // in endpoint id
    int bulkEpO;            // out endpoint id
    int devId;              // device id
    unsigned char configId; // configuration id
    unsigned char status;
    unsigned char interfaceNumber; // interface number
    unsigned char interfaceAlt;    // interface alternate setting
    int ioSema;
    struct scsi_interface scsi;
} mass_dev;

typedef struct _cbw_packet
{
    unsigned int signature;
    unsigned int tag;
    unsigned int dataTransferLength;
    unsigned char flags; // 80->data in,  00->out
    unsigned char lun;
    unsigned char comLength;   // command data length
    unsigned char comData[16]; // command data
} cbw_packet;

typedef struct _csw_packet
{
    unsigned int signature;
    unsigned int tag;
    unsigned int dataResidue;
    unsigned char status;
} csw_packet;

static sceUsbdLddOps driver;

typedef struct _usb_callback_data
{
    int sema;
    int returnCode;
    int returnSize;
} usb_callback_data;

#define USB_BLOCK_SIZE 4096 // Maximum single USB 1.1 transfer length.

typedef struct _usb_transfer_callback_data
{
    int sema;
    int pipe;
    u8 *buffer;
    int returnCode;
    unsigned int remaining;
} usb_transfer_callback_data;

#define NUM_DEVICES 2
static mass_dev g_mass_device[NUM_DEVICES];
static int usb_mass_update_sema;

static void usb_callback(int resultCode, int bytes, void *arg);
#ifndef ASYNC
static int perform_bulk_transfer(usb_transfer_callback_data *data);
static void usb_transfer_callback(int resultCode, int bytes, void *arg);
#endif
static void usb_mass_release(mass_dev *dev);

static void usb_callback(int resultCode, int bytes, void *arg)
{
    usb_callback_data *data = (usb_callback_data *)arg;
    data->returnCode        = resultCode;
    data->returnSize        = bytes;
    M_DEBUG("callback: res %d, bytes %d, arg %p \n", resultCode, bytes, arg);
    SignalSema(data->sema);
}

#ifndef ASYNC
static int perform_bulk_transfer(usb_transfer_callback_data *data)
{
    int ret, len;

    len = data->remaining > USB_BLOCK_SIZE ? USB_BLOCK_SIZE : data->remaining;
    ret = sceUsbdBulkTransfer(
        data->pipe,   // bulk pipe epI (Read) or epO (Write)
        data->buffer, // data ptr
        len,          // data length
        &usb_transfer_callback,
        (void *)data);
    return ret;
}

static void usb_transfer_callback(int resultCode, int bytes, void *arg)
{
    usb_transfer_callback_data *data = (usb_transfer_callback_data *)arg;

    data->returnCode = resultCode;
    if (resultCode == USB_RC_OK) { // Update transfer progress if successful.
        data->remaining -= bytes;
        data->buffer += bytes;
    }

    if ((resultCode == USB_RC_OK) && (data->remaining > 0)) { // OK to continue.
        int ret;

        ret = perform_bulk_transfer(data);
        if (ret != USB_RC_OK) {
            data->returnCode = ret;
            SignalSema(data->sema);
        }
    } else {
        SignalSema(data->sema);
    }
}
#endif

static int usb_set_configuration(mass_dev *dev, int configNumber)
{
    int ret;
    usb_callback_data cb_data;

    cb_data.sema = dev->ioSema;

    M_DEBUG("setting configuration controlEp=%i, confNum=%i \n", dev->controlEp, configNumber);
    ret = sceUsbdSetConfiguration(dev->controlEp, configNumber, usb_callback, (void *)&cb_data);

    if (ret == USB_RC_OK) {
        WaitSema(cb_data.sema);
        ret = cb_data.returnCode;
    }

    return ret;
}

static int usb_set_interface(mass_dev *dev, int interface, int altSetting)
{
    int ret;
    usb_callback_data cb_data;

    cb_data.sema = dev->ioSema;

    M_DEBUG("setting interface controlEp=%i, interface=%i altSetting=%i\n", dev->controlEp, interface, altSetting);
    ret = sceUsbdSetInterface(dev->controlEp, interface, altSetting, usb_callback, (void *)&cb_data);

    if (ret == USB_RC_OK) {
        WaitSema(cb_data.sema);
        ret = cb_data.returnCode;
    }

    return ret;
}

static int usb_bulk_clear_halt(mass_dev *dev, int endpoint)
{
    int ret;
    usb_callback_data cb_data;

    cb_data.sema = dev->ioSema;

    ret = sceUsbdClearEndpointFeature(
        dev->controlEp, // Config pipe
        0,              // HALT feature
        (endpoint == USB_BLK_EP_IN) ? dev->bulkEpI : dev->bulkEpO,
        usb_callback,
        (void *)&cb_data);

    if (ret == USB_RC_OK) {
        WaitSema(cb_data.sema);
        ret = cb_data.returnCode;
    }
    if (ret != USB_RC_OK) {
        M_DEBUG("ERROR: sending clear halt %d\n", ret);
    }

    return ret;
}

#ifndef ASYNC
static void usb_bulk_reset(mass_dev *dev, int mode)
{
    int ret;
    usb_callback_data cb_data;

    cb_data.sema = dev->ioSema;

    // Call Bulk only mass storage reset
    ret = sceUsbdControlTransfer(
        dev->controlEp, // default pipe
        0x21,           // bulk reset
        0xFF,
        0,
        dev->interfaceNumber, // interface number
        0,                    // length
        NULL,                 // data
        usb_callback,
        (void *)&cb_data);

    if (ret == USB_RC_OK) {
        WaitSema(cb_data.sema);
        ret = cb_data.returnCode;
    }
    if (ret == USB_RC_OK) {
        // clear bulk-in endpoint
        if (mode & 0x01)
            ret = usb_bulk_clear_halt(dev, USB_BLK_EP_IN);
    }
    if (ret == USB_RC_OK) {
        // clear bulk-out endpoint
        if (mode & 0x02)
            ret = usb_bulk_clear_halt(dev, USB_BLK_EP_OUT);
    }
    if (ret != USB_RC_OK) {
        M_DEBUG("ERROR: sending reset %d to device %d.\n", ret, dev->devId);
        dev->status |= USBMASS_DEV_STAT_ERR;
    }
}

static int usb_bulk_status(mass_dev *dev, csw_packet *csw, unsigned int tag)
{
    int ret;
    usb_callback_data cb_data;

    cb_data.sema = dev->ioSema;

    csw->signature   = CSW_TAG;
    csw->tag         = tag;
    csw->dataResidue = 0;
    csw->status      = 0;

    ret = sceUsbdBulkTransfer(
        dev->bulkEpI, // bulk input pipe
        csw,          // data ptr
        13,           // data length
        usb_callback,
        (void *)&cb_data);

    if (ret == USB_RC_OK) {
        WaitSema(cb_data.sema);
        ret = cb_data.returnCode;

#ifdef DEBUG
        if (cb_data.returnSize != 13)
            M_DEBUG("bulk csw.status returnSize: %i != 13\n", cb_data.returnSize);
        if (csw->dataResidue != 0)
            M_DEBUG("bulk csw.status residue: %i\n", csw->dataResidue);
        M_DEBUG("bulk csw result: %d, csw.status: %i\n", ret, csw->status);
#endif
    }

    return ret;
}

/* see flow chart in the usbmassbulk_10.pdf doc (page 15)

    Returned values:
        <0 Low-level USBD error.
        0 = Command completed successfully.
        1 = Command failed.
        2 = Phase error.
*/
static int usb_bulk_manage_status(mass_dev *dev, unsigned int tag)
{
    int ret;
    csw_packet csw;

    // XPRINTF("USBHDFSD: usb_bulk_manage_status 1 ...\n");
    ret = usb_bulk_status(dev, &csw, tag);       /* Attempt to read CSW from bulk in endpoint */
    if (ret != USB_RC_OK) {                      /* STALL bulk in  -OR- Bulk error */
        usb_bulk_clear_halt(dev, USB_BLK_EP_IN); /* clear the stall condition for bulk in */

        M_DEBUG("ERROR: usb_bulk_manage_status error %d ...\n", ret);
        ret = usb_bulk_status(dev, &csw, tag); /* Attempt to read CSW from bulk in endpoint */
    }

    /* CSW not valid  or stalled or phase error */
    if (ret != USB_RC_OK || csw.signature != CSW_TAG || csw.tag != tag || csw.status == 2) {
        M_DEBUG("ERROR: usb_bulk_manage_status call reset recovery ...\n");
        usb_bulk_reset(dev, 3); /* Perform reset recovery */
    }

    return ((ret == USB_RC_OK && csw.signature == CSW_TAG && csw.tag == tag) ? csw.status : -1);
}
#endif

static int usb_bulk_get_max_lun(struct scsi_interface *scsi)
{
    mass_dev *dev = (mass_dev *)scsi->priv;
    int ret;
    usb_callback_data cb_data;
    char max_lun;

    cb_data.sema = dev->ioSema;

    // Call Bulk only mass storage reset
    ret = sceUsbdControlTransfer(
        dev->controlEp, // default pipe
        0xA1,
        0xFE,
        0,
        dev->interfaceNumber, // interface number
        1,                    // length
        &max_lun,             // data
        usb_callback,
        (void *)&cb_data);

    if (ret == USB_RC_OK) {
        WaitSema(cb_data.sema);
        ret = cb_data.returnCode;
    }
    if (ret == USB_RC_OK) {
        ret = max_lun;
    } else {
        // Devices that do not support multiple LUNs may STALL this command.
        usb_bulk_clear_halt(dev, USB_BLK_EP_IN);
        usb_bulk_clear_halt(dev, USB_BLK_EP_OUT);

        ret = -ret;
    }

    return ret;
}

struct usbmass_cmd
{
    mass_dev *dev;

    cbw_packet cbw;
    csw_packet csw;
    int cmd_count;

    int returnCode;
};

#ifndef ASYNC
static int usb_bulk_command(mass_dev *dev, cbw_packet *packet)
{
    int ret;
    usb_callback_data cb_data;

    if (dev->status & USBMASS_DEV_STAT_ERR) {
        M_DEBUG("Rejecting I/O to offline device %d.\n", dev->devId);
        return -1;
    }

    cb_data.sema = dev->ioSema;

    ret = sceUsbdBulkTransfer(
        dev->bulkEpO, // bulk output pipe
        packet,       // data ptr
        31,           // data length
        usb_callback,
        (void *)&cb_data);

    if (ret == USB_RC_OK) {
        WaitSema(cb_data.sema);
        ret = cb_data.returnCode;
    }
    if (ret != USB_RC_OK) {
        M_DEBUG("ERROR: sending bulk command %d. Calling reset recovery.\n", ret);
        usb_bulk_reset(dev, 3);
    }

    return ret;
}

static int usb_bulk_transfer(mass_dev *dev, int direction, void *buffer, unsigned int transferSize)
{
    int ret;
    usb_transfer_callback_data cb_data;

    cb_data.sema       = dev->ioSema;
    cb_data.pipe       = (direction == USB_BLK_EP_IN) ? dev->bulkEpI : dev->bulkEpO;
    cb_data.buffer     = buffer;
    cb_data.returnCode = 0;
    cb_data.remaining  = transferSize;

    ret = perform_bulk_transfer(&cb_data);
    if (ret == USB_RC_OK) {
        WaitSema(cb_data.sema);
        ret = cb_data.returnCode;
    }

    if (ret != USB_RC_OK) {
        M_DEBUG("ERROR: bulk data transfer %d. Clearing HALT state.\n", ret);
        usb_bulk_clear_halt(dev, direction);
    }

    return ret;
}

#else

static void scsi_cmd_callback(int resultCode, int bytes, void *arg)
{
    struct usbmass_cmd *ucmd = (struct usbmass_cmd *)arg;
    mass_dev *dev            = (mass_dev *)ucmd->dev;

    M_DEBUG("%s, result=%d\n", __func__, resultCode);

    ucmd->cmd_count--;
    if ((resultCode != USB_RC_OK) || (ucmd->cmd_count == 0)) {
        // TODO: handle error in an async way (cannot block in usb callback)
        ucmd->returnCode = resultCode;
        SignalSema(dev->ioSema);
    }
}
#endif

int usb_queue_cmd(struct scsi_interface *scsi, const unsigned char *cmd, unsigned int cmd_len, unsigned char *data, unsigned int data_len, unsigned int data_wr)
{
    mass_dev *dev = (mass_dev *)scsi->priv;
    static struct usbmass_cmd ucmd;
    int result;
#ifndef ASYNC
    int rcode;
#endif
    static unsigned int tag = 0;

    M_DEBUG("%s\n", __func__);

    tag++;

    // Create USB command
    ucmd.dev       = dev;
    ucmd.cmd_count = 0;

    // Create CBW
    ucmd.cbw.signature          = CBW_TAG;
    ucmd.cbw.tag                = tag;
    ucmd.cbw.dataTransferLength = data_len;
    ucmd.cbw.flags              = data_wr ? 0 : 0x80;
    ucmd.cbw.lun                = 0;
    ucmd.cbw.comLength          = cmd_len;
    memcpy(ucmd.cbw.comData, cmd, cmd_len);

    // Create CSW
    ucmd.csw.signature   = CSW_TAG;
    ucmd.csw.tag         = tag;
    ucmd.csw.dataResidue = 0;
    ucmd.csw.status      = 0;

#ifndef ASYNC
    result = -EIO;
    if (usb_bulk_command(dev, &ucmd.cbw) == USB_RC_OK) {
        if (data_len > 0)
            rcode = usb_bulk_transfer(dev, data_wr ? USB_BLK_EP_OUT : USB_BLK_EP_IN, data, data_len);
        else
            rcode = USB_RC_OK;

        result = usb_bulk_manage_status(dev, tag);

        if (rcode != USB_RC_OK)
            result = -EIO;
    }

    return result;
#else
    // Send the CBW (command)
    ucmd.cmd_count++;
    result = sceUsbdBulkTransfer(dev->bulkEpO, &ucmd.cbw, 31, scsi_cmd_callback, (void *)&ucmd);
    if (result != USB_RC_OK)
        return -EIO;

    // Send/Receive data
    while (data_len > 0) {
        unsigned int tr_len = (data_len < USB_BLOCK_SIZE) ? data_len : USB_BLOCK_SIZE;
        ucmd.cmd_count++;
        result = sceUsbdBulkTransfer(data_wr ? dev->bulkEpO : dev->bulkEpI, data, tr_len, scsi_cmd_callback, (void *)&ucmd);
        if (result != USB_RC_OK)
            return -EIO;
        data_len -= tr_len;
        data += tr_len;
    }

    // Receive CSW (status)
    ucmd.cmd_count++;
    result = sceUsbdBulkTransfer(dev->bulkEpI, &ucmd.csw, 13, scsi_cmd_callback, (void *)&ucmd);
    if (result != USB_RC_OK)
        return -EIO;

    // Wait for SCSI command to finish
    WaitSema(dev->ioSema);
    if (ucmd.returnCode != USB_RC_OK)
        return -EIO;

    return 0;
#endif
}

static mass_dev *usb_mass_findDevice(int devId, int create)
{
    mass_dev *dev = NULL;
    int i;
    M_DEBUG("usb_mass_findDevice devId %i\n", devId);
    for (i = 0; i < NUM_DEVICES; ++i) {
        if (g_mass_device[i].devId == devId) {
            M_DEBUG("usb_mass_findDevice exists %i\n", i);
            dev = &g_mass_device[i];
            break;
        } else if (create && dev == NULL && g_mass_device[i].devId == -1) {
            dev = &g_mass_device[i];
            break;
        }
    }
    return dev;
}

/* test that endpoint is bulk endpoint and if so, update device info */
static void usb_bulk_probeEndpoint(int devId, mass_dev *dev, UsbEndpointDescriptor *endpoint)
{
    if (endpoint->bmAttributes == USB_ENDPOINT_XFER_BULK) {
        if ((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT && dev->bulkEpO < 0) {
            /* When sceUsbdOpenPipe() is used to work around the hardware errata that occurs when an unaligned memory address is specified,
               some USB devices become incompatible. Hence it is preferable to do alignment correction in software instead. */
            dev->bulkEpO = sceUsbdOpenPipeAligned(devId, endpoint);
            M_DEBUG("register Output endpoint id =%i addr=%02X packetSize=%i\n", dev->bulkEpO, endpoint->bEndpointAddress, (unsigned short int)endpoint->wMaxPacketSizeHB << 8 | endpoint->wMaxPacketSizeLB);
        } else
            /* Open this pipe with sceUsbdOpenPipe, to allow unaligned addresses to be used.
               According to the Sony documentation and the USBD code,
               there is always an alignment check if the pipe is opened with the sceUsbdOpenPipeAligned(),
               even when there is never any correction for the bulk out pipe. */
            if ((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN && dev->bulkEpI < 0) {
                dev->bulkEpI = sceUsbdOpenPipe(devId, endpoint);
                M_DEBUG("register Input endpoint id =%i addr=%02X packetSize=%i\n", dev->bulkEpI, endpoint->bEndpointAddress, (unsigned short int)endpoint->wMaxPacketSizeHB << 8 | endpoint->wMaxPacketSizeLB);
            }
    }
}

static int usb_mass_probe(int devId)
{
    UsbDeviceDescriptor *device  = NULL;
    UsbConfigDescriptor *config  = NULL;
    UsbInterfaceDescriptor *intf = NULL;

    M_DEBUG("probe: devId=%i\n", devId);

    mass_dev *mass_device = usb_mass_findDevice(devId, 0);

    /* only one device supported */
    if ((mass_device != NULL) && (mass_device->status & USBMASS_DEV_STAT_CONN)) {
        M_PRINTF("ERROR: Only one mass storage device allowed!\n");
        return 0;
    }

    /* get device descriptor */
    device = (UsbDeviceDescriptor *)sceUsbdScanStaticDescriptor(devId, NULL, USB_DT_DEVICE);
    if (device == NULL) {
        M_DEBUG("ERROR: Couldn't get device descriptor\n");
        return 0;
    }

    /* Check if the device has at least one configuration */
    if (device->bNumConfigurations < 1) {
        return 0;
    }

    /* read configuration */
    config = (UsbConfigDescriptor *)sceUsbdScanStaticDescriptor(devId, device, USB_DT_CONFIG);
    if (config == NULL) {
        M_DEBUG("ERROR: Couldn't get configuration descriptor\n");
        return 0;
    }
    /* check that at least one interface exists */
    M_DEBUG("bNumInterfaces %d\n", config->bNumInterfaces);
    if ((config->bNumInterfaces < 1) || (config->wTotalLength < (sizeof(UsbConfigDescriptor) + sizeof(UsbInterfaceDescriptor)))) {
        M_DEBUG("ERROR: No interfaces available\n");
        return 0;
    }
    /* get interface */
    intf = (UsbInterfaceDescriptor *)((char *)config + config->bLength); /* Get first interface */
    M_DEBUG("bInterfaceClass %X bInterfaceSubClass %X bInterfaceProtocol %X\n",
            intf->bInterfaceClass, intf->bInterfaceSubClass, intf->bInterfaceProtocol);

    if ((intf->bInterfaceClass != USB_CLASS_MASS_STORAGE) || (intf->bInterfaceSubClass != USB_SUBCLASS_MASS_SCSI && intf->bInterfaceSubClass != USB_SUBCLASS_MASS_SFF_8070I) || (intf->bInterfaceProtocol != USB_PROTOCOL_MASS_BULK_ONLY) || (intf->bNumEndpoints < 2)) { // one bulk endpoint is not enough because
        return 0;                                                                                                                                                                                                                                                         // we send the CBW to te bulk out endpoint
    }

    return 1;
}

static int usb_mass_connect(int devId)
{
    int i;
    int epCount;
    UsbDeviceDescriptor *device;
    UsbConfigDescriptor *config;
    UsbInterfaceDescriptor *interface;
    UsbEndpointDescriptor *endpoint;
    iop_sema_t SemaData;
    mass_dev *dev;

    M_PRINTF("connect: devId=%i\n", devId);
    dev = usb_mass_findDevice(devId, 1);

    if (dev == NULL) {
        M_PRINTF("ERROR: Unable to allocate space!\n");
        return 1;
    }

    /* only one mass device allowed */
    if (dev->devId != -1) {
        M_PRINTF("ERROR: Only one mass storage device allowed!\n");
        return 1;
    }

    dev->status  = 0;
    dev->bulkEpI = -1;
    dev->bulkEpO = -1;

    /* open the config endpoint */
    dev->controlEp = sceUsbdOpenPipe(devId, NULL);

    device = (UsbDeviceDescriptor *)sceUsbdScanStaticDescriptor(devId, NULL, USB_DT_DEVICE);

    config = (UsbConfigDescriptor *)sceUsbdScanStaticDescriptor(devId, device, USB_DT_CONFIG);

    interface = (UsbInterfaceDescriptor *)((char *)config + config->bLength); /* Get first interface */

    // store interface numbers
    dev->interfaceNumber = interface->bInterfaceNumber;
    dev->interfaceAlt    = interface->bAlternateSetting;

    epCount  = interface->bNumEndpoints;
    endpoint = (UsbEndpointDescriptor *)sceUsbdScanStaticDescriptor(devId, NULL, USB_DT_ENDPOINT);
    usb_bulk_probeEndpoint(devId, dev, endpoint);

    for (i = 1; i < epCount; i++) {
        endpoint = (UsbEndpointDescriptor *)((char *)endpoint + endpoint->bLength);
        usb_bulk_probeEndpoint(devId, dev, endpoint);
    }

    // Bail out if we do NOT have enough bulk endpoints.
    if (dev->bulkEpI < 0 || dev->bulkEpO < 0) {
        usb_mass_release(dev);
        M_PRINTF("ERROR: connect failed: not enough bulk endpoints!\n");
        return -1;
    }

    SemaData.initial = 0;
    SemaData.max     = 1;
    SemaData.option  = 0;
    SemaData.attr    = 0;
    if ((dev->ioSema = CreateSema(&SemaData)) < 0) {
        M_PRINTF("ERROR: Failed to allocate I/O semaphore\n");
        return -1;
    }

    /*store current configuration id - can't call set_configuration here */
    dev->configId = config->bConfigurationValue;
    dev->status   = USBMASS_DEV_STAT_CONN;
    // Set this last, with a memory barrier, in order to avoid a race condition in usb_mass_update with partially updated data
    __asm__ __volatile__("" : : : "memory");
    dev->devId    = devId;
    M_DEBUG("connect ok: epI=%i, epO=%i\n", dev->bulkEpI, dev->bulkEpO);

    SignalSema(usb_mass_update_sema);

    return 0;
}

static void usb_mass_release(mass_dev *dev)
{
    if (dev->bulkEpI >= 0)
        sceUsbdClosePipe(dev->bulkEpI);

    if (dev->bulkEpO >= 0)
        sceUsbdClosePipe(dev->bulkEpO);

    dev->bulkEpI   = -1;
    dev->bulkEpO   = -1;
    dev->controlEp = -1;
    dev->status    = 0;
}

static int usb_mass_disconnect(int devId)
{
    mass_dev *dev;
    dev = usb_mass_findDevice(devId, 0);

    M_PRINTF("disconnect: devId=%i\n", devId);

    if (dev == NULL) {
        M_PRINTF("ERROR: disconnect: no device storage!\n");
        return 0;
    }

    if (dev->status & USBMASS_DEV_STAT_CONN) {
        usb_mass_release(dev);
        dev->devId = -1;

        DeleteSema(dev->ioSema);

        // Should this move to the thread
        // just like the scsi_connect?
        scsi_disconnect(&dev->scsi);
    }

    return 0;
}

static void usb_mass_update(void *arg)
{
    int i;

    (void)arg;

    M_DEBUG("update thread running\n");

    while (1) {
        mass_dev *new_devs[NUM_DEVICES];
        int new_devs_count;

        // Wait for event from USBD thread
        WaitSema(usb_mass_update_sema);

        // Determine which devices are new and need to be connected.
        {
            new_devs_count = 0;
            for (i = 0; i < NUM_DEVICES; i += 1) {
                mass_dev *dev = &g_mass_device[i];
                if (dev->devId != -1 && (dev->status & USBMASS_DEV_STAT_CONN) && !(dev->status & USBMASS_DEV_STAT_CONF)) {
                    new_devs[new_devs_count] = dev;
                    new_devs_count += 1;
                }
            }
        }

        // Connect new devices
        for (i = 0; i < new_devs_count; i += 1) {
            mass_dev *dev = new_devs[i];
            {
                int ret;
                if ((ret = usb_set_configuration(dev, dev->configId)) != USB_RC_OK) {
                    M_PRINTF("ERROR: sending set_configuration %d\n", ret);
                    usb_mass_release(dev);
                    continue;
                }

                if ((ret = usb_set_interface(dev, dev->interfaceNumber, dev->interfaceAlt)) != USB_RC_OK) {
                    M_PRINTF("ERROR: sending set_interface %d\n", ret);
                    if (ret == USB_RC_STALL) {
                        /* USB Specification 1.1, section 9.4.10: Devices that only support a default setting for the specified interface may return a STALL.
                           As with Linux, we shall clear the halt state of the interface's pipes and continue. */
                        usb_bulk_clear_halt(dev, USB_BLK_EP_IN);
                        usb_bulk_clear_halt(dev, USB_BLK_EP_OUT);
                    } else {
                        usb_mass_release(dev);
                        continue;
                    }
                }

                dev->status |= USBMASS_DEV_STAT_CONF;
                scsi_connect(&dev->scsi);

                // This is the same wait amount as done in fat_getData in usbhdfsd.
                // This is a workaround to avoid incorrect initialization when attaching multiple drives at the same time.
                DelayThread(5000);
            }
        }
    }
}

int usb_mass_init(void)
{
    iop_thread_t thread;
    iop_sema_t sema;
    int ret;
    int i;

    M_DEBUG("%s\n", __func__);

    for (i = 0; i < NUM_DEVICES; ++i) {
        g_mass_device[i].status = 0;
        g_mass_device[i].devId  = -1;

        g_mass_device[i].scsi.priv = &g_mass_device[i];
        g_mass_device[i].scsi.name = "usb";
        // The maximum number of sectors should be 0xffff but
        // some usb drives seem to freeze above 128 sectors (64Kib)
        g_mass_device[i].scsi.max_sectors = 128; // 0xffff
        g_mass_device[i].scsi.get_max_lun = usb_bulk_get_max_lun;
        g_mass_device[i].scsi.queue_cmd   = usb_queue_cmd;
    }

    sema.attr            = 0;
    sema.option          = 0;
    sema.initial         = 0;
    sema.max             = 1;
    usb_mass_update_sema = CreateSema(&sema);

    driver.next       = NULL;
    driver.prev       = NULL;
    driver.name       = "mass-stor";
    driver.probe      = usb_mass_probe;
    driver.connect    = usb_mass_connect;
    driver.disconnect = usb_mass_disconnect;

    ret = sceUsbdRegisterLdd(&driver);
    M_DEBUG("sceUsbdRegisterLdd=%i\n", ret);
    if (ret < 0) {
        M_PRINTF("ERROR: register driver failed! ret=%d\n", ret);
        return -1;
    }

    thread.attr      = TH_C;
    thread.option    = 0;
    thread.thread    = usb_mass_update;
    thread.stacksize = 2 * 1024;
    thread.priority  = 0x10;

    ret = CreateThread(&thread);
    if (ret < 0) {
        M_PRINTF("ERROR: CreateThread failed! ret=%d\n", ret);
        return -1;
    }

    ret = StartThread(ret, 0);
    if (ret < 0) {
        M_PRINTF("ERROR: StartThread failed! ret=%d\n", ret);
        DeleteThread(ret);
        return -1;
    }

    return 0;
}
