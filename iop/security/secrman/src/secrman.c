#include <loadcore.h>
#include <ioman.h>
#include <modload.h>
#include <stdio.h>
#include <sysclib.h>
#include <irx.h>
#include <sio2man.h>

#include "secrman.h"

#include "main.h"
#include "MechaAuth.h"
#include "CardAuth.h"
#include "keyman.h"

// Based on the module from DVD Player Installer 2.14

#define MODNAME "secrman_special"
IRX_ID(MODNAME, 1, 4);

#ifdef DEX_SUPPORT
int IsDEX;
#endif

extern struct irx_export_table _exp_secrman;

#if 0
static void _printf(const char *format, ...);
#endif

struct ModloadBitTableDescriptor
{
    SecrBitTableHeader_t header;
    SecrBitBlockData_t *blocks;
    unsigned int padding; // Doesn't seem to be used.
};

static loadfile_elf_load_proc_callback_t loadfile_elf_load_proc; // 0x00003d20
static loadfile_check_valid_ee_elf_callback_t loadfile_check_valid_ee_elf; // 0x00003d24
static loadfile_elf_get_program_header_callback_t loadfile_elf_get_program_header; // 0x00003d28
static loadfile_elf_load_alloc_buffer_from_heap_callback_t loadfile_elf_load_alloc_buffer_from_heap; // 0x00003d2c
static loadfile_elf_load_dealloc_buffer_from_heap_callback_t loadfile_elf_load_dealloc_buffer_from_heap; // 0x00003d30

// Function prototypes.
static int func_0000077c(int port, int slot, void *buffer);
static int func_00000bfc(void *dest, unsigned int size);
static int func_00002eb0(struct ModloadBitTableDescriptor *ModloadBitTableDescriptor, int fd, int port, int slot, int device);
static int secrman_loadfile_read_callback_common(int fd, loadfile_allocate_handler_struct_t *allocate_info, const struct ModloadBitTableDescriptor *ModloadBitTableDescriptor, int (*DecryptBlockFunction)(void *src, void *dest, unsigned int size));
static int secrman_loadfile_read_callback_card(int fd, loadfile_allocate_handler_struct_t *allocate_info, void *read_callback_userdata);
static int secrman_load_kelf_common(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs, int port, int slot, int *result_out, int *result_module_out, int device, loadfile_read_chunk_callback_t decrypt);
static int secrman_load_kelf_from_card(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs, int port, int slot, int *result_out, int *result_module_out);
static int secrman_loadfile_read_callback_disk(int fd, loadfile_allocate_handler_struct_t *allocate_info, void *read_callback_userdata);
static int secrman_load_kelf_from_disk(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs, int *result_out, int *result_module_out);
static void SecrGetLoadfileCallbacks(SetLoadfileCallbacks_struct_t *unknown);
static void RegisterSecrCallbackHandlers(void);
static int card_encrypt(int port, int slot, void *buffer);
static unsigned short int GetHeaderLength(const void *buffer);
static int secr_set_header(int mode, int cnum, int arg2, void *buffer);
static int Read_BIT(SecrBitTable_t *BitTable);
static unsigned int get_BitTableOffset(const void *buffer);
static int func_00000b5c(const void *block, unsigned int size);
static int func_00000c94(void *kbit);
static int func_00000cd4(void *kc);
static int func_00000d14(void *icvps2);
static int Uses_ICVPS2(const void *buffer);

// 0x00000000
#if 0
static void _printf(const char *format, ...){

}
#endif

// 0x0000077c
static int func_0000077c(int port, int slot, void *buffer)
{
    int result;

    if (GetMcCommandHandler() != NULL) {
        _printf("card decrypt start\n");

        if (card_auth(port, slot, 0xF1, 0x40)) {
            _printf("card decrypt 0x40\n");

            if (card_auth_write(port, slot, buffer, 0xF1, 0x41)) {
                _printf("card decrypt 0x41\n");

                if (card_auth(port, slot, 0xF1, 0x42)) {
                    _printf("card decrypt 0x42\n");

                    if (card_auth_read(port, slot, buffer, 0xF1, 0x43)) {
                        _printf("card decrypt 0x43\n");
                        result = 1;
                    } else
                        result = 0;
                } else
                    result = 0;
            } else
                result = 0;
        } else
            result = 0;
    } else
        result = 0;

    return result;
}

