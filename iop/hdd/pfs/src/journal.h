#ifndef _JOURNAL_H
#define _JOURNAL_H

int journalCheckSum(void *header);
void journalWrite(pfs_mount_t *pfsMount, pfs_cache_t *clink, u32 numBuffers);
int journalReset(pfs_mount_t *pfsMount);
int journalFlush(pfs_mount_t *pfsMount);
int journalResetore(pfs_mount_t *pfsMount);
int journalResetThis(block_device *blockDev, int fd, u32 sector);

#endif /* _JOURNAL_H */

