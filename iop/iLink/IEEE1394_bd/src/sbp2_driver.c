#include <intrman.h>
#include <loadcore.h>
#include <stdio.h>
#include <sysclib.h>
#include <sysmem.h>
#include <thbase.h>
#include <thevent.h>

#include "iLinkman.h"
#include "sbp2_disk.h"
#include "scsi.h"
#include "usbhdfsd-common.h"

//#define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

#define MAX_DEVICES 5
static struct SBP2Device SBP2Devices[MAX_DEVICES];
static u8 *writeBuffer = NULL;

/* Thread creation data. */
static iop_thread_t threadData = {
    TH_C,   /* attr */
    0,      /* option */
    NULL,   /* thread */
    0x1000, /* stacksize */
    0x30    /* priority */
};

//static unsigned long int iLinkBufferOffset, iLinkTransferSize;
static int sbp2_event_flag;

static void ieee1394_callback(int reason, unsigned long int offset, unsigned long int size);
static void iLinkIntrCBHandlingThread(void* arg);
static int ieee1394_SendManagementORB(int mode, struct SBP2Device* dev);
static int ieee1394_InitializeFetchAgent(struct SBP2Device* dev);
static inline int ieee1394_Sync_withTimeout(u32 n500mSecUnits);
void free(void* buffer);
void* malloc(int NumBytes);

static volatile union {
    struct sbp2_status status;
    unsigned char buffer[32]; /* Maximum 32 bytes. */
} statusFIFO;

static void ieee1394_callback(int reason, unsigned long int offset, unsigned long int size)
{
#if 0
	iLinkBufferOffset=offset;
	iLinkTransferSize=size;

	if(reason==iLink_CB_WRITE_REQUEST){
		u32 statusFIFO_result;

//		M_DEBUG("Write request: Offset: 0x%08lx; size: 0x%08lx.\n", iLinkBufferOffset, iLinkTransferSize);

		if(iLinkBufferOffset==(u32)&statusFIFO){	/* Check to be sure that we don't signal ieee1394_Sync() too early; Some transactions are transactions involving multiple ORBs. */
			statusFIFO_result=statusFIFO.status.status;
			if(RESP_SRC(statusFIFO_result)!=0) SetEventFlag(sbp2_event_flag, WRITE_REQ_INCOMING); /* Signal ieee1394_Sync() only after the last ORB was read. */
		}
	}
	else
#endif
    if (reason == iLink_CB_BUS_RESET) {
        SetEventFlag(sbp2_event_flag, BUS_RESET_COMPLETE);
    }
#if 0
	else{
		//M_DEBUG("Read request: Offset: 0x%08lx; size: 0x%08lx;\n", iLinkBufferOffset, iLinkTransferSize);
	}
#endif
}

/* Event flag creation data. */
static iop_event_t evfp = {
    2, /* evfp.attr=EA_MULTI */
    0, /* evfp.option */
    0  /* evfp.bits */
};

int iLinkIntrCBThreadID;
static int sbp2_get_max_lun(struct scsi_interface* scsi);
static int sbp2_queue_cmd(struct scsi_interface* scsi, const unsigned char* cmd, unsigned int cmd_len, unsigned char* data, unsigned int data_len, unsigned int data_wr);

void init_ieee1394DiskDriver(void)
{
    int i;

    for (i = 0; i < MAX_DEVICES; i++) {
        SBP2Devices[i].IsConnected = 0;
        SBP2Devices[i].nodeID      = 0;
        SBP2Devices[i].trContext   = (-1);

        SBP2Devices[i].scsi.priv        = &SBP2Devices[i];
        SBP2Devices[i].scsi.name        = "sd";
        SBP2Devices[i].scsi.max_sectors = XFER_BLOCK_SIZE / 512;
        SBP2Devices[i].scsi.get_max_lun = sbp2_get_max_lun;
        SBP2Devices[i].scsi.queue_cmd   = sbp2_queue_cmd;
    }

    writeBuffer = malloc(XFER_BLOCK_SIZE);

    sbp2_event_flag = CreateEventFlag(&evfp);

    M_DEBUG("Starting threads..\n");

    threadData.thread   = &iLinkIntrCBHandlingThread;
    iLinkIntrCBThreadID = CreateThread(&threadData);
    StartThread(iLinkIntrCBThreadID, NULL);

    iLinkSetTrCallbackHandler(&ieee1394_callback);

    M_DEBUG("Threads created and started.\n");

    iLinkEnableSBus();
}

