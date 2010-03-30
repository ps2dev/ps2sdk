#ifndef __VIF_CODES_H__
#define __VIF_CODES_H__

#include <tamtypes.h>

#define VIF_CMD_NOP			0x00	// No Operation
#define VIF_CMD_STCYCL		0x01	// Sets CYCLE register
#define VIF_CMD_OFFSET		0x02	// Sets OFFSET register (VIF1)
#define VIF_CMD_BASE		0x03	// Sets BASE register (VIF1)
#define VIF_CMD_ITOP		0x04	// Sets ITOPS register
#define VIF_CMD_STMOD		0x05	// Sets MODE register
#define VIF_CMD_MSKPATH3	0x06	// Mask GIF transfer (VIF1)
#define VIF_CMD_MARK		0x07	// Sets Mark register
#define VIF_CMD_FLUSHE		0x10	// Wait for end of microprogram
#define VIF_CMD_FLUSH		0x11	// Wait for end of microprogram & Path 1/2 GIF xfer (VIF1)
#define VIF_CMD_FLUSHA		0x13	// Wait for end of microprogram & all Path GIF xfer (VIF1)
#define VIF_CMD_MSCAL		0x14	// Activate microprogram
#define VIF_CMD_MSCNT		0x17	// Execute microrprogram continuously
#define VIF_CMD_MSCALF		0x15	// Activate microprogram (VIF1)
#define VIF_CMD_STMASK		0x20	// Sets MASK register
#define VIF_CMD_STROW		0x30	// Sets ROW register
#define VIF_CMD_STCOL		0x31	// Sets COL register
#define VIF_CMD_MPG			0x4A	// Load microprogram
#define VIF_CMD_DIRECT		0x50	// Transfer data to GIF (VIF1)
#define VIF_CMD_DIRECTHL	0x51	// Transfer data to GIF but stall for Path 3 IMAGE mode (VIF1)

#define PACK_VIFTAG(Q,W0,W1,W2,W3) \
	Q->sw[0] = (u32)(W0), \
	Q->sw[1] = (u32)(W1), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

// Transfers data to the VU Mem
#define VIF_CMD_UNPACK(M,VN,VL) \
	(u32)((VL) & 0x00000003) << 0 | (u32)((VN) & 0x00000003) << 2 | \
	(u32)((M)  & 0x00000001) << 4 | (u32)((3)  & 0x00000003) << 5

#define STCYCL_IMDT(CL,WL) \
	(u32)((CL) & 0x000000FF) << 0 | (u32)((WL) & 0x000000FF) << 8

#define OFFSET_IMDT(OFFSET) \
	(u32)((OFFSET) & 0x000003FF)

#define BASE_IMDT(BASE) \
	(u32)((BASE) & 0x000003FF)

#define ITOP_IMDT(ADDR) \
	(u32)((ADDR) & 0x000003FF)

#define STMOD_IMDT(MODE) \
	(u32)((MODE) & 0x00000003)

#define MSKPATH3_IMDT(MASK) \
	(u32)((MASK) & 0x00000001) << 15

#define MARK_IMDT(MARK) \
	(u32)((MARK) & 0x0000FFFF)

#define MSCAL_IMDT(EXECADDR) \
	(u32)((EXECADDR) & 0x0000FFFF)

#define MSCALF_IMDT(EXECADDR) \
	(u32)((EXECADDR) & 0x0000FFFF)

#define MPG_IMDT(LOADADDR) \
	(u32)((LOADADDR) & 0x0000FFFF)

#define MPG_NUM(SIZE) \
	(u32)((SIZE) & 0x000000FF) << 16

#define DIRECT_IMDT(SIZE) \
	(u32)((SIZE) & 0x0000FFFF)

#define DIRECTHL_IMDT(EXECADDR) \
	(u32)((EXECADDR) & 0x0000FFFF)

#define UNPACK_IMDT(ADDR,USN,FLG) \
	(u32)((ADDR) & 0x000003FF) << 0  | (u32)((USN) & 0x00000001) << 14 | \
	(u32)((FLG)  & 0x00000001) << 15

#define UNPACK_NUM(SIZE) \
	(u32)((SIZE) & 0x000000FF) << 16

#define VIF_CODE(IMDT,NUM,CMD,IRQ) \
	(u32)((IMDT) & 0x0000FFFF) << 0  | (u32)((NUM)  & 0x000000FF) << 16 | \
	(u32)((CMD)  & 0x000000FF) << 24 | (u32)((IRQ)  & 0x00000001) << 31

#endif /*__VIF_CODES_H__*/
