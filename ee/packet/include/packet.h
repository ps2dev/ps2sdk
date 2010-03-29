#ifndef __PACKET_H__
#define __PACKET_H__

#include <tamtypes.h>

/* 
 * Maximum number allowed, but each channel has its own limitations.
 * 
 */
#define QWC_MAX 65535

/* 
 * Dmatags use qwc for the number of qwords for they handle.
 * 
 * The dma channel QWC register uses it as the total number of qwords that
 * are being sent at a time.
 * 
 */
typedef struct {
	u32 total;
	u16 qwc;
	u8 spr;
	u8 ucab;
	QWORD *data __attribute__((aligned(64)));
} __attribute__ ((packed)) PACKET;

#ifdef __cplusplus
extern "C" {
#endif

	// Allocate a new packet for use, size in quadwords.
	int packet_allocate(PACKET *packet, int num, int ucab, int spr);

	// Reset the packet quadword counter and zero out data.
	void packet_reset(PACKET *packet);

	// Free the space allocated by packet.
	void packet_free(PACKET *packet);

	// Returns the currently unused qword.
	QWORD *packet_get_qword(PACKET *packet);

	// Advances the qwc and returns the current qword count.
	QWORD *packet_increment_qwc(PACKET *packet, int num);

#ifdef __cplusplus
}
#endif

#endif /*__PACKET_H__*/