static int initConfigureSBP2Device(struct SBP2Device* dev)
{
    unsigned char retries;

    retries = 10;
    do {
        if (ieee1394_SendManagementORB(SBP2_LOGIN_REQUEST, dev) < 0) {
            M_DEBUG("Error logging into the SBP-2 device.\n");
            retries--;
        } else
            break;
    } while (retries > 0);
    if (retries == 0) {
        M_DEBUG("Failed to log into the SBP-2 device.\n");
        return -1;
    }

    if (ieee1394_InitializeFetchAgent(dev) < 0) {
        M_DEBUG("Error initializing the SBP-2 device's Fetch Agent.\n");
        return -2;
    }

    scsi_connect(&dev->scsi);

    M_DEBUG("Completed device initialization.\n");

    /****** !!!FOR TESTING ONLY!!! ******/
#if 0
	iop_sys_clock_t lTime;
	u32 lSecStart, lUSecStart;
	u32 lSecEnd,   lUSecEnd, nbytes, i, rounds;
	void *buffer;
	int result;

	nbytes=1048576;
	rounds=64;
	if((buffer=malloc(nbytes))==NULL) M_PRINTF("Unable to allocate memory. :(\n");
	M_PRINTF("Read test: %p.\n", buffer);
	M_PRINTF("Start reading data...\n" );

	GetSystemTime ( &lTime );
	SysClock2USec ( &lTime, &lSecStart, &lUSecStart );

	for(i=0; i<rounds; i++){
		if((result=scsiReadSector(dev, 16, buffer, nbytes/512))!=0){
		    M_PRINTF("Sector read error %d.\n", result);
		    break;
		}
	}
	free(buffer);

	GetSystemTime ( &lTime );
	SysClock2USec ( &lTime, &lSecEnd, &lUSecEnd );

	M_PRINTF("Completed.\n");

	M_PRINTF( "Done: %lu %lu/%lu %lu\n", lSecStart, lUSecStart, lSecEnd, lUSecEnd );
	M_PRINTF("KB: %lu, time: %lu, Approximate KB/s: %lu", (nbytes*rounds/1024), (lSecEnd -lSecStart), (nbytes*rounds/1024)/(lSecEnd -lSecStart));
	/****** !!!FOR TESTING ONLY!!! ******/
#endif

    return 0;
}

