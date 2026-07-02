/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <ioprpgen.h>
#include <string.h>
#include <stdio.h>

static int ioprpgen_dummy_writecb(void *userdata, const struct ioprpgen_ctx *ctx, const void *buf, u32 size)
{
	(void)userdata;
	(void)ctx;
	(void)buf;

	return size;
}

static int ioprpgen_membuf_writecb(void *userdata, const struct ioprpgen_ctx *ctx, const void *buf, u32 size)
{
	struct ioprpgen_memwrite_ctx *memwrite_ctx;

	(void)ctx;
	memwrite_ctx = (struct ioprpgen_memwrite_ctx *)userdata;
	if ( (memwrite_ctx->m_write_ptr_curpos + size) > memwrite_ctx->m_write_ptr_size )
		return 0;
	if ( buf )
		memcpy(&((u8 *)(memwrite_ctx->m_write_ptr))[memwrite_ctx->m_write_ptr_curpos], buf, size);
	else
		memset(&((u8 *)(memwrite_ctx->m_write_ptr))[memwrite_ctx->m_write_ptr_curpos], 0, size);
	memwrite_ctx->m_write_ptr_curpos += size;

	return size;
}

void ioprpgen_setup_membuf(struct ioprpgen_ctx *ctx, struct ioprpgen_memwrite_ctx *memwrite_ctx, void *buf, u32 size)
{
	memset(ctx, 0, sizeof(*ctx));
	if ( !buf || !size )
	{
		ctx->m_write_cb = ioprpgen_dummy_writecb;
		ctx->m_write_cb_userdata = NULL;
		return;
	}
	memwrite_ctx->m_write_ptr = buf;
	memwrite_ctx->m_write_ptr_size = size;
	memwrite_ctx->m_write_ptr_curpos = 0;
	ctx->m_write_cb = ioprpgen_membuf_writecb;
	ctx->m_write_cb_userdata = (void *)memwrite_ctx;
}

struct ioprp_romdir_entry
{
	char m_name[10];
	u16 m_extinfo_size;
	u32 m_data_size;
};

struct ioprp_extinfo_entry
{
	u16 m_value;
	u8 m_extlen;
	u8 m_type;
};

static int
ioprpgen_write_romdir_entry(const struct ioprpgen_ctx *ctx, const char *name, u32 extinfo_size, u32 data_size)
{
	struct ioprp_romdir_entry ent;
	memset(&ent, 0, sizeof(ent));
	snprintf(ent.m_name, sizeof(ent.m_name), "%s", name ? name : "");
	ent.m_extinfo_size = extinfo_size;
	ent.m_data_size = data_size;
	return ctx->m_write_cb(ctx->m_write_cb_userdata, ctx, &ent, sizeof(ent)) == sizeof(ent);
}

u32 ioprpgen_write_ioprp(const struct ioprpgen_ctx *ctx, const struct ioprpgen_entry *entries)
{
	int entry_count;
	int i;
	u32 romdir_size;
	u32 extinfo_size;
	u32 total_size;

	entry_count = 0;
	for ( i = 0; !!entries[i].m_name; i += 1 )
	{
		if (
			!strcmp(entries[i].m_name, "RESET") || !strcmp(entries[i].m_name, "ROMDIR")
			|| !strcmp(entries[i].m_name, "EXTINFO") )
			return 0;
		entry_count += 1;
	}
	if ( !ioprpgen_write_romdir_entry(ctx, "RESET", 0, 0) )
		return 0;
	romdir_size = 16 * (entry_count + 4);
	if ( !ioprpgen_write_romdir_entry(ctx, "ROMDIR", 0, romdir_size) )
		return 0;
	total_size = romdir_size;
	extinfo_size = 4 * entry_count;
	if ( !ioprpgen_write_romdir_entry(ctx, "EXTINFO", 0, extinfo_size) )
		return 0;
	for ( i = 0; i < entry_count; i += 1 )
		if ( !ioprpgen_write_romdir_entry(ctx, entries[i].m_name, 4, entries[i].m_data_size) )
			return 0;
	if ( !ioprpgen_write_romdir_entry(ctx, NULL, 0, 0) )
		return 0;
	{
		u32 cur_entry_size;
		u32 padding_size;
		u32 cur_padding_size;
		struct ioprp_extinfo_entry extinfo_entry;

		// These values are needed in order for UDNL to select the module
		extinfo_entry.m_value = 0xFFFF;
		extinfo_entry.m_extlen = 0;
		extinfo_entry.m_type = 2;
		cur_entry_size = 0;
		for ( i = 0; i < entry_count; i += 1 )
		{
			u32 cur_value_size;

			cur_value_size = ctx->m_write_cb(ctx->m_write_cb_userdata, ctx, &extinfo_entry, sizeof(extinfo_entry));
			if ( cur_value_size != sizeof(extinfo_entry) )
				return 0;
			cur_entry_size += cur_value_size;
		}
		if ( cur_entry_size != extinfo_size )
			return 0;
		total_size += cur_entry_size;
		padding_size = ((((cur_entry_size - 1) >> 4) + 1) << 4) - cur_entry_size;
		cur_padding_size = ctx->m_write_cb(ctx->m_write_cb_userdata, ctx, NULL, padding_size);
		if ( cur_padding_size != padding_size )
			return 0;
		total_size += cur_padding_size;
	}
	for ( i = 0; i < entry_count; i += 1 )
	{
		u32 cur_entry_size;
		u32 padding_size;
		u32 cur_padding_size;

		cur_entry_size = ctx->m_write_cb(ctx->m_write_cb_userdata, ctx, entries[i].m_data, entries[i].m_data_size);
		if ( cur_entry_size != entries[i].m_data_size )
			return 0;
		total_size += cur_entry_size;
		padding_size = ((((cur_entry_size - 1) >> 4) + 1) << 4) - cur_entry_size;
		cur_padding_size = ctx->m_write_cb(ctx->m_write_cb_userdata, ctx, NULL, padding_size);
		if ( cur_padding_size != padding_size )
			return 0;
		total_size += cur_padding_size;
	}
	return total_size;
}