// 0x00000d34 - export #08
int SecrCardBootHeader(int port, int slot, void *buffer, SecrBitTable_t *BitTable, s32 *pSize)
{
    int result;
    unsigned char Kbit[16], Kc[16];

    if (GetMcCommandHandler() != NULL) {
        get_Kbit(buffer, Kbit);
        get_Kc(buffer, Kc);

        if (func_0000077c(port, slot, Kbit)) {
            if (func_0000077c(port, slot, &Kbit[8])) {
                if (func_0000077c(port, slot, Kc)) {
                    if (func_0000077c(port, slot, &Kc[8])) {
                        int cnum;

                        store_kc(buffer, Kc);
                        store_kbit(buffer, Kbit);

                        if ((cnum = McDeviceIDToCNum(port, slot)) >= 0) {
                            if (secr_set_header(1, cnum, 0, buffer)) {
                                int BitTableSize;

                                if ((BitTableSize = Read_BIT(BitTable)) != 0) {
                                    if (pSize != NULL)
                                        *pSize = BitTableSize;
                                    result = 1;
                                } else {
                                    _printf("Cannot read BIT\n");
                                    result = 0;
                                }
                            } else {
                                _printf("Set Header failed\n");
                                result = 0;
                            }
                        } else
                            result = 0;
                    } else
                        result = 0;
                } else
                    result = 0;
            } else
                result = 0;
        } else
            result = 0;
    } else {
        _printf("mcCommand isn't assigned\n");
        result = 0;
    }

    return result;
}



// 0x00000bfc
static int func_00000bfc(void *dest, unsigned int size)
{
    int result;

    if (func_00001c98(size)) {
        /*
            There was something like this here that doesn't make sense (instead of just the simple loop below):
            if(size>0){
                if(size<0) size+=0xF

                while(size>0){
                    ...
                }
            }
        */
        while (size > 0) {
            if (size >= 16) { // if(size>>4!=0)
                result = func_00001b00(dest, 0x10);
                size -= 0x10;
                dest = (void *)((u8 *)dest + 0x10);
            } else {
                result = func_00001b00(dest, size);
                size   = 0;
            }

            if (result == 0) {
                return 0;
            }
        }

        result = 1;
    } else
        result = 0;

    return result;
}

// 0x00000e90 - export #09
int SecrCardBootBlock(void *src, void *dest, unsigned int size)
{
    int result;

    if (func_00000b5c(src, size)) {
        result = func_00000bfc(dest, size) == 0 ? 0 : 1;
    } else
        result = 0;

    return result;
}

// 0x000011b4 - export #10
void *SecrCardBootFile(int port, int slot, void *buffer)
{
    void *result;
    SecrBitTable_t *BitTableData;

    BitTableData = (SecrBitTable_t *)((unsigned int)buffer + get_BitTableOffset(buffer));
    if (SecrCardBootHeader(port, slot, buffer, BitTableData, NULL)) {
        if (BitTableData->header.block_count > 0) {
            unsigned int offset, i;

            offset = BitTableData->header.headersize;
            for (i = 0; i < BitTableData->header.block_count; i++) {
                if (BitTableData->blocks[i].flags & 3) {
                    if (!SecrCardBootBlock((void *)((unsigned int)buffer + offset), (void *)((unsigned int)buffer + offset), BitTableData->blocks[i].size)) {
                        _printf("SecrCardBootFile: failed\n");
                        return NULL;
                    }
                }
                offset += BitTableData->blocks[i].size;
            }
        }

        result = (void *)((unsigned int)buffer + GetHeaderLength(buffer));
    } else {
        _printf("SecrCardBootFile: Cannot decrypt header\n");
        result = NULL;
    }

    return result;
}

// 0x000010dc - export #11
int SecrDiskBootHeader(void *buffer, SecrBitTable_t *BitTable, s32 *pSize)
{
    int result;

    if (secr_set_header(0, 0, 0, buffer)) {
        int BitTableSize;

        if ((BitTableSize = Read_BIT(BitTable)) != 0) {
            if (pSize != NULL)
                *pSize = BitTableSize;
            result = 1;
        } else {
            _printf("Cannot read BIT\n");
            result = 0;
        }
    } else {
        _printf("Set Header failed\n");
        result = 0;
    }

    return result;
}

