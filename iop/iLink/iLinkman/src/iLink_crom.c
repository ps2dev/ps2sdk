/*    iLink_crom.c
 *    Purpose:    Contains the functions related to configuration ROM access/management.
 *        !!!NOTE!!! All data accessible by external IEEE1394 nodes are usually in Big-Endian!!!
 *
 *    Last Updated:    2011/11/21
 *    Programmer:    SP193
 */

#include <stdio.h>
#include <sysclib.h>
#include <sysmem.h>
#include <thbase.h>

#include "iLinkman.h"
#include "iLink_CROM.h"
#include "iLink_internal.h"

extern u64 ConsoleGUID;
extern char ConsoleModelName[32];
extern unsigned short int LocalNodeID;
extern int nNodes;

static void *ExtraCROMUnits[16];
static unsigned int ExtraCROMUnitsSize[16];
static unsigned int nExtraCROMUnits;
unsigned int *ConfigurationROM;
unsigned int ConfigurationROMSize;
static unsigned char LinkSpeed, CycleClkAcc, Max_Rec;

extern int NodeCapabilities;
extern unsigned int NodeData[63];

/* Function prototypes. */
static void ieee1394Swab32(void *dest, void *src, unsigned int nQuads);
static void BuildConfigurationROM(void);

static void ieee1394Swab32(void *dest, void *src, unsigned int nQuads)
{
    register unsigned int i;

    for (i = 0; i < nQuads; i++)
        ((unsigned int *)dest)[i] = BSWAP32(((unsigned int *)src)[i]);
}

unsigned short int iLinkCalculateCRC16(void *data, unsigned int nQuads)
{
    int i, lShift;
    unsigned int lSum;
    unsigned short int lCRC = 0;

    for (i = 0; (unsigned int)i < nQuads; ++i) {
        unsigned int lData;

        lData = ((unsigned int *)data)[i];
        for (lShift = 28; lShift >= 0; lShift -= 4) {
            lSum = ((lCRC >> 12) ^ (lData >> lShift)) & 15;
            lCRC = (lCRC << 4) ^ (lSum << 12) ^ (lSum << 5) ^ lSum;
        } /* end for */
        lCRC &= 0xFFFF;
    } /* end for */

    return lCRC;
}

int iLinkAddCROMUnit(unsigned int *data, unsigned int nQuads)
{
    unsigned int i, CROM_EntrySizeInBytes;
    int result;

    result = -1002;
    for (i = 0; i < 16; i++) {
        if (ExtraCROMUnits[i] == NULL) {
            CROM_EntrySizeInBytes = nQuads * 4;
            if ((ExtraCROMUnits[i] = malloc(CROM_EntrySizeInBytes)) != NULL) {
                ExtraCROMUnitsSize[i] = CROM_EntrySizeInBytes;

                ieee1394Swab32(ExtraCROMUnits[i], data, nQuads);
                nExtraCROMUnits++;

                BuildConfigurationROM();

                result = i;
            }

            break;
        }
    }

    return result;
}

void iLinkDeleteCROMUnit(unsigned int id)
{
    free(ExtraCROMUnits[id]);
    ExtraCROMUnits[id] = NULL;
    BuildConfigurationROM();

    nExtraCROMUnits--;
}

int iLinkGetNodeCapabilities(unsigned short NodeID)
{
    int result;
    unsigned int data;

    if (NodeID == LocalNodeID)
        return NodeCapabilities;

    result = iLinkReadCROM(NodeID, 8, 1, &data);
    result = (result < 0) ? (-1) : (int)(data >> 27);

    return result;
}

int iLinkGetNodeMaxSpeed(unsigned short int NodeID)
{
    unsigned int i;
    int result;

    result = -1;
    for (i = 0; i < (unsigned int)nNodes; i++) {
        if (SELF_ID_NODEID(NodeData[i]) == (NodeID & 0x3F)) {
            result = SELF_ID_SPEED(NodeData[i]);
            break;
        }
    }

    return result;
}

