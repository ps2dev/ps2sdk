#ifndef _BLOCK_H
#define _BLOCK_H

int blockSeekNextSegment(pfs_cache_t *clink, pfs_blockpos_t *blockpos);
u32 blockSyncPos(pfs_blockpos_t *blockpos, u64 size);
int blockInitPos(pfs_cache_t *clink, pfs_blockpos_t *blockpos, u64 position);
int blockExpandSegment(pfs_cache_t *clink, pfs_blockpos_t *blockpos, u32 count);
int blockAllocNewSegment(pfs_cache_t *clink, pfs_blockpos_t *blockpos, u32 blocks);
pfs_blockinfo* blockGetCurrent(pfs_blockpos_t *blockpos);
pfs_cache_t *blockGetNextSegment(pfs_cache_t *clink, int *result);
pfs_cache_t *blockGetLastSegmentDescriptorInode(pfs_cache_t *clink, int *result);

#endif