// 0x00001164 - export #12
int SecrDiskBootBlock(void *src, void *dest, unsigned int size)
{
    int result;

    if (func_00000b5c(src, size)) {
        result = func_00000bfc(dest, size) == 0 ? 0 : 1;
    } else
        result = 0;

    return result;
}

// 0x00001474 - export #13
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
        _printf("SecrDiskBootFile: Cannot decrypt header\n");
        result = NULL;
    }

    return result;
}

// 0x00002eb0
static int func_00002eb0(struct ModloadBitTableDescriptor *ModloadBitTableDescriptor, int fd, int port, int slot, int device)
{
    int result;
    SecrKELFHeader_t header;
    SecrBitTable_t *BitTableData;

    if (lseek(fd, 0, SEEK_SET) == 0) {
        if (read(fd, &header, sizeof(SecrKELFHeader_t)) == sizeof(SecrKELFHeader_t)) {
            unsigned short int HeaderLength;
            void *HeaderBuffer;

            HeaderLength = GetHeaderLength(&header);
            if ((HeaderBuffer = loadfile_elf_load_alloc_buffer_from_heap(HeaderLength)) != NULL) {
                if (lseek(fd, 0, SEEK_SET) == 0) {
                    if (read(fd, HeaderBuffer, HeaderLength) == HeaderLength) {
                        BitTableData = (SecrBitTable_t *)((unsigned int)HeaderBuffer + get_BitTableOffset(HeaderBuffer));

                        if (device == 0) {
                            result = SecrCardBootHeader(port, slot, HeaderBuffer, BitTableData, NULL);
                        } else if (device == 1) {
                            result = SecrDiskBootHeader(HeaderBuffer, BitTableData, NULL);
                        } else {
                            return -1;
                        }

                        if (result == 1) {
                            result = 0;
                            memcpy(&ModloadBitTableDescriptor->header, &BitTableData->header, sizeof(ModloadBitTableDescriptor->header));
                            ModloadBitTableDescriptor->blocks = BitTableData->blocks;
                        } else {
                            loadfile_elf_load_dealloc_buffer_from_heap(HeaderBuffer);
                            result = -201;
                        }
                    } else {
                        loadfile_elf_load_dealloc_buffer_from_heap(HeaderBuffer);
                        result = -204;
                    }
                } else {
                    loadfile_elf_load_dealloc_buffer_from_heap(HeaderBuffer);
                    result = -201;
                }
            } else {
                result = -400;
            }
        } else
            result = -204;
    } else
        result = -204;

    return result;
}

/* 0x0000306c	- Reads a block of encrypted data and decrypts it.
    This is one really irritating function. Since nobody has ever completed and released a disassembly of MODLOAD, there were a whole load of structures that I had to figure out by cross-referencing LOADFILE, MODLOAD and SECRMAN. */
