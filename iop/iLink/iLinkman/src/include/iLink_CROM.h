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
    u16 ROM_CRC_value;
    u8  CRC_length;
    u8  Bus_info_length;
};

struct BusInformationBlock
{
    u32 BusName;
    u8  misc; /* g, resv. and link_spd fields. */
    u8  Max_Rec;
    u8  Cyc_Clk_Acc;
    u8  capabilities;

    u32 HardwareID; /* NodeVendorID | Chip_ID_High */
    u32 Chip_ID_Low;
};

struct DirectoryHeader
{
    u16 CRC16;
    u16 Directory_length;
};

struct Root_Directory
{
    u32 VendorID;
    u32 Module_Vendor_ID_Texual_Descriptor_Offset;
    u32 Node_Capabilities;
    u32 Node_Unique_ID_Offset;
    u32 Module_Vendor_ID_Offset;
};

struct Module_Vendor_ID_Texual_Descriptor
{
    u32 Specifier_ID;
    u32 Language_ID;
    u32 Vendor_Name;
};

struct Module_Vendor_Id
{ /* For Playstation 2 consoles only? */
    u32 Textual_Descriptor;
};

struct ModelID_Textual_Descriptor
{
    u32 Specifier_ID;
    u32 Language_ID;
    u8  Model_Name[12]; /* E.g. "SCPH-10000" + 2x00-bytes at the end. */
};

struct Node_Unique_Id
{
    u32 HardwareID; /* Node_Vendor_ID | Chip_ID_High */
    u32 Chip_ID_Low;
};