static inline int initSBP2Disk(struct SBP2Device* dev)
{
    void* managementAgentAddr = NULL;
    unsigned int ConfigurationROMOffset, DirectorySize, i, LeafData, UnitDirectoryOffset;
    int result;

    UnitDirectoryOffset = 0;

    /* !! NOTE !! sce1394CrRead() reads data starting from the start of the target device's configuration ROM (0x400),
		and the offset and data to be read is specified in quadlets (Groups of 4 bytes)!!!!! */

    M_DEBUG("Reading configuration ROM for the unit directory offset of node 0x%08x\n", dev->nodeID);

    /* Offset in quadlets: 0x14 / 4 = 5 */
    ConfigurationROMOffset = 5;

    if ((result = iLinkReadCROM(dev->nodeID, ConfigurationROMOffset, 1, &LeafData)) < 0) {
        M_DEBUG("Error reading the configuration ROM. Error %d.\n", result);
        return (result);
    }

    ConfigurationROMOffset++;

    /* Get the Unit Directory's offset from the root directory. */
    DirectorySize = LeafData >> 16;
    for (i = 0; i < DirectorySize; i++) {
        if ((result = iLinkReadCROM(dev->nodeID, ConfigurationROMOffset, 1, &LeafData)) < 0) {
            M_DEBUG("Error reading the configuration ROM. Error %d.\n", result);
            return (result);
        }

        if ((LeafData >> 24) == IEEE1394_CROM_UNIT_DIRECTORY) {
            UnitDirectoryOffset = ConfigurationROMOffset + (LeafData & 0x00FFFFFF);
            break;
        }

        ConfigurationROMOffset++;
    }
    if (i == DirectorySize)
        return -1;

    if ((result = iLinkReadCROM(dev->nodeID, UnitDirectoryOffset, 1, &LeafData)) < 0) {
        M_DEBUG("Error reading the configuration ROM. Error %d.\n", result);
        return (result);
    }

    DirectorySize = LeafData >> 16;
    for (i = 0; i < DirectorySize; i++) {
        /* Get the offset in the configuration ROM of the unit record. */
        if ((result = iLinkReadCROM(dev->nodeID, UnitDirectoryOffset + 1 + i, 1, &LeafData)) < 0) {
            M_DEBUG("Error reading the configuration ROM. Error %d.\n", result);
            return (result);
        }

        switch (LeafData >> 24) {
        /* Now get the address of the Management Agent CSR. */
        case IEEE1394_CROM_CSR_OFFSET:
            managementAgentAddr = (void*)(LeafData & 0x00FFFFFF); /* Mask off the bits that represent the Management Agent key. */
            M_DEBUG("managementAgentAddr=0x%08lx.\n", (u32)managementAgentAddr * 4);

            dev->ManagementAgent_low  = (u32)managementAgentAddr * 4 + 0xf0000000;
            dev->ManagementAgent_high = 0x0000ffff;
            break;
        case IEEE1394_CROM_UNIT_CHARA:
            dev->mgt_ORB_timeout = (LeafData & 0x0000FF00) >> 8;
            dev->ORB_size        = LeafData & 0x000000FF;
            M_DEBUG("mgt_ORB_timeout=%u; ORB_size=%u.\n", dev->mgt_ORB_timeout, dev->ORB_size);
            break;
        case IEEE1394_CROM_LOGICAL_UNIT_NUM:
            dev->LUN = LeafData & 0x0000FFFF;
            M_DEBUG("LUN=0x%08x.\n", dev->LUN);
            break;
        }
    }

    return 1;
}

/* Hardware event handling threads. */
static void iLinkIntrCBHandlingThread(void* arg)
{
    int nNodes, i, targetDeviceID, nodeID, result;
    static const unsigned char PayloadSizeLookupTable[] = {
        7, /* S100; 2^(7+2)=512 */
        8, /* S200; 2^(8+2)=1024 */
        9, /* S400; 2^(9+2)=2048 */
    };

    while (1) {
        WaitEventFlag(sbp2_event_flag, BUS_RESET_COMPLETE, WEF_AND | WEF_CLEAR, NULL);

        /* Disconnect all currently connected nodes. */
        for (i = 0; i < MAX_DEVICES; i++) {
            if (SBP2Devices[i].trContext >= 0) {
                scsi_disconnect(&SBP2Devices[i].scsi);

                iLinkTrFree(SBP2Devices[i].trContext);
                SBP2Devices[i].trContext = -1;
            }
        }

        if ((nNodes = iLinkGetNodeCount()) < 0)
            M_DEBUG("Critical error: Failure getting the number of nodes!\n"); /* Error. */

        M_PRINTF("BUS RESET DETECTED. Nodes: %d\n", nNodes);
        M_PRINTF("Local Node: 0x%08x.\n", iLinkGetLocalNodeID());

        //DelayThread(500000); /* Give the devices on the bus some time to initialize themselves (The SBP-2 standard states that a maximum of 5 seconds may be given). */

        targetDeviceID = 0;

        for (i = 0; i < nNodes; i++) {
            if (targetDeviceID >= MAX_DEVICES) {
                M_DEBUG("Warning! There are more devices that what can be connected to.\n");
                break;
            }

            M_PRINTF("Attempting to initialize unit %d...\n", i);

            /* 16-bit node ID = BUS NUMBER (Upper 10 bits) followed by the NODE ID (Lower 6 bits). */
            if ((nodeID = iLinkFindUnit(i, 0x0609e, 0x010483)) < 0) {
                M_DEBUG("Error: Unit %d is not a valid SBP-2 device. Code: %d.\n", i, nodeID);
                continue;
            }

            M_PRINTF("Detected SBP-2 device.\n");

            SBP2Devices[targetDeviceID].InitiatorNodeID = iLinkGetLocalNodeID();

            M_PRINTF("Local Node: 0x%08x.\n", SBP2Devices[targetDeviceID].InitiatorNodeID);

#if 0
		if(SBP2Devices[targetDeviceID].IsConnected){ /* Already connected to the device. */
				if(SBP2Devices[targetDeviceID].nodeID==nodeID){	/* Make sure that we're attempting to re-connect to the same device. */
				if(ieee1394_SendManagementORB(SBP2_RECONNECT_REQUEST, &SBP2Devices[i])<0){
					M_DEBUG("Error reconnecting to the SBP-2 device %u.\n", nodeID);
				}
				else{
					M_DEBUG("Successfully reconnected to SBP-2 device %u.", nodeID);
					targetDeviceID++;
					continue;
				}
			}
			else SBP2Devices[targetDeviceID].IsConnected=0;
		}
#endif

            SBP2Devices[targetDeviceID].IsConnected = 0;

            /* Attempt a login into the device. */
            SBP2Devices[targetDeviceID].nodeID = nodeID;
            if ((result = initSBP2Disk(&SBP2Devices[targetDeviceID])) < 0) {
                M_DEBUG("Error initializing the device. Code: %d.\n", result);
                continue;
            }

            SBP2Devices[targetDeviceID].trContext = iLinkTrAlloc(nodeID, iLinkGetNodeMaxSpeed(nodeID));

            if (SBP2Devices[targetDeviceID].trContext >= 0) {
                M_PRINTF("Connected device as node 0x%08x.\n", SBP2Devices[targetDeviceID].nodeID);
                M_PRINTF("Generation number: %d.\n", iLinkGetGenerationNumber());

                SBP2Devices[targetDeviceID].speed = iLinkGetNodeTrSpeed(SBP2Devices[targetDeviceID].trContext);
                M_PRINTF("Current speed: %d.\n", SBP2Devices[targetDeviceID].speed);

                SBP2Devices[targetDeviceID].max_payload = PayloadSizeLookupTable[SBP2Devices[targetDeviceID].speed];

                if (initConfigureSBP2Device(&SBP2Devices[targetDeviceID]) >= 0) {
                    SBP2Devices[targetDeviceID].IsConnected = 1;
                    targetDeviceID++;
                }
            } else
                M_DEBUG("Error allocating a transaction.\n");
        }

        for (; targetDeviceID < MAX_DEVICES; targetDeviceID++)
            SBP2Devices[targetDeviceID].IsConnected = 0; /* Mark the unused device slots as being unused. */
    }
}