static int secrman_loadfile_read_callback_common(int fd, loadfile_allocate_handler_struct_t *allocate_info, const struct ModloadBitTableDescriptor *ModloadBitTableDescriptor, int (*DecryptBlockFunction)(void *src, void *dest, unsigned int size))
{
    unsigned int i, PayloadLength, excess;
    loadfile_ee_elf_ringbuffer_content_t *entry;
    int result;

#if 0
	printf(	"secrman_loadfile_read_callback_common:\n"
		"\tModloadUnk2->ring_buffer_index:\t%d\n"
		"\tModloadUnk2->read_buffer_offset:\t%d\n"
		"\tModloadUnk2->read_buffer_length:\t%d\n", allocate_info->ring_buffer_index, allocate_info->read_buffer_offset, allocate_info->read_buffer_length);
#endif

    entry         = &allocate_info->ring_buffer_contents[allocate_info->ring_buffer_index];
    entry->buffer_offset = allocate_info->read_buffer_offset;
    for (i = 0, PayloadLength = 0; i < ModloadBitTableDescriptor->header.block_count; PayloadLength += ModloadBitTableDescriptor->blocks[i].size, i++) {
        if (allocate_info->read_buffer_offset < PayloadLength + ModloadBitTableDescriptor->blocks[i].size) {
            break;
        }
    }

    // 0x000030fc - Partially decrypting blocks is not possible.
    if (((excess = PayloadLength - entry->buffer_offset) != 0) && (ModloadBitTableDescriptor->blocks[i].flags & 3)) {
#if 0
		printf("WIP function 0x0000306c: can't partially decrypt block.\n");
#endif
        result = -201;
    } else {
        unsigned int ExcessBlockNum;
        void *BlockExcessRegionPtr;

        ExcessBlockNum       = i;
        BlockExcessRegionPtr = (void *)((unsigned int)entry->buffer_base + excess);

        for (; i < ModloadBitTableDescriptor->header.block_count; excess += ModloadBitTableDescriptor->blocks[i].size, i++) {
            // 0x00003140
            if (ModloadBitTableDescriptor->blocks[i].size + excess >= allocate_info->read_buffer_length) {
                if (!(ModloadBitTableDescriptor->blocks[i].flags & 3)) {
                    excess = allocate_info->read_buffer_length;
                }

                break;
            }
        }

        if (((unsigned int)read(fd, entry->buffer_base, excess)) == excess) {
            allocate_info->read_buffer_offset += excess;

            for (; ExcessBlockNum < i; BlockExcessRegionPtr = (void *)((u8 *)BlockExcessRegionPtr + ModloadBitTableDescriptor->blocks[ExcessBlockNum].size), ExcessBlockNum++) {
                if (ModloadBitTableDescriptor->blocks[ExcessBlockNum].flags & 3) {
                    // 0x000031d8
                    if (DecryptBlockFunction(BlockExcessRegionPtr, BlockExcessRegionPtr, ModloadBitTableDescriptor->blocks[ExcessBlockNum].size) != 1) {
                        return -201;
                    }
                }
            }

            result = 0;
        } else {
            result = -204;
        }
    }

    return result;
}

// 0x00003364
static int secrman_loadfile_read_callback_card(int fd, loadfile_allocate_handler_struct_t *allocate_info, void *read_callback_userdata)
{
    return secrman_loadfile_read_callback_common(fd, allocate_info, (const struct ModloadBitTableDescriptor *)read_callback_userdata, &SecrCardBootBlock);
}

// 0x00003230
static int secrman_load_kelf_common(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs, int port, int slot, int *result_out, int *result_module_out, int device, loadfile_read_chunk_callback_t decrypt)
{
    int result;
    struct ModloadBitTableDescriptor ModloadBitTableDescriptor;

    if (func_00002eb0(&ModloadBitTableDescriptor, flhs->fd, port, slot, device) == 0) {
        if (((u32)lseek(flhs->fd, ModloadBitTableDescriptor.header.headersize, SEEK_SET)) == ModloadBitTableDescriptor.header.headersize) {
            if (decrypt(flhs->fd, allocate_info, &ModloadBitTableDescriptor) == 0) {
                if (loadfile_check_valid_ee_elf(allocate_info, flhs) == 0) {
                    if (loadfile_elf_get_program_header(allocate_info, flhs) == 0) {
                        if (loadfile_elf_load_proc(allocate_info, flhs, &ModloadBitTableDescriptor, decrypt) == 0) {
                            result = 0;

                            // 0x00003318
                            *result_out = flhs->elf_header.e_entry;
                            *result_module_out = 0;
                        } else {
                            printf("load elf error\n");
                            result = -201;
                        }
                    } else {
                        printf("don't get program header\n");
                        result = -201;
                    }
                } else {
                    printf("don't get elf header\n");
                    result = -201;
                }
            } else
                result = -204;
        } else
            result = -204;
    } else
        result = -201;

    return result;
}

// 0x0000338c
static int secrman_load_kelf_from_card(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs, int port, int slot, int *result_out, int *result_module_out)
{
    return secrman_load_kelf_common(allocate_info, flhs, port, slot, result_out, result_module_out, 0, &secrman_loadfile_read_callback_card);
}

// 0x000033c8
static int secrman_loadfile_read_callback_disk(int fd, loadfile_allocate_handler_struct_t *allocate_info, void *read_callback_userdata)
{
    return secrman_loadfile_read_callback_common(fd, allocate_info, (const struct ModloadBitTableDescriptor *)read_callback_userdata, &SecrDiskBootBlock);
}

// 0x000033f0
static int secrman_load_kelf_from_disk(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs, int *result_out, int *result_module_out)
{
    return secrman_load_kelf_common(allocate_info, flhs, 0, 0, result_out, result_module_out, 1, &secrman_loadfile_read_callback_disk);
}

