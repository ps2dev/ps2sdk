#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include <sifrpc.h>

#include "libsecr.h"
#include "secrsif.h"

static SifRpcClientData_t SifRpcClient01;
static SifRpcClientData_t SifRpcClient02;
static SifRpcClientData_t SifRpcClient03;
static SifRpcClientData_t SifRpcClient04;
static SifRpcClientData_t SifRpcClient05;
static SifRpcClientData_t SifRpcClient06;
static SifRpcClientData_t SifRpcClient07;

static unsigned char RpcBuffer[0x1000] ALIGNED(64);

#define _printf(args...) // printf(args)

int SecrInit(void)
{
    sceSifInitRpc(0);

    nopdelay();
    while (sceSifBindRpc(&SifRpcClient01, 0x80000A01, 0) < 0 || SifRpcClient01.server == NULL) {
        _printf("libsecr: bind failed\n");
    }

    nopdelay();
    while (sceSifBindRpc(&SifRpcClient02, 0x80000A02, 0) < 0 || SifRpcClient02.server == NULL) {
        _printf("libsecr: bind failed\n");
    }

    nopdelay();
    while (sceSifBindRpc(&SifRpcClient03, 0x80000A03, 0) < 0 || SifRpcClient03.server == NULL) {
        _printf("libsecr: bind failed\n");
    }

    nopdelay();
    while (sceSifBindRpc(&SifRpcClient04, 0x80000A04, 0) < 0 || SifRpcClient04.server == NULL) {
        _printf("libsecr: bind failed\n");
    }

    nopdelay();
    while (sceSifBindRpc(&SifRpcClient05, 0x80000A05, 0) < 0 || SifRpcClient05.server == NULL) {
        _printf("libsecr: bind failed\n");
    }

    nopdelay();
    while (sceSifBindRpc(&SifRpcClient06, 0x80000A06, 0) < 0 || SifRpcClient06.server == NULL) {
        _printf("libsecr: bind failed\n");
    }

    nopdelay();
    while (sceSifBindRpc(&SifRpcClient07, 0x80000A07, 0) < 0 || SifRpcClient07.server == NULL) {
        _printf("libsecr: bind failed\n");
    }

    return 1;
}

void SecrDeinit(void)
{
    memset(&SifRpcClient01, 0, sizeof(SifRpcClientData_t));
    memset(&SifRpcClient02, 0, sizeof(SifRpcClientData_t));
    memset(&SifRpcClient03, 0, sizeof(SifRpcClientData_t));
    memset(&SifRpcClient04, 0, sizeof(SifRpcClientData_t));
    memset(&SifRpcClient05, 0, sizeof(SifRpcClientData_t));
    memset(&SifRpcClient06, 0, sizeof(SifRpcClientData_t));
    memset(&SifRpcClient07, 0, sizeof(SifRpcClientData_t));
}

int SecrDownloadHeader(int port, int slot, void *buffer, SecrBitTable_t *BitTable, s32 *pSize)
{
    int result;

    ((struct SecrSifDownloadHeaderParams *)RpcBuffer)->port = port;
    ((struct SecrSifDownloadHeaderParams *)RpcBuffer)->slot = slot;
    memcpy(((struct SecrSifDownloadHeaderParams *)RpcBuffer)->buffer, buffer, sizeof(((struct SecrSifDownloadHeaderParams *)RpcBuffer)->buffer));

    if (sceSifCallRpc(&SifRpcClient01, 1, 0, RpcBuffer, sizeof(RpcBuffer), RpcBuffer, sizeof(RpcBuffer), NULL, NULL) < 0) {
        _printf("sceSecrDownloadHeader: rpc error\n");
        result = 0;
    } else {
        memcpy(BitTable, &((struct SecrSifDownloadHeaderParams *)RpcBuffer)->BitTable, ((struct SecrSifDownloadHeaderParams *)RpcBuffer)->size);
        // BUG: pSize doesn't seem to be filled in within the Sony original.
        if (pSize != NULL)
            *pSize = ((struct SecrSifDownloadHeaderParams *)RpcBuffer)->size;
        result = ((struct SecrSifDownloadHeaderParams *)RpcBuffer)->result;
    }

    return result;
}

int SecrDownloadBlock(void *src, unsigned int size)
{
    int result;

    memcpy(((struct SecrSifDownloadBlockParams *)RpcBuffer)->buffer, src, sizeof(((struct SecrSifDownloadBlockParams *)RpcBuffer)->buffer));
    ((struct SecrSifDownloadBlockParams *)RpcBuffer)->size = size;

    if (sceSifCallRpc(&SifRpcClient02, 1, 0, RpcBuffer, sizeof(RpcBuffer), RpcBuffer, sizeof(RpcBuffer), NULL, NULL) < 0) {
        _printf("sceSecrDownloadBlock: rpc error\n");
        result = 0;
    } else {
        result = ((struct SecrSifDownloadBlockParams *)RpcBuffer)->result;
    }

    return result;
}