static inline int ParseUnitCROM(unsigned short NodeID, unsigned int *UnitSpec, unsigned int *UnitSW_Version)
{
    unsigned int data, UnitDirectoryOffset, CROMOffset;
    int result, i;
    unsigned short int DirectoryLength;

    CROMOffset          = 0x14 >> 2;
    UnitDirectoryOffset = 0;

    /* Search the root directory for the unit directory key. */
    if ((result = iLinkReadCROM(NodeID, CROMOffset, 1, &data)) < 0)
        return (result);
    DirectoryLength = (unsigned short int)(data >> 16);

    CROMOffset++;

    for (i = 0; i < DirectoryLength; i++) {
        if ((result = iLinkReadCROM(NodeID, CROMOffset, 1, &data)) < 0)
            return (result);

        if ((data >> 24) == IEEE1394_CROM_UNIT_DIRECTORY) {
            UnitDirectoryOffset = data & 0x00FFFFFF;
            break;
        }

        CROMOffset++;
    }
    if (i == DirectoryLength)
        return -1;

    CROMOffset += UnitDirectoryOffset;
    DEBUG_PRINTF("CROMOffset=0x%08x.\n", CROMOffset);

    /* Search the unit directory for the unit spec and unit SW keys. */
    if ((result = iLinkReadCROM(NodeID, CROMOffset, 1, &data)) < 0)
        return result;
    DirectoryLength = (unsigned short int)(data >> 16);

    CROMOffset++;

    for (i = 0; i < DirectoryLength; i++) {
        if ((result = iLinkReadCROM(NodeID, CROMOffset, 1, &data)) < 0)
            return (result);

        CROMOffset++;

        if ((data >> 24) == IEEE1394_CROM_UNIT_SPEC) {
            *UnitSpec = data & 0x00FFFFFF;
        } else if ((data >> 24) == IEEE1394_CROM_UNIT_SW_VERSION) {
            *UnitSW_Version = data & 0x00FFFFFF;
        }

        if ((*UnitSpec != 0) && (*UnitSW_Version != 0))
            return 1;
    }

    return -1;
}

int iLinkFindUnit(int UnitInList, unsigned int UnitSpec, unsigned int UnitSW_Version)
{
    int result;
    unsigned int CurrentUnitSW_Version, CurrentUnitSpec;

    DEBUG_PRINTF("iLinkFindUnit() %d UnitSpec: 0x%08x; UnitSW Version: 0x%08x.\n", UnitInList, UnitSpec, UnitSW_Version);

    result = -1;
    if (UnitInList < nNodes) {
        unsigned short int NodeID;

        NodeID                = (unsigned short int)SELF_ID_NODEID(NodeData[UnitInList]) | (LocalNodeID & 0xFFC0);
        CurrentUnitSW_Version = CurrentUnitSpec = 0;

        if ((result = ParseUnitCROM(NodeID, &CurrentUnitSpec, &CurrentUnitSW_Version)) >= 0) {
            result = ((CurrentUnitSW_Version == UnitSW_Version) && (CurrentUnitSpec == UnitSpec)) ? NodeID : (-1);
        }
    }

    return result;
}

int iLinkReadCROM(unsigned short int NodeID, unsigned int Offset, unsigned int nQuads, unsigned int *buffer)
{
    unsigned int i;
    int result, trContext;

    DEBUG_PRINTF("Reading CROM of node 0x%04x, offset: %u, nquads: %u.\n", NodeID, Offset, nQuads);

    result = -1;
    if ((trContext = iLinkTrAlloc(NodeID, S100)) < 0)
        return trContext;

    for (i = 0; i < nQuads; i++) {
        int retries;

        retries = 10;
        do {
            if ((result = iLinkTrRead(trContext, 0x0000FFFF, 0xF0000400 + (Offset << 2), buffer, 4)) >= 0) {
                *buffer = BSWAP32(*buffer);
                buffer++;
                Offset++;
            } else {
                DelayThread(100000);
                retries--;
            }
        } while ((result < 0) && (retries > 0));
        if (retries == 0)
            break;
    }

    iLinkTrFree(trContext);

    return result;
}

void InitializeConfigurationROM(void)
{
    register unsigned int i;

    for (i = 0; i < 16; i++)
        ExtraCROMUnits[i] = NULL;
    nExtraCROMUnits  = 0;
    ConfigurationROM = NULL;

    LinkSpeed   = 4;
    CycleClkAcc = ~0;
    Max_Rec     = 10;

    BuildConfigurationROM();
}

