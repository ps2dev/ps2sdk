#ifndef _SIFDMA_H
#define _SIFDMA_H

#include <tamtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SIF_DMA_INT_I	0x2
#define SIF_DMA_INT_O	0x4

typedef struct t_SifDmaTransfer
{
   void				*src,
      				*dest;
   int				size;
   int				attr;
} SifDmaTransfer_t;

u32 SifSetDma(SifDmaTransfer_t *sdd, s32 len);
s32 SifDmaStat(u32 id);

#ifdef __cplusplus
}
#endif

#endif