int SecrDownloadGetKbit(int port, int slot, void *kbit)
{
    int result;

    ((struct SecrSifDownloadGetKbitParams *)RpcBuffer)->port = port;
    ((struct SecrSifDownloadGetKbitParams *)RpcBuffer)->slot = slot;

    if (sceSifCallRpc(&SifRpcClient03, 1, 0, RpcBuffer, sizeof(RpcBuffer), RpcBuffer, sizeof(RpcBuffer), NULL, NULL) < 0) {
        _printf("sceSecrDownloadGetKbit: rpc error\n");
        result = 0;
    } else {
        memcpy(kbit, ((struct SecrSifDownloadGetKbitParams *)RpcBuffer)->kbit, sizeof(((struct SecrSifDownloadGetKbitParams *)RpcBuffer)->kbit));
        result = ((struct SecrSifDownloadGetKbitParams *)RpcBuffer)->result;
    }

    return result;
}

int SecrDownloadGetKc(int port, int slot, void *kc)
{
    int result;

    ((struct SecrSifDownloadGetKcParams *)RpcBuffer)->port = port;
    ((struct SecrSifDownloadGetKcParams *)RpcBuffer)->slot = slot;

    if (sceSifCallRpc(&SifRpcClient04, 1, 0, RpcBuffer, sizeof(RpcBuffer), RpcBuffer, sizeof(RpcBuffer), NULL, NULL) < 0) {
        _printf("sceSecrDownloadGetKc: rpc error\n");
        result = 0;
    } else {
        memcpy(kc, ((struct SecrSifDownloadGetKcParams *)RpcBuffer)->kc, sizeof(((struct SecrSifDownloadGetKcParams *)RpcBuffer)->kc));
        result = ((struct SecrSifDownloadGetKcParams *)RpcBuffer)->result;
    }

    return result;
}

int SecrDownloadGetICVPS2(void *icvps2)
{
    int result;

    if (sceSifCallRpc(&SifRpcClient05, 1, 0, RpcBuffer, sizeof(RpcBuffer), RpcBuffer, sizeof(RpcBuffer), NULL, NULL) < 0) {
        _printf("sceSecrDownloadGetICVPS2: rpc error\n");
        result = 0;
    } else {
        memcpy(icvps2, ((struct SecrSifDownloadGetIcvps2Params *)RpcBuffer)->icvps2, sizeof(((struct SecrSifDownloadGetIcvps2Params *)RpcBuffer)->icvps2));
        result = ((struct SecrSifDownloadGetIcvps2Params *)RpcBuffer)->result;
    }

    return result;
}

int SecrDiskBootHeader(void *buffer, SecrBitTable_t *BitTable, s32 *pSize)
{
    int result;

    memcpy(((struct SecrSifDiskBootHeaderParams *)RpcBuffer)->buffer, buffer, sizeof(((struct SecrSifDiskBootHeaderParams *)RpcBuffer)->buffer));

    if (sceSifCallRpc(&SifRpcClient06, 1, 0, RpcBuffer, sizeof(RpcBuffer), RpcBuffer, sizeof(RpcBuffer), NULL, NULL) < 0) {
        _printf("sceSecrDiskBootHeader: rpc error\n");
        result = 0;
    } else {
        memcpy(BitTable, &((struct SecrSifDiskBootHeaderParams *)RpcBuffer)->BitTable, ((struct SecrSifDiskBootHeaderParams *)RpcBuffer)->size);
        // BUG: pSize doesn't seem to be filled in within the Sony original.
        if (pSize != NULL)
            *pSize = ((struct SecrSifDiskBootHeaderParams *)RpcBuffer)->size;
        result = ((struct SecrSifDiskBootHeaderParams *)RpcBuffer)->result;
    }

    return result;
}

int SecrDiskBootBlock(void *src, void *dst, unsigned int size)
{
    int result;

    memcpy(((struct SecrSifDiskBootBlockParams *)RpcBuffer)->source, src, size);
    ((struct SecrSifDiskBootBlockParams *)RpcBuffer)->size = size;

    if (sceSifCallRpc(&SifRpcClient07, 1, 0, RpcBuffer, sizeof(RpcBuffer), RpcBuffer, sizeof(RpcBuffer), NULL, NULL) < 0) {
        _printf("sceSecrDiskBootBlock: rpc error\n");
        result = 0;
    } else {
        result = ((struct SecrSifDiskBootBlockParams *)RpcBuffer)->result;
        memcpy(dst, ((struct SecrSifDiskBootBlockParams *)RpcBuffer)->destination, size);
    }

    return result;
}

static unsigned short int GetHeaderLength(const void *buffer)
{
    return ((const SecrKELFHeader_t *)buffer)->KELF_header_size;
}