static void ieee1394_ResetFetchAgent(struct SBP2Device* dev)
{
    unsigned long int dummy;
    int result;

    /* Reset the Fetch Agent. */
    if ((result = iLinkTrWrite(dev->trContext, dev->CommandBlockAgent_high, dev->CommandBlockAgent_low + 0x04, &dummy, 4)) < 0) {
        M_DEBUG("Error writing to the Fetch Agent's AGENT_RESET register @ 0x%08lx %08lx. Code: %d.\n", dev->CommandBlockAgent_high, dev->CommandBlockAgent_low + 0x04, result);
    }
}

static int ieee1394_GetFetchAgentState(struct SBP2Device* dev)
{
    unsigned long int FetchAgentState;
    int result;

    if ((result = iLinkTrRead(dev->trContext, dev->CommandBlockAgent_high, dev->CommandBlockAgent_low, &FetchAgentState, 4)) < 0) {
        M_DEBUG("Error reading the Fetch Agent's AGENT_STATE register @ 0x%08lx %08lx. Code: %d.\n", dev->CommandBlockAgent_high, dev->CommandBlockAgent_low, result);
        return -1;
    } else {
        FetchAgentState = BSWAP32(FetchAgentState);
        M_DEBUG("Fetch Agent state: %lu.\n", FetchAgentState);
        return FetchAgentState;
    }
}

