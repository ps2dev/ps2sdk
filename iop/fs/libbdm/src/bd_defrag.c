#include <bd_defrag.h>

//#define DEBUG  //comment out this line when not debugging
#include "module_debug.h"


int bd_defrag(struct block_device* bd, u32 fragcount, struct bd_fragment* fraglist, u64 sector, void* buffer, u16 count)
{
    u64 sector_start = sector;
    u16 count_left = count;

    while (count_left > 0) {
        u16 count_read;
        u64 offset = 0; // offset of fragment in bd/file
        struct bd_fragment *f = NULL;
        int i;

        // Locate fragment containing start sector
        for (i=0; (u32)i<fragcount; i++) {
            f = &fraglist[i];
            if (offset <= sector_start && (offset + f->count) > sector_start) {
                // Fragment found
                break;
            }
            offset += f->count;
        }

        if ((u32)i == fragcount) {
            M_PRINTF("%s: ERROR: fragment not found!\n", __FUNCTION__);
            return -1;
        }

        // Clip to fragment size
        count_read = count_left;
        if ((sector_start + count_read) > (offset + f->count)) {
            count_read = (offset + f->count) - sector_start;
            M_DEBUG("%s: clipping sectors %d -> %d\n", __FUNCTION__, count_left, count_read);
        }

        // Do the read
        if (bd->read(bd, f->sector + (sector_start - offset), buffer, count_read) != count_read) {
            M_PRINTF("%s: ERROR: read failed!\n", __FUNCTION__);
            return -1;
        }

        // Advance to next fragment
        sector_start += count_read;
        count_left -= count_read;
        buffer = (u8*)buffer + (count_read * bd->sectorSize);
    }

    return count;
}
