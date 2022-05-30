/* Configuration ROM keys */
#define IEEE1394_CROM_DESC             0x01
#define IEEE1394_CROM_VENDOR           0x03
#define IEEE1394_CROM_NODE_CAPS        0x0C
#define IEEE1394_CROM_EUI64            0x0D
#define IEEE1394_CROM_UNIT             0x11
#define IEEE1394_CROM_UNIT_SPEC        0x12
#define IEEE1394_CROM_UNIT_SW_VERSION  0x13
#define IEEE1394_CROM_MODEL            0x17
#define IEEE1394_CROM_MODEL_ID         0x81
#define IEEE1394_CROM_NODE_UID         0x8D
#define IEEE1394_CROM_MODULE_VENDOR_ID 0xC3
#define IEEE1394_CROM_UNIT_DIRECTORY   0xD1

struct BusInformationBlockHeader
{
    unsigned char Bus_info_length;
    unsigned char CRC_length;
    unsigned short int ROM_CRC_value;
};

struct BusInformationBlock
{
    unsigned char BusName[4];
    unsigned char capabilities;
    unsigned char Cyc_Clk_Acc;
    unsigned char Max_Rec;
    unsigned char misc; /* g, resv. and link_spd fields. */

    unsigned int HardwareID; /* NodeVendorID | Chip_ID_High */
    unsigned int Chip_ID_Low;
};

struct DirectoryHeader
{
    unsigned short int Directory_length;
    unsigned short int CRC16;
};

struct Root_Directory
{
    unsigned int VendorID;
    unsigned int Module_Vendor_ID_Texual_Descriptor_Offset;
    unsigned int Node_Capabilities;
    unsigned int Node_Unique_ID_Offset;
    unsigned int Module_Vendor_ID_Offset;
};

struct Module_Vendor_ID_Texual_Descriptor
{
    unsigned int Specifier_ID;
    unsigned int Language_ID;
    unsigned char Vendor_Name[4]; /* "Sony" */
};

struct Module_Vendor_Id
{ /* For Playstation 2 consoles only? */
    unsigned int Textual_Descriptor;
};

struct ModelID_Textual_Descriptor
{
    unsigned int Specifier_ID;
    unsigned int Language_ID;
    unsigned char Model_Name[12]; /* E.g. "SCPH-10000" + 2x00-bytes at the end. */
};

struct Node_Unique_Id
{
    unsigned int HardwareID; /* Node_Vendor_ID | Chip_ID_High */
    unsigned int Chip_ID_Low;
};