static int ieee1394_InitializeFetchAgent(struct SBP2Device* dev)
{
    int result, retries;
    void* dummy_ORB;
    struct sbp2_pointer address;

    M_DEBUG("Initializing fetch agent...");

    dummy_ORB = malloc(dev->ORB_size * 4);
    memset(dummy_ORB, 0, dev->ORB_size * 4);

    ieee1394_ResetFetchAgent(dev);

    /* Write the address of the dummy ORB to the fetch agent's ORB_POINTER register. */
    address.low    = (u32)dummy_ORB;
    address.NodeID = dev->InitiatorNodeID;
    address.high   = 0;

    ((struct sbp2_ORB_pointer*)dummy_ORB)->reserved = NULL_POINTER;
    ((struct sbp2_ORB_pointer*)dummy_ORB)->high = ((struct sbp2_ORB_pointer*)dummy_ORB)->low = 0;
    ((struct management_ORB*)dummy_ORB)->flags                                               = (u32)(ORB_NOTIFY | ORB_REQUEST_FORMAT(3));

    retries = 0;
    while ((result = iLinkTrWrite(dev->trContext, dev->CommandBlockAgent_high, dev->CommandBlockAgent_low + 0x08, &address, 8)) < 0) {
        M_DEBUG("Error writing to the Fetch Agent's ORB_POINTER register @ 0x%08lx %08lx. Code: %d.\n", dev->CommandBlockAgent_high, dev->CommandBlockAgent_low + 0x08, result);
        ieee1394_ResetFetchAgent(dev);

        retries++;
        if (retries > 3)
            return (-1);
    }

    result = ieee1394_Sync();
    ieee1394_GetFetchAgentState(dev);

    free(dummy_ORB);

    M_DEBUG("done!\n");

    return 1;
}

static int ieee1394_SendManagementORB(int mode, struct SBP2Device* dev)
{
    int result;
    struct sbp2_pointer address;

    struct sbp2_login_response login_result;
    struct management_ORB new_management_ORB;

    memset((void*)&statusFIFO, 0, sizeof(statusFIFO));
    memset(&login_result, 0, sizeof(struct sbp2_login_response));
    memset(&new_management_ORB, 0, sizeof(struct management_ORB));

    new_management_ORB.status_FIFO.low = (u32)&statusFIFO;
    new_management_ORB.flags           = (u32)(ORB_NOTIFY | ORB_REQUEST_FORMAT(0) | MANAGEMENT_ORB_FUNCTION(mode));

    switch (mode) {
    case SBP2_LOGIN_REQUEST: /* Login. */
        /* The reconnect timeout is now set to 0, since the mechanism that determines whether the initiator was already logged into the target or not was not accurate. */
        new_management_ORB.flags |= (u32)(MANAGEMENT_ORB_RECONNECT(0) | MANAGEMENT_ORB_EXCLUSIVE(1) | MANAGEMENT_ORB_LUN(dev->LUN));

        new_management_ORB.login.response.low = (u32)&login_result;
        new_management_ORB.length             = (MANAGEMENT_ORB_RESPONSE_LENGTH(sizeof(struct sbp2_login_response)) | MANAGEMENT_ORB_PASSWORD_LENGTH(0));
        break;
#if 0
		case SBP2_RECONNECT_REQUEST: /* Reconnect. */
			new_management_ORB.flags|=(u32)(MANAGEMENT_ORB_RECONNECT(4) | MANAGEMENT_ORB_LOGINID(dev->loginID)); /* loginID was stored as a big endian number, so it has to be converted into little endian first. :( */
			break;
#endif
    default:
        M_DEBUG("Warning! Unsupported management ORB type!\n");
    }

    address.NodeID = dev->InitiatorNodeID;
    address.high   = 0;
    address.low    = (u32)&new_management_ORB;

    M_DEBUG("Management ORB: %p. size=%d.\n", &new_management_ORB, sizeof(struct management_ORB));
    M_DEBUG("Address pointer: %p\n", &address);

    if ((result = iLinkTrWrite(dev->trContext, dev->ManagementAgent_high, dev->ManagementAgent_low, &address, 8)) < 0) { /* Write the Login ORB address to the target's Management Agent CSR. */
        M_DEBUG("Error writing to the Management Agent CSR @ 0x%08lx %08lx. Code: %d.\n", dev->ManagementAgent_high, dev->ManagementAgent_low, result);
        return (result);
    }

    M_DEBUG("Waiting for ILINK...\n");

    result = ieee1394_Sync_withTimeout(dev->mgt_ORB_timeout);

    M_DEBUG("statusFIFO= %p; [0]=%u.\n", &statusFIFO, statusFIFO.buffer[0]);
    if (result >= 0) {
        switch (mode) {
        case SBP2_LOGIN_REQUEST: /* Login. */
            M_DEBUG("Done. command_block_agent= 0x%08x %08lx.\n", login_result.command_block_agent.high, login_result.command_block_agent.low);

            /* Store the address to the target's command block agent. */

            dev->CommandBlockAgent_high = login_result.command_block_agent.high;
            dev->CommandBlockAgent_low  = login_result.command_block_agent.low;

            dev->loginID = login_result.login_ID; /* Store the login ID. */

            M_DEBUG("Login response size: %u.", login_result.length);
            M_DEBUG("Signed into SBP-2 device node 0x%08x. Login ID: %u\n", dev->nodeID, dev->loginID);
            break;
#if 0
			case SBP2_RECONNECT_REQUEST: /* Reconnect. */
					/* Just return the result from ieee1394_Sync(). */
				break;
#endif
        default:
            M_DEBUG("Warning! Unsupported management ORB type!\n");
        }

        /* Validate the fetch agent's CSR address. */
        if ((dev->CommandBlockAgent_low < 0xF0010000) || ((dev->CommandBlockAgent_high & 0xFFFF) != 0x0000FFFF))
            result = (-1);
    }

    return result;
}