// 0x00003430
static void SecrGetLoadfileCallbacks(SetLoadfileCallbacks_struct_t *callbackinfo)
{
    loadfile_elf_load_proc = callbackinfo->elf_load_proc;
    loadfile_check_valid_ee_elf = callbackinfo->check_valid_ee_elf;
    loadfile_elf_get_program_header = callbackinfo->elf_get_program_header;
    loadfile_elf_load_alloc_buffer_from_heap = callbackinfo->elf_load_alloc_buffer_from_heap;
    loadfile_elf_load_dealloc_buffer_from_heap = callbackinfo->elf_load_dealloc_buffer_from_heap;

    callbackinfo->load_kelf_from_card = &secrman_load_kelf_from_card;
    callbackinfo->load_kelf_from_disk = &secrman_load_kelf_from_disk;
}

// 0x0000348c
static void RegisterSecrCallbackHandlers(void)
{
    SetSecrmanCallbacks(&SecrCardBootFile, &SecrDiskBootFile, &SecrGetLoadfileCallbacks);
}

// 0x00000018
int _start(int argc, char *argv[])
{
    int result;
#ifdef DEX_SUPPORT
    int fd;
    char type;
#endif

    (void)argc;
    (void)argv;

#ifdef DEX_SUPPORT
    fd = open("rom0:ROMVER", O_RDONLY);
    lseek(fd, 5, SEEK_SET);
    read(fd, &type, 1);
    close(fd);

    IsDEX = type == 'D';
#endif

    if (RegisterLibraryEntries(&_exp_secrman) == 0) {
        ResetMcHandlers();
        RegisterSecrCallbackHandlers();
        result = MODULE_RESIDENT_END;
    } else
        result = MODULE_NO_RESIDENT_END;

    return result;
}

// 0x0000005c
int _exit(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    return MODULE_NO_RESIDENT_END;
}

// 0x00000064 - export #04
void SecrSetMcCommandHandler(McCommandHandler_t handler)
{
    SetMcCommandHandler(handler);
}

// 0x00000084 - export #05
void SecrSetMcDevIDHandler(McDevIDHandler_t handler)
{
    SetMcDevIDHandler(handler);
}

