/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Definitions for IOPRP generation library.
 */

#ifndef __IOPRPGEN__
#define __IOPRPGEN__

#include <tamtypes.h>

struct ioprpgen_ctx;

typedef int (*ioprpgen_write_cb_t)(void *userdata, const struct ioprpgen_ctx *ctx, const void *buf, u32 size);

struct ioprpgen_ctx
{
	ioprpgen_write_cb_t m_write_cb;
	void *m_write_cb_userdata;
};

struct ioprpgen_memwrite_ctx
{
	void *m_write_ptr;
	u32 m_write_ptr_size;
	u32 m_write_ptr_curpos;
};

struct ioprpgen_entry
{
	const char *m_name;
	const void *m_data;
	u32 m_data_size;
};

/** Setup ioprpgen context
 *
 * @param ctx Pointer to context
 * @param memwrite_ctx Pointer to memwrite context (ensure it has the same lifetime as ctx)
 * @param buf Output buffer
 * @param size Size of the output buffer
 * @return 1 on success, 0 on failure.
 */
extern void ioprpgen_setup_membuf(struct ioprpgen_ctx *ctx, struct ioprpgen_memwrite_ctx *memwrite_ctx, void *buf, u32 size);
/** Generate IOPRP image using provided entries
 *
 * @param ctx Pointer to context prepared by ioprpgen_setup_membuf
 * @param entries Pointer to NULL-terminated list of entries
 * @return 0 on failure, other value for size
 */
extern u32 ioprpgen_write_ioprp(const struct ioprpgen_ctx *ctx, const struct ioprpgen_entry *entries);

#endif /* __IOPRPGEN__ */