int ieee1394_SendCommandBlockORB(struct SBP2Device* dev, struct CommandDescriptorBlock* firstCDB)
{
    int result, retries;
    struct sbp2_pointer address;

    memset((void*)&statusFIFO, 0, sizeof(statusFIFO));

    address.low    = (u32)firstCDB;
    address.NodeID = dev->InitiatorNodeID;
    address.high   = 0;

    retries = 0;

    /* Write the pointer to the first ORB in the fetch agent's NEXT_ORB register. */
    while ((result = iLinkTrWrite(dev->trContext, dev->CommandBlockAgent_high, dev->CommandBlockAgent_low + 0x08, &address, 8)) < 0) {
        M_DEBUG("Error writing to the Fetch Agent's ORB_POINTER register @ 0x%08lx %08lx. Code: %d.\n", dev->CommandBlockAgent_high, dev->CommandBlockAgent_low + 0x08, result);

        DelayThread(200000);
        ieee1394_ResetFetchAgent(dev);

        retries++;
        if (retries > 3)
            return (-1);
    }

#if 0
	/* Write a value to the fetch agent's DOORBELL register. */
	result=1;
	if((result=iLinkTrWrite(dev->trContext, dev->CommandBlockAgent_high, dev->CommandBlockAgent_low+0x10, &result, 4))<0){
		M_DEBUG("Error writing to the Fetch Agent's DOORBELL register @ 0x%08lx %08lx. Code: %d.\n", dev->CommandBlockAgent_high, dev->CommandBlockAgent_low+0x10, result);
		return -2;
	}
#endif

    return 1;
}

static int sbp2_get_max_lun(struct scsi_interface* scsi)
{
    return 0;
}

static int sbp2_queue_cmd(struct scsi_interface* scsi, const unsigned char* cmd, unsigned int cmd_len, unsigned char* data, unsigned int data_len, unsigned int data_wr)
{
    struct SBP2Device* dev = (struct SBP2Device*)scsi->priv;
    int i;
    int ret;
    struct CommandDescriptorBlock cdb;

    M_DEBUG("sbp2_queue_cmd(0x%02x)\n", cmd[0]);

    cdb.misc = ORB_NOTIFY | ORB_REQUEST_FORMAT(0) | CDB_MAX_PAYLOAD(dev->max_payload) | CDB_SPEED(dev->speed);
    cdb.misc |= data_wr ? CDB_DIRECTION(WRITE_TRANSACTION) : CDB_DIRECTION(READ_TRANSACTION);
    if (data_len > 0)
        cdb.misc |= CDB_DATA_SIZE(data_len);

    cdb.DataDescriptor.low    = data_wr ? (u32)writeBuffer : (u32)data;
    cdb.DataDescriptor.high   = 0;
    cdb.DataDescriptor.NodeID = dev->InitiatorNodeID;

    cdb.NextOrb.high     = 0;
    cdb.NextOrb.low      = 0;
    cdb.NextOrb.reserved = NULL_POINTER;

    // Copy and BSWAP32 the SCSI command
    for (i = 0; i < cmd_len / 4; i++)
        ((unsigned int*)cdb.CDBs)[i] = BSWAP32(((unsigned int*)cmd)[i]);

    if ((data_len > 0) && (data_wr == 1)) {
        // BSWAP32 all data we write
        for (i = 0; i < data_len / 4; i++)
            ((unsigned int*)writeBuffer)[i] = BSWAP32(((unsigned int*)data)[i]);
    }

    ieee1394_SendCommandBlockORB(dev, &cdb);
    ret = ieee1394_Sync();

    if (ret != 0) {
        M_DEBUG("sbp2_queue_cmd error %d\n", ret);
    } else if ((data_len > 0) && (data_wr == 0)) {
        // BSWAP32 all data we read
        for (i = 0; i < data_len / 4; i++)
            ((unsigned int*)data)[i] = BSWAP32(((unsigned int*)data)[i]);
    }

    return ret;
}