// 0x000000a4 - export #06
// This is the most badass function in the whole file. Not only was it difficult to see whether I got all the pointers right, it invokes many functions, making it a difficult one to debug. I tried to make the buffer names the same as within the FMCB PS3MCA bundle's Card_Authentificate() function.
int SecrAuthCard(int port, int slot, int cnum)
{
    int result;
    unsigned char MechaChallenge2[8], CardIV[8], CardMaterial[8], CardNonce[8], MechaChallenge1[8], MechaChallenge3[8], CardResponse1[8], CardResponse2[8], CardResponse3[8];

    if (GetMcCommandHandler() != NULL) {
        _printf("SecrAuthCard start\n");

        memset(MechaChallenge1, 0, sizeof(MechaChallenge1));

        if (card_auth_60(port, slot)) {
            _printf("card auth 0x60\n");

            if (mechacon_auth_80(cnum)) {
                _printf("mechacon auth 0x80\n");

                /*	The normal secrman_for_dex module does not have this step. With this step, card authentication does not work on a DEX.
                    On a CEX, it appears that card authentication still works without this step, but I don't know whether there are any side-effects.	*/
                if (
#ifdef DEX_SUPPORT
                    IsDEX ||
#endif
                    card_auth_key_change(port, slot, 1)) {
                    _printf("card auth key change\n");

                    if (card_auth(port, slot, 0xF0, 0x00)) {
                        _printf("card auth 0x00\n");

                        if (mechacon_auth_81(cnum)) {
                            _printf("mechacon auth 0x81\n");

                            if (card_auth_read(port, slot, CardIV, 0xF0, 0x01)) {
                                _printf("card auth 0x01\n");

                                if (card_auth_read(port, slot, CardMaterial, 0xF0, 0x02)) {
                                    _printf("card auth 0x02\n");

                                    if (mechacon_auth_82(CardIV, CardMaterial)) {
                                        _printf("mechacon auth 0x82\n");

                                        if (card_auth(port, slot, 0xF0, 0x03)) {
                                            _printf("card auth 0x03\n");

                                            if (card_auth_read(port, slot, CardNonce, 0xF0, 0x04)) {
                                                _printf("card auth 0x04\n");

                                                if (mechacon_auth_83(CardNonce)) {
                                                    _printf("mechacon auth 0x83\n");

                                                    if (card_auth(port, slot, 0xF0, 0x05)) {
                                                        _printf("card auth 0x05\n");

                                                        if (pol_cal_cmplt()) {
                                                            _printf("mechacon auth 0x8f\n");

                                                            if (mechacon_auth_84(MechaChallenge1, MechaChallenge2)) {
                                                                _printf("mechacon auth 0x84\n");

                                                                if (mechacon_auth_85(&MechaChallenge2[4], MechaChallenge3)) {
                                                                    _printf("mechacon auth 0x85\n");

                                                                    if (card_auth_write(port, slot, MechaChallenge3, 0xF0, 0x06)) {
                                                                        _printf("card auth 0x06\n");

                                                                        if (card_auth_write(port, slot, MechaChallenge2, 0xF0, 0x07)) {
                                                                            _printf("card auth 0x07\n");

                                                                            if (card_auth(port, slot, 0xF0, 0x08)) {
                                                                                _printf("card auth 0x08\n");

                                                                                if (card_auth2(port, slot, 0xF0, 0x09)) {
                                                                                    _printf("card auth 0x09\n");

                                                                                    if (card_auth(port, slot, 0xF0, 0x0A)) {
                                                                                        _printf("card auth 0x0a\n");

                                                                                        if (card_auth_write(port, slot, MechaChallenge1, 0xF0, 0x0B)) {
                                                                                            _printf("card auth 0x0b\n");

                                                                                            if (card_auth(port, slot, 0xF0, 0x0C)) {
                                                                                                _printf("card auth 0x0c\n");

                                                                                                if (card_auth2(port, slot, 0xF0, 0x0D)) {
                                                                                                    _printf("card auth 0x0d\n");

                                                                                                    if (card_auth(port, slot, 0xF0, 0x0E)) {
                                                                                                        _printf("card auth 0x0e\n");

                                                                                                        if (card_auth_read(port, slot, CardResponse1, 0xF0, 0x0F)) { // Originally, it used the same region as CardNonce. But that might have just been a result of compiler code optimization.
                                                                                                            _printf("card auth 0x0f\n");

                                                                                                            if (card_auth(port, slot, 0xF0, 0x10)) {
                                                                                                                _printf("card auth 0x10\n");

                                                                                                                if (card_auth_read(port, slot, CardResponse2, 0xF0, 0x11)) {
                                                                                                                    _printf("card auth 0x11\n");

                                                                                                                    if (mechacon_auth_86(CardResponse1, CardResponse2)) {
                                                                                                                        _printf("mechacon auth 0x86\n");

                                                                                                                        if (card_auth(port, slot, 0xF0, 0x12)) {
                                                                                                                            _printf("card auth 0x12\n");

                                                                                                                            if (card_auth_read(port, slot, CardResponse3, 0xF0, 0x13)) {
                                                                                                                                _printf("card auth 0x13\n");

                                                                                                                                if (mechacon_auth_87(CardResponse3)) {
                                                                                                                                    _printf("mechacon auth 0x87\n");

                                                                                                                                    if (pol_cal_cmplt()) {
                                                                                                                                        _printf("mechacon auth 0x8f\n");

                                                                                                                                        if (card_auth(port, slot, 0xF0, 0x14)) {
                                                                                                                                            _printf("card auth 0x14\n");

                                                                                                                                            if (mechacon_auth_88()) {
                                                                                                                                                _printf("mechacon auth 0x88\n");
                                                                                                                                                result = 1;
                                                                                                                                            } else {
                                                                                                                                                goto Error2_end;
                                                                                                                                            }
                                                                                                                                        } else {
                                                                                                                                            goto Error2_end;
                                                                                                                                        }
                                                                                                                                    } else {
                                                                                                                                        goto Error2_end;
                                                                                                                                    }
                                                                                                                                } else {
                                                                                                                                    goto Error2_end;
                                                                                                                                }
                                                                                                                            } else {
                                                                                                                                goto Error2_end;
                                                                                                                            }
                                                                                                                        } else {
                                                                                                                            goto Error2_end;
                                                                                                                        }
                                                                                                                    } else {
                                                                                                                        goto Error2_end;
                                                                                                                    }
                                                                                                                } else {
                                                                                                                    goto Error2_end;
                                                                                                                }
                                                                                                            } else {
                                                                                                                goto Error2_end;
                                                                                                            }
                                                                                                        } else {
                                                                                                            goto Error2_end;
                                                                                                        }

                                                                                                    } else {
                                                                                                        goto Error2_end;
                                                                                                    }
                                                                                                } else {
                                                                                                    goto Error2_end;
                                                                                                }
                                                                                            } else {
                                                                                                goto Error2_end;
                                                                                            }
                                                                                        } else {
                                                                                            goto Error2_end;
                                                                                        }
                                                                                    } else {
                                                                                        goto Error2_end;
                                                                                    }
                                                                                } else {
                                                                                    goto Error2_end;
                                                                                }
                                                                            } else {
                                                                                goto Error2_end;
                                                                            }
                                                                        } else {
                                                                            goto Error2_end;
                                                                        }
                                                                    } else {
                                                                        goto Error2_end;
                                                                    }
                                                                } else {
                                                                    goto Error2_end;
                                                                }
                                                            } else {
                                                                goto Error2_end;
                                                            }
                                                        } else {
                                                            goto Error2_end;
                                                        }
                                                    } else {
                                                        goto Error2_end;
                                                    }
                                                } else {
                                                    goto Error2_end;
                                                }
                                            } else {
                                                goto Error2_end;
                                            }
                                        } else {
                                            goto Error2_end;
                                        }
                                    } else {
                                        goto Error2_end;
                                    }
                                } else {
                                    goto Error2_end;
                                }
                            } else {
                                goto Error2_end;
                            }
                        } else {
                            card_auth_60(port, slot);
                            result = 0;
                        }
                    } else {
                        card_auth_60(port, slot);
                        result = 0;
                    }
                } else {
                    card_auth_60(port, slot);
                    result = 0;
                }
            } else
                result = 0;
        } else
            result = 0;
    } else {
        _printf("mcCommand isn't assigned\n");
        result = 0;
    }

    return result;

Error2_end:
    card_auth_60(port, slot);
    mechacon_auth_80(cnum);

    return 0;
}

