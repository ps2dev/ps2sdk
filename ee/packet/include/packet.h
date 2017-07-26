/**
 * @file
 * Packet library functions.
 */

#ifndef __PACKET_H__
#define __PACKET_H__

#include <tamtypes.h>

/** Maximum number allowed, but each channel has its own limitations. */
#define QWC_MAX 65535

#define PACKET_NORMAL 0x00
#define PACKET_UCAB   0x01
#define PACKET_SPR    0x02
/**
 * Dmatags use qwc for the number of qwords for they handle.
 *
 * The dma channel QWC register uses it as the total number of qwords that
 * are being sent at a time.
 */
typedef struct {
	u32 qwords;
	u16 qwc;
	u16 type;
	qword_t *data __attribute__((aligned(64)));
} packet_t;

#ifdef __cplusplus
extern "C" {
#endif

/** Allocate a new packet for use, size in quadwords. */
packet_t *packet_init(int qwords, int type);

/** Reset the packet quadword counter and zero out data. */
void packet_reset(packet_t *packet);

/** Free the space allocated by packet. */
void packet_free(packet_t *packet);

/** Advances the qwc and returns the current qword count. */
qword_t *packet_increment_qwc(packet_t *packet, int num);

/** For those that like getters and setters */
static inline qword_t *packet_get_qword(packet_t *packet)
{
	return packet->data;
}

#ifdef __cplusplus
}
#endif

#endif /* __PACKET_H__ */