static void BuildConfigurationROM(void)
{
    unsigned char *CROM_Buffer;
    unsigned int CROMSize, CurrentOffset;

    struct BusInformationBlock *BusInfoBlk;
    struct Root_Directory *RootDirectory;
    struct Module_Vendor_Id *ModuleVendorID;
    struct DirectoryHeader *DirectoryHeader;
    struct Module_Vendor_ID_Texual_Descriptor *ModuleTexualDescriptor;
    struct Node_Unique_Id *NodeUniqueID;
    struct ModelID_Textual_Descriptor *ModelName;

    unsigned int i, TotalExtraCROMUnitSize;
    unsigned short int TotalRootDirectorySizeInQuads;

    TotalExtraCROMUnitSize = 0;
    for (i = 0; i < 16; i++)
        if (ExtraCROMUnits[i] != NULL)
            TotalExtraCROMUnitSize += ExtraCROMUnitsSize[i];

    TotalRootDirectorySizeInQuads = (sizeof(struct Root_Directory) + TotalExtraCROMUnitSize) / 4;

    CROMSize = sizeof(struct BusInformationBlockHeader) + sizeof(struct BusInformationBlock);
    CROMSize = CROMSize + sizeof(struct DirectoryHeader) + sizeof(struct Root_Directory) + TotalExtraCROMUnitSize;
    CROMSize = CROMSize + sizeof(struct DirectoryHeader) + sizeof(struct Module_Vendor_ID_Texual_Descriptor);
    CROMSize = CROMSize + sizeof(struct DirectoryHeader) + sizeof(struct Module_Vendor_Id);
    CROMSize = CROMSize + sizeof(struct DirectoryHeader) + sizeof(struct ModelID_Textual_Descriptor);
    CROMSize = CROMSize + sizeof(struct DirectoryHeader) + sizeof(struct Node_Unique_Id);

    ConfigurationROMSize = CROMSize;

    if ((CROM_Buffer = malloc(CROMSize)) == NULL)
        return;

    CurrentOffset = sizeof(struct BusInformationBlockHeader) + sizeof(struct BusInformationBlock);
    /* Fill in the fields in the Root Directory (Exists immediately after the Bus Information block). */
    DirectoryHeader                                          = (struct DirectoryHeader *)&CROM_Buffer[CurrentOffset];
    RootDirectory                                            = (struct Root_Directory *)((unsigned char *)DirectoryHeader + sizeof(struct DirectoryHeader));
    RootDirectory->Module_Vendor_ID_Texual_Descriptor_Offset = (IEEE1394_CROM_VENDOR << 24) | (sizeof(struct DirectoryHeader) * 2 / 4 + TotalRootDirectorySizeInQuads - 2 + sizeof(struct Module_Vendor_Id) / 4); /* Calculate the relative offset (In quadlets!!). */
    RootDirectory->Node_Capabilities                         = (IEEE1394_CROM_NODE_CAPS << 24) | 0x0C0083C0;
    RootDirectory->Node_Unique_ID_Offset                     = (IEEE1394_CROM_NODE_UID << 24) | (sizeof(struct DirectoryHeader) * 3 / 4 + TotalRootDirectorySizeInQuads - 2 + sizeof(struct Module_Vendor_Id) / 4 + sizeof(struct Module_Vendor_ID_Texual_Descriptor) / 4);
    RootDirectory->Module_Vendor_ID_Offset                   = (IEEE1394_CROM_MODULE_VENDOR_ID << 24) | (nExtraCROMUnits + 1);

    ieee1394Swab32(RootDirectory, RootDirectory, sizeof(struct Root_Directory) / 4); /* Convert this block of data to Big-endian data. */

    CurrentOffset = CurrentOffset + sizeof(struct DirectoryHeader) + sizeof(struct Root_Directory);

    for (i = 0; i < 16; i++) {
        if (ExtraCROMUnits[i] != NULL) {
            memcpy(&CROM_Buffer[CurrentOffset], ExtraCROMUnits[i], ExtraCROMUnitsSize[i]); /* Append the extra records to the end of the Root Directory. */
            CurrentOffset += ExtraCROMUnitsSize[i];
        }
    }

    DirectoryHeader->Directory_length = TotalRootDirectorySizeInQuads;
    DirectoryHeader->CRC16            = BSWAP16(iLinkCalculateCRC16(RootDirectory, DirectoryHeader->Directory_length));
    DirectoryHeader->Directory_length = BSWAP16(DirectoryHeader->Directory_length);

    /* Fill in the fields of the Module Vendor ID record. */
    ModuleVendorID                     = (struct Module_Vendor_Id *)&CROM_Buffer[CurrentOffset + sizeof(struct DirectoryHeader)];
    ModuleVendorID->Textual_Descriptor = (IEEE1394_CROM_MODEL_ID << 24) | ((sizeof(struct DirectoryHeader) * 2 / 4 + sizeof(struct Module_Vendor_ID_Texual_Descriptor) + sizeof(struct Node_Unique_Id)) / 4);

    ieee1394Swab32(ModuleVendorID, ModuleVendorID, sizeof(struct Module_Vendor_Id) / 4); /* Convert this block of data to Big-endian data. */

    DirectoryHeader                   = (struct DirectoryHeader *)&CROM_Buffer[CurrentOffset];
    DirectoryHeader->Directory_length = 1;
    DirectoryHeader->CRC16            = BSWAP16(iLinkCalculateCRC16(ModuleVendorID, DirectoryHeader->Directory_length));
    DirectoryHeader->Directory_length = BSWAP16(DirectoryHeader->Directory_length);

    CurrentOffset = CurrentOffset + sizeof(struct Module_Vendor_Id) + sizeof(struct DirectoryHeader);

    /* Fill in the fields of the Textual_Descriptor */
    DirectoryHeader                      = (struct DirectoryHeader *)&CROM_Buffer[CurrentOffset];
    ModuleTexualDescriptor               = (struct Module_Vendor_ID_Texual_Descriptor *)((unsigned char *)DirectoryHeader + sizeof(struct DirectoryHeader));
    ModuleTexualDescriptor->Specifier_ID = 0x00000000;
    ModuleTexualDescriptor->Language_ID  = 0x00000000;

    ieee1394Swab32(ModuleTexualDescriptor, ModuleTexualDescriptor, sizeof(struct Module_Vendor_ID_Texual_Descriptor) / 4); /* Convert this block of data to Big-endian data. */

    memcpy(ModuleTexualDescriptor->Vendor_Name, "Sony", 4); /* Don't flop the "Sony" text. */

    DirectoryHeader->Directory_length = 3;
    DirectoryHeader->CRC16            = BSWAP16(iLinkCalculateCRC16(ModuleTexualDescriptor, DirectoryHeader->Directory_length));
    DirectoryHeader->Directory_length = BSWAP16(DirectoryHeader->Directory_length);

    CurrentOffset = CurrentOffset + sizeof(struct Module_Vendor_ID_Texual_Descriptor) + sizeof(struct DirectoryHeader);

    /* Fill in the fields in the Node Unique ID section. */
    DirectoryHeader = (struct DirectoryHeader *)&CROM_Buffer[CurrentOffset];
    NodeUniqueID    = (struct Node_Unique_Id *)((unsigned char *)DirectoryHeader + sizeof(struct DirectoryHeader));

    NodeUniqueID->HardwareID  = BSWAP32((ConsoleGUID >> 32));
    NodeUniqueID->Chip_ID_Low = BSWAP32((ConsoleGUID & 0xFFFFFFFF));

    DirectoryHeader->Directory_length = sizeof(struct Node_Unique_Id) / 4;
    DirectoryHeader->CRC16            = BSWAP16(iLinkCalculateCRC16(NodeUniqueID, DirectoryHeader->Directory_length));
    DirectoryHeader->Directory_length = BSWAP16(DirectoryHeader->Directory_length);

    CurrentOffset = CurrentOffset + sizeof(struct DirectoryHeader) + sizeof(struct Node_Unique_Id);

    /* Fill in the fields in the modelname block. */
    DirectoryHeader = (struct DirectoryHeader *)&CROM_Buffer[CurrentOffset];
    ModelName       = (struct ModelID_Textual_Descriptor *)((unsigned char *)DirectoryHeader + sizeof(struct DirectoryHeader));

    ModelName->Specifier_ID = 0x00000000;
    ModelName->Language_ID  = 0x00000000;

    memcpy(ModelName->Model_Name, ConsoleModelName, sizeof(ModelName->Model_Name));

    DirectoryHeader->Directory_length = sizeof(struct ModelID_Textual_Descriptor) / 4;
    DirectoryHeader->CRC16            = BSWAP16(iLinkCalculateCRC16(ModelName, DirectoryHeader->Directory_length));
    DirectoryHeader->Directory_length = BSWAP16(DirectoryHeader->Directory_length);

    CurrentOffset = sizeof(struct BusInformationBlockHeader);

    /* Fill in the fields in the Bus Information Block. */
    BusInfoBlk = (struct BusInformationBlock *)&CROM_Buffer[CurrentOffset];
    memcpy(BusInfoBlk->BusName, "1394", 4);
    BusInfoBlk->capabilities = NodeCapabilities << 3;
    BusInfoBlk->Cyc_Clk_Acc  = CycleClkAcc;
    BusInfoBlk->Max_Rec      = Max_Rec << 4;
    BusInfoBlk->misc         = LinkSpeed;
    BusInfoBlk->HardwareID   = NodeUniqueID->HardwareID;
    BusInfoBlk->Chip_ID_Low  = NodeUniqueID->Chip_ID_Low;

    /* Fill in the fields in the Bus Information Block Header. */
    ((struct BusInformationBlockHeader *)CROM_Buffer)->Bus_info_length = 4; /* According to the standard. */
    ((struct BusInformationBlockHeader *)CROM_Buffer)->CRC_length      = sizeof(struct BusInformationBlock) / 4;
    ((struct BusInformationBlockHeader *)CROM_Buffer)->ROM_CRC_value   = BSWAP16(iLinkCalculateCRC16(BusInfoBlk, sizeof(struct BusInformationBlock) / 4));

    if (ConfigurationROM != NULL)
        free(ConfigurationROM);
    ConfigurationROM = (unsigned int *)CROM_Buffer;
}
