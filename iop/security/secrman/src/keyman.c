#include <stdio.h>
#include <sysclib.h>

#include "secrman.h"

#include "main.h"
#include "keyman.h"

#if 0
static void _printf2(const char *format, ...);
#endif

// 0x00002a80
#if 0
static void _printf2(const char *format, ...){

}
#endif

// 0x00002bd0
void store_kbit(void *buffer, const void *kbit)
{
    const SecrKELFHeader_t *header = buffer;
    int offset                     = 0x20;
    u8 *kbit_offset;

    if (header->BIT_count > 0)
        offset += header->BIT_count * 0x10; // They used a loop for this. D:
    if (((header->flags) & 1) != 0)
        offset += ((unsigned char *)buffer)[offset] + 1;
    if (((header->flags) & 0xF000) == 0)
        offset += 8;

    kbit_offset = (u8 *)buffer + offset;
    memcpy((void *)kbit_offset, kbit, 16);
    _printf2("kbit_offset: %p\n", kbit_offset);
}

// 0x00002d38
void store_kc(void *buffer, const void *kc)
{
    const SecrKELFHeader_t *header = buffer;
    int offset                     = 0x20;
    u8 *kc_offset;

    if (header->BIT_count > 0)
        offset += header->BIT_count * 0x10; // They used a loop for this. D:
    if (((header->flags) & 1) != 0)
        offset += ((unsigned char *)buffer)[offset] + 1;
    if (((header->flags) & 0xF000) == 0)
        offset += 8;

    kc_offset = (u8 *)buffer + offset + 0x10;
    memcpy((void *)kc_offset, kc, 16);
    _printf2("kc_offset: %p\n", kc_offset);
}

// 0x00002b20
void get_Kbit(const void *buffer, void *Kbit)
{
    const SecrKELFHeader_t *header = buffer;
    int offset                     = sizeof(SecrKELFHeader_t);
    u8 *kbit_offset;

    if (header->BIT_count > 0)
        offset += header->BIT_count * sizeof(SecrBitBlockData_t); // They used a loop for this. D:
    if (((header->flags) & 1) != 0)
        offset += ((unsigned char *)buffer)[offset] + 1;
    if (((header->flags) & 0xF000) == 0)
        offset += 8;

    kbit_offset = (u8 *)buffer + offset;
    memcpy(Kbit, (void *)kbit_offset, 16);
    _printf2("kbit_offset: %p\n", kbit_offset);
}

// 0x00002c80
void get_Kc(const void *buffer, void *Kc)
{
    const SecrKELFHeader_t *header = buffer;
    int offset                     = sizeof(SecrKELFHeader_t);
    u8 *kc_offset;

    if (header->BIT_count > 0)
        offset += header->BIT_count * sizeof(SecrBitBlockData_t); // They used a loop for this. D:
    if (((header->flags) & 1) != 0)
        offset += ((unsigned char *)buffer)[offset] + 1;
    if (((header->flags) & 0xF000) == 0)
        offset += 8;

    kc_offset = (u8 *)buffer + offset + 0x10; // Goes after Kbit
    memcpy(Kc, (void *)kc_offset, 16);
    _printf2("kc_offset: %p\n", kc_offset);
}

// 0x00002df0 - unused.
void get_icvps2(const void *buffer, void *icvps2)
{
    u8 *pICVPS2;

    pICVPS2 = (u8 *)buffer + ((SecrKELFHeader_t *)buffer)->KELF_header_size - 8;
    memcpy(icvps2, (void *)pICVPS2, 8);
    _printf2("icvps2_offset %p\n", pICVPS2);
}

// 0x00002e4c
void store_icvps2(void *buffer, const void *icvps2)
{
    u8 *pICVPS2;

    pICVPS2 = (u8 *)buffer + ((SecrKELFHeader_t *)buffer)->KELF_header_size - 8;
    memcpy((void *)pICVPS2, icvps2, 8);
    _printf2("icvps2_offset %p\n", pICVPS2);
}
