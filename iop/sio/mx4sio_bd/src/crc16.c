#include "crc16.h"


uint16_t crc16(void *buf, int len)
{
    uint16_t *bf  = buf;
    uint32_t poly = 0; // Initializing poly to 0 AND using it as initializer in the loop for polyIn results in the compiler forcing it always to 0 in the loop (so don't use variables as initializers - i.e. u32 polyIn = poly; ).
    int i;
    len /= 2; // to u16
    for (i = 0; i < len; i++) {
        uint32_t polyIn, di;

        di     = (((bf[i]) << 8) & 0xFF00) | (((bf[i]) >> 8) & 0xFF);
        polyIn = poly;
        // PPC has an instruction for shifting and masking (continuous mask) at the same time.
        // This sequence must be folowed because some of the bits altered in the first operations are used in the folowing.
        polyIn ^= ((polyIn ^ di) >> 4) & 0x0F00;
        polyIn ^= ((polyIn ^ di) >> 4) & 0x00F0;
        polyIn ^= ((polyIn ^ di) >> 11) & 0x001F;
        polyIn ^= ((polyIn ^ di) >> 4) & 0x000F;
        polyIn ^= di;
        poly = polyIn ^ (polyIn << 5) ^ (polyIn << 12);
    }

    uint16_t crc = poly;
    return crc;
}