/* static unsigned int alarm_cb(void *arg){
	iSetEventFlag(sbp2_event_flag, ERROR_TIME_OUT);
	return 0;
} */

static int ProcessStatus(void)
{
    u32 statusFIFO_result;
    unsigned char status, resp, dead, sense, len;
    int result;

    statusFIFO_result = statusFIFO.status.status;
    status            = RESP_SBP_STATUS(statusFIFO_result);
    resp              = RESP_RESP(statusFIFO_result);
    dead              = RESP_DEAD(statusFIFO_result);
    len               = RESP_LEN(statusFIFO_result);
    sense             = ((statusFIFO.status.data[0]) >> 16) & 0xF;

    M_DEBUG("result: 0x%08lx; status: 0x%02x; RESP: 0x%02x; Dead: %u; len: 0x%02x; sense: 0x%02x.\n", statusFIFO_result, status, resp, dead, len, sense);
    if (sense == 0)
        sense = 1; /* Workaround for faulty firmwares: The sense code is 0 or was not sent when it's supposed to be sent. */

    result = (((status == 0) || (status == 0x0B)) && (resp == 0) && (!dead) && (len == 1)) ? 0 : -sense;

    if (result < 0) {
        M_PRINTF("result: 0x%08lx; status: 0x%02x; RESP: 0x%02x; Dead: %u; len: 0x%02x; sense: 0x%02x.\n", statusFIFO_result, status, resp, dead, len, sense);
    }

    return result;
}

/* This is declared as inline as there is only one call to it. */
static inline int ieee1394_Sync_withTimeout(u32 n500mSecUnits)
{
    /*	iop_sys_clock_t timeout_clk;
	u32 ef_result;

	USec2SysClock(n500mSecUnits*500000, &timeout_clk);

	SetAlarm(&timeout_clk, &alarm_cb, NULL);
	WaitEventFlag(sbp2_event_flag, WRITE_REQ_INCOMING|ERROR_TIME_OUT, WEF_OR|WEF_CLEAR, &ef_result);

	if(ef_result&ERROR_TIME_OUT){
		M_DEBUG("-=Time out=-\n");
		return(-1);
	}
	else CancelAlarm(&alarm_cb, NULL); */

    unsigned int i = 0;

    while (RESP_SRC(statusFIFO.status.status) == 0) {
        if (i > n500mSecUnits * 10000) {
            M_DEBUG("-=Time out=-\n");
            return (-1);
        }

        DelayThread(50);
        i++;
    };

    return (ProcessStatus());
}

int ieee1394_Sync(void)
{
    //	WaitEventFlag(sbp2_event_flag, WRITE_REQ_INCOMING, WEF_AND|WEF_CLEAR, NULL);

    while (RESP_SRC(statusFIFO.status.status) == 0) {
        DelayThread(50);
    };

    return (ProcessStatus());
}

void DeinitIEEE1394(void)
{
    TerminateThread(iLinkIntrCBThreadID);
    DeleteThread(iLinkIntrCBThreadID);
    DeleteEventFlag(sbp2_event_flag);
}

void* malloc(int NumBytes)
{
    int OldState;
    void* buffer;

    CpuSuspendIntr(&OldState);
    buffer = AllocSysMemory(ALLOC_FIRST, NumBytes, NULL);
    CpuResumeIntr(OldState);

    return buffer;
}

void free(void* buffer)
{
    int OldState;

    CpuSuspendIntr(&OldState);
    FreeSysMemory(buffer);
    CpuResumeIntr(OldState);
}