static void store_kbit(void *buffer, const void *kbit)
{
    const SecrKELFHeader_t *header = buffer;
    int offset                     = 0x20, kbit_offset;

    if (header->BIT_count > 0)
        offset += header->BIT_count * sizeof(SecrBitBlockData_t);
    if (((header->flags) & 1) != 0)
        offset += ((unsigned char *)buffer)[offset] + 1;
    if (((header->flags) & 0xF000) == 0)
        offset += 8;

    kbit_offset = (unsigned int)buffer + offset;
    memcpy((void *)kbit_offset, kbit, 16);
    _printf("kbit_offset: %d\n", kbit_offset);
}

static void store_kc(void *buffer, const void *kc)
{
    const SecrKELFHeader_t *header = buffer;
    int offset                     = 0x20, kc_offset;

    if (header->BIT_count > 0)
        offset += header->BIT_count * sizeof(SecrBitBlockData_t);
    if (((header->flags) & 1) != 0)
        offset += ((unsigned char *)buffer)[offset] + 1;
    if (((header->flags) & 0xF000) == 0)
        offset += 8;

    kc_offset = (unsigned int)buffer + offset + 0x10; // Goes after Kbit.
    memcpy((void *)kc_offset, kc, 16);
    _printf("kc_offset: %d\n", kc_offset);
}

static int Uses_ICVPS2(const void *buffer)
{
    return (((const SecrKELFHeader_t *)buffer)->flags >> 1 & 1);
}

static void store_icvps2(void *buffer, const void *icvps2)
{
    unsigned int pICVPS2;

    pICVPS2 = (unsigned int)buffer + ((SecrKELFHeader_t *)buffer)->KELF_header_size - 8;
    memcpy((void *)pICVPS2, icvps2, 8);
    _printf("icvps2_offset %d\n", pICVPS2);
}

static unsigned int get_BitTableOffset(const void *buffer)
{
    const SecrKELFHeader_t *header = buffer;
    int offset                     = sizeof(SecrKELFHeader_t);

    if (header->BIT_count > 0)
        offset += header->BIT_count * sizeof(SecrBitBlockData_t); // They used a loop for this. D:
    if ((header->flags & 1) != 0)
        offset += ((const unsigned char *)buffer)[offset] + 1;
    if ((header->flags & 0xF000) == 0)
        offset += 8;
    return (offset + 0x20); // Goes after Kbit and Kc.
}

void *SecrDownloadFile(int port, int slot, void *buffer)
{
    SecrBitTable_t BitTableData;
    void *result;

    _printf("SecrDownloadFile start\n");
    if (SecrDownloadHeader(port, slot, buffer, &BitTableData, NULL) != 0) {
        unsigned char kbit[16], kcontent[16];

        if (BitTableData.header.block_count > 0) {
            unsigned int offset, i;

            offset = BitTableData.header.headersize;
            for (i = 0; i < BitTableData.header.block_count; i++) {
                if (BitTableData.blocks[i].flags & 2) {
                    if (!SecrDownloadBlock((void *)((unsigned int)buffer + offset), BitTableData.blocks[i].size)) {
                        _printf("SecrDownloadFile: failed\n");
                        return NULL;
                    }
                }
                offset += BitTableData.blocks[i].size;
            }
        }

        if (SecrDownloadGetKbit(port, slot, kbit) == 0) {
            _printf("SecrDownloadFile: Cannot get kbit\n");
            return NULL;
        }
        if (SecrDownloadGetKc(port, slot, kcontent) == 0) {
            _printf("SecrDownloadFile: Cannot get kc\n");
            return NULL;
        }

        store_kbit(buffer, kbit);
        store_kc(buffer, kcontent);

        if (Uses_ICVPS2(buffer) == 1) {
            unsigned char icvps2[8];

            if (SecrDownloadGetICVPS2(icvps2) == 0) {
                _printf("SecrDownloadFile: Cannot get icvps2\n");
                return NULL;
            }

            store_icvps2(buffer, icvps2);
        }

        result = buffer;
    } else {
        _printf("SecrDownloadFile: Cannot encrypt header\n");
        return NULL;
    }

    _printf("SecrDownloadFile complete\n");

    return result;
}

void *SecrDiskBootFile(void *buffer)
{
    void *result;
    SecrBitTable_t *BitTableData;

    BitTableData = (SecrBitTable_t *)((unsigned int)buffer + get_BitTableOffset(buffer));
    if (SecrDiskBootHeader(buffer, BitTableData, NULL)) {
        if (BitTableData->header.block_count > 0) {
            unsigned int offset, i;

            offset = BitTableData->header.headersize;
            for (i = 0; i < BitTableData->header.block_count; i++) {
                if (BitTableData->blocks[i].flags & 3) {
                    if (!SecrDiskBootBlock((void *)((unsigned int)buffer + offset), (void *)((unsigned int)buffer + offset), BitTableData->blocks[i].size)) {
                        _printf("SecrDiskBootFile: failed\n");
                        return NULL;
                    }
                }
                offset += BitTableData->blocks[i].size;
            }
        }

        result = (void *)((unsigned int)buffer + GetHeaderLength(buffer));
    } else {
        _printf("sceSecrDiskBootFile: Cannot decrypt header\n");
        result = NULL;
    }

    return result;
}