// 0x00000750 - export #07
void SecrResetAuthCard(int port, int slot, int cnum)
{
    card_auth_60(port, slot);
    mechacon_auth_80(cnum);
}

// 0x00002a98
static unsigned short int GetHeaderLength(const void *buffer)
{
    return ((const SecrKELFHeader_t *)buffer)->KELF_header_size;
}

// 0x000009ac
static int secr_set_header(int mode, int cnum, int arg2, void *buffer)
{
    unsigned short int HeaderLength;
    int result;

    HeaderLength = GetHeaderLength(buffer);
    _printf("header length %d\n", HeaderLength);
    if (write_HD_start(mode, cnum, arg2, HeaderLength) != 0) {
        while (HeaderLength > 0) {
            if (HeaderLength >= 0x10) { // if(HeaderLength>>4!=0)
                result = write_data(buffer, 0x10);
                buffer = (void *)((u8 *)buffer + 0x10);
                HeaderLength -= 0x10;
            } else {
                result       = write_data(buffer, HeaderLength);
                HeaderLength = 0;
            }

            if (result == 0) {
                _printf("secr_set_header: fail write_data\n");
                goto err_end;
            }
        }

        if (pol_cal_cmplt() == 0) {
            _printf("secr_set_header: fail pol_cal_cmplt\n");
            goto err_end;
        }
    } else {
        _printf("secr_set_header: fail write_HD_start\n");
        goto err_end;
    }

    return 1;

err_end:
    return 0;
}

// 0x00000acc
static int Read_BIT(SecrBitTable_t *BitTable)
{
    unsigned short int BitLength;

    if (get_BIT_length(&BitLength) != 0) {
        unsigned short int DataToCopy;

        DataToCopy = BitLength;

        while (DataToCopy != 0) {
            int result;

            if (DataToCopy >= 0x10) {
                result   = func_00001b00(BitTable, 0x10);
                BitTable = (SecrBitTable_t *)((u8 *)BitTable + 0x10);
                DataToCopy -= 0x10;
            } else {
                result     = func_00001b00(BitTable, DataToCopy);
                DataToCopy = 0;
            }

            if (result == 0) {
                return 0;
            }
        }
    } else {
        return 0;
    }

    return BitLength;
}

// 0x00000ee0 - export #14
int SecrDownloadHeader(int port, int slot, void *buffer, SecrBitTable_t *BitTable, s32 *pSize)
{
    int cnum, result;

    if (GetMcCommandHandler() == NULL) {
        _printf("mcCommand isn't assigned\n");
        return 0;
    }

    if ((cnum = McDeviceIDToCNum(port, slot)) >= 0) {
        if (secr_set_header(2, cnum, 0, buffer) != 0) {
            int size;

            if ((size = Read_BIT(BitTable)) != 0) {
                if (pSize != NULL)
                    *pSize = size;
                result = 1;
            } else
                result = 0;
        } else
            result = 0;
    } else {
        _printf("invalid cnum '%d'\n", cnum);
        result = 0;
    }

    return result;
}

// 0x00002aa4
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

// 0x00000b5c
static int func_00000b5c(const void *block, unsigned int size)
{
    int result;

    if (mechacon_set_block_size(size)) {
        while (size > 0) {
            result = write_data(block, 0x10);
            block  = (const void *)((const u8 *)block + 0x10);
            size -= 0x10;

            if (result == 0) {
                goto err_end;
            }
        }

        result = (pol_cal_cmplt() == 0) ? 0 : 1;
    } else
        result = 0;

    return result;
err_end:
    return 0;
}

// 0x00000fb4 - export #15
int SecrDownloadBlock(void *block, unsigned int size)
{
    return (func_00000b5c(block, size) == 1 ? 1 : 0);
}

// 0x00000c94
static int func_00000c94(void *kbit)
{
    int result;

    if ((result = func_00001ce8(kbit)) != 0) {
        result = func_00001d64((void *)((unsigned int)kbit + 8)) == 1 ? 1 : 0;
    }

    return result;
}


// 0x00000cd4
static int func_00000cd4(void *kc)
{
    int result;

    if ((result = func_00001de0(kc)) != 0) {
        result = func_00001e5c((void *)((unsigned int)kc + 8)) == 1 ? 1 : 0;
    }

    return result;
}

// 0x00000894
static int card_encrypt(int port, int slot, void *buffer)
{
    int result;

    if (GetMcCommandHandler() != NULL) {
        _printf("card encrypt start\n");

        if (card_auth(port, slot, 0xF2, 0x50)) {
            _printf("card encrypt 0x50\n");
            if (card_auth_write(port, slot, buffer, 0xF2, 0x51)) {
                _printf("card encrypt 0x51\n");
                if (card_auth(port, slot, 0xF2, 0x52)) {
                    _printf("card encrypt 0x52\n");
                    if (card_auth_read(port, slot, buffer, 0xF2, 0x53)) {
                        _printf("card encrypt 0x53\n");
                        result = 1;
                    } else
                        result = 0;
                } else
                    result = 0;
            } else
                result = 0;
        } else
            result = 0;
    } else
        result = 0;

    return result;
}

// 0x00000fd4 - export #17
int SecrDownloadGetKbit(int port, int slot, void *kbit)
{
    return ((func_00000c94(kbit) && card_encrypt(port, slot, kbit) && card_encrypt(port, slot, (void *)((u8 *)kbit + 8))) ? 1 : 0);
}

// 0x00001048 - export #18
int SecrDownloadGetKc(int port, int slot, void *kc)
{
    return ((func_00000cd4(kc) && card_encrypt(port, slot, kc) && card_encrypt(port, slot, (void *)((u8 *)kc + 8))) ? 1 : 0);
}

// 0x00000d14
static int func_00000d14(void *icvps2)
{
    return (func_00001ed8(icvps2) == 0 ? 0 : 1);
}

// 0x000010bc - export #19
int SecrDownloadGetICVPS2(void *icvps2)
{
    return (func_00000d14(icvps2) == 0 ? 0 : 1);
}

// 0x00002b0c
static int Uses_ICVPS2(const void *buffer)
{
    return (((const SecrKELFHeader_t *)buffer)->flags >> 1 & 1);
}

// 0x000012c0 - export #16
void *SecrDownloadFile(int port, int slot, void *buffer)
{
    SecrBitTable_t BitTableData;
    void *result;

    _printf("SecrDownloadFile start\n");
    get_BitTableOffset(buffer); // Doesn't use its return value?
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
