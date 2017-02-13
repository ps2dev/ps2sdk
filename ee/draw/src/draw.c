#include <dma_tags.h>
#include <gif_tags.h>

#include <gs_privileged.h>
#include <gs_gp.h>
#include <gs_psm.h>

#include <draw.h>
#include <draw2d.h>

qword_t *draw_setup_environment(qword_t *q, int context, framebuffer_t *frame, zbuffer_t *z)
{

	// Change this if modifying the gif packet after the giftag.
	int qword_count = 15;

	atest_t atest;
	dtest_t dtest;
	ztest_t ztest;
	blend_t blend;
	texwrap_t wrap;

	atest.enable = DRAW_ENABLE;
	atest.method = ATEST_METHOD_NOTEQUAL;
	atest.compval = 0x00;
	atest.keep = ATEST_KEEP_FRAMEBUFFER;

	dtest.enable = DRAW_DISABLE;
	dtest.pass = DRAW_DISABLE;

	// Enable or Disable ZBuffer
	if (z->enable) {
		ztest.enable = DRAW_ENABLE;
		ztest.method = z->method;
	} else {
		z->mask = 1;
		ztest.enable = DRAW_ENABLE;
		ztest.method = ZTEST_METHOD_ALLPASS;
	}

	// Setup alpha blending
	blend.color1 = BLEND_COLOR_SOURCE;
	blend.color2 = BLEND_COLOR_DEST;
	blend.alpha = BLEND_ALPHA_SOURCE;
	blend.color3 = BLEND_COLOR_DEST;
	blend.fixed_alpha = 0x80;

	// Setup whole texture clamping
	wrap.horizontal = WRAP_CLAMP;
	wrap.vertical = WRAP_CLAMP;
	wrap.minu = wrap.maxu = 0;
	wrap.minv = wrap.maxv = 0;

	// Begin packed gif data packet with another qword.
	PACK_GIFTAG(q, GIF_SET_TAG(qword_count, 0, 0, 0, GIF_FLG_PACKED, 1), GIF_REG_AD);
	q++;
	// Framebuffer setting
	PACK_GIFTAG(q, GS_SET_FRAME(frame->address >> 11, frame->width >> 6, frame->psm, frame->mask), GS_REG_FRAME + context);
	q++;
	// ZBuffer setting
	PACK_GIFTAG(q, GS_SET_ZBUF(z->address >> 11, z->zsm, z->mask), GS_REG_ZBUF + context);
	q++;
	// Override Primitive Control
	PACK_GIFTAG(q, GS_SET_PRMODECONT(PRIM_OVERRIDE_DISABLE), GS_REG_PRMODECONT);
	q++;
	// Primitive coordinate offsets
	PACK_GIFTAG(q, GS_SET_XYOFFSET(ftoi4(2048.0f), ftoi4(2048.0f)), GS_REG_XYOFFSET + context);
	q++;
	// Scissoring area
	PACK_GIFTAG(q, GS_SET_SCISSOR(0, frame->width - 1, 0, frame->height - 1), GS_REG_SCISSOR + context);
	q++;
	// Pixel testing
	PACK_GIFTAG(q, GS_SET_TEST(atest.enable, atest.method, atest.compval, atest.keep,
	                           dtest.enable, dtest.pass,
	                           ztest.enable, ztest.method),
	            GS_REG_TEST + context);
	q++;
	// Fog Color
	PACK_GIFTAG(q, GS_SET_FOGCOL(0, 0, 0), GS_REG_FOGCOL);
	q++;
	// Per-pixel Alpha Blending (Blends if MSB of ALPHA is true)
	PACK_GIFTAG(q, GS_SET_PABE(DRAW_DISABLE), GS_REG_PABE);
	q++;
	// Alpha Blending
	PACK_GIFTAG(q, GS_SET_ALPHA(blend.color1, blend.color2, blend.alpha,
	                            blend.color3, blend.fixed_alpha),
	            GS_REG_ALPHA + context);
	q++;
	// Dithering
	PACK_GIFTAG(q, GS_SET_DTHE(GS_DISABLE), GS_REG_DTHE);
	q++;
	PACK_GIFTAG(q, GS_SET_DIMX(4, 2, 5, 3, 0, 6, 1, 7, 5, 3, 4, 2, 1, 7, 0, 6), GS_REG_DIMX);
	q++;
	// Color Clamp
	PACK_GIFTAG(q, GS_SET_COLCLAMP(GS_ENABLE), GS_REG_COLCLAMP);
	q++;
	// Alpha Correction
	if ((frame->psm == GS_PSM_16) || (frame->psm == GS_PSM_16S)) {
		PACK_GIFTAG(q, GS_SET_FBA(ALPHA_CORRECT_RGBA16), GS_REG_FBA + context);
		q++;
	} else {
		PACK_GIFTAG(q, GS_SET_FBA(ALPHA_CORRECT_RGBA32), GS_REG_FBA + context);
		q++;
	}
	// Texture wrapping/clamping
	PACK_GIFTAG(q, GS_SET_CLAMP(wrap.horizontal, wrap.vertical, wrap.minu,
	                            wrap.maxu, wrap.minv, wrap.maxv),
	            GS_REG_CLAMP + context);
	q++;
	PACK_GIFTAG(q, GS_SET_TEXA(0x80, ALPHA_EXPAND_NORMAL, 0x80), GS_REG_TEXA);
	q++;

	return q;
}

qword_t *draw_disable_tests(qword_t *q, int context, zbuffer_t *z)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1, 0, 0, 0, GIF_FLG_PACKED, 1), GIF_REG_AD);
	q++;
	PACK_GIFTAG(q, GS_SET_TEST(DRAW_ENABLE, ATEST_METHOD_NOTEQUAL, 0x00, ATEST_KEEP_FRAMEBUFFER,
	                           DRAW_DISABLE, DRAW_DISABLE,
	                           DRAW_ENABLE, ZTEST_METHOD_ALLPASS),
	            GS_REG_TEST + context);
	q++;

	return q;
}

qword_t *draw_enable_tests(qword_t *q, int context, zbuffer_t *z)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1, 0, 0, 0, GIF_FLG_PACKED, 1), GIF_REG_AD);
	q++;
	PACK_GIFTAG(q, GS_SET_TEST(DRAW_ENABLE, ATEST_METHOD_NOTEQUAL, 0x00, ATEST_KEEP_FRAMEBUFFER,
	                           DRAW_DISABLE, DRAW_DISABLE,
	                           DRAW_ENABLE, z->method),
	            GS_REG_TEST + context);
	q++;

	return q;
}

qword_t *draw_clear(qword_t *q, int context, float x, float y, float width, float height, int r, int g, int b)
{

	rect_t rect;

	union
	{
		float fvalue;
		u32 ivalue;
	} q0 = {
	    1.0f};

	rect.v0.x = x;
	rect.v0.y = y;
	rect.v0.z = 0x00000000;

	rect.color.rgbaq = GS_SET_RGBAQ(r, g, b, 0x80, q0.ivalue);

	rect.v1.x = x + width - 0.9375f;
	rect.v1.y = y + height - 0.9375f;
	rect.v1.z = 0x00000000;

	PACK_GIFTAG(q, GIF_SET_TAG(2, 0, 0, 0, 0, 1), GIF_REG_AD);
	q++;
	PACK_GIFTAG(q, GS_SET_PRMODECONT(PRIM_OVERRIDE_ENABLE), GS_REG_PRMODECONT);
	q++;
	PACK_GIFTAG(q, GS_SET_PRMODE(0, 0, 0, 0, 0, 0, context, 1), GS_REG_PRMODE);
	q++;

	q = draw_rect_filled_strips(q, context, &rect);

	PACK_GIFTAG(q, GIF_SET_TAG(1, 0, 0, 0, 0, 1), GIF_REG_AD);
	q++;
	PACK_GIFTAG(q, GS_SET_PRMODECONT(PRIM_OVERRIDE_DISABLE), GS_REG_PRMODECONT);
	q++;

	return q;
}

qword_t *draw_finish(qword_t *q)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1, 1, 0, 0, GIF_FLG_PACKED, 1), GIF_REG_AD);
	q++;
	PACK_GIFTAG(q, 1, GS_REG_FINISH);
	q++;

	return q;
}

void draw_wait_finish(void)
{

	while (!(*GS_REG_CSR & 2))
		;
	*GS_REG_CSR |= 2;
}

qword_t *draw_texture_flush(qword_t *q)
{

	// Flush texture buffer
	DMATAG_END(q, 2, 0, 0, 0);
	q++;
	PACK_GIFTAG(q, GIF_SET_TAG(1, 1, 0, 0, GIF_FLG_PACKED, 1), GIF_REG_AD);
	q++;
	PACK_GIFTAG(q, 1, GS_REG_TEXFLUSH);
	q++;

	return q;
}

qword_t *draw_texture_transfer(qword_t *q, void *src, int width, int height, int psm, int dest, int dest_width)
{

	int i;
	int remaining;
	int qwords = 0;

	switch (psm) {
		case GS_PSM_8: {
			qwords = (width * height) >> 4;
			break;
		}

		case GS_PSM_32:
		case GS_PSM_24: {
			qwords = (width * height) >> 2;
			break;
		}

		case GS_PSM_4: {
			qwords = (width * height) >> 5;
			break;
		}

		case GS_PSM_16:
		case GS_PSM_16S: {
			qwords = (width * height) >> 3;
			break;
		}

		default: {
			switch (psm) {
				case GS_PSM_8H: {
					qwords = (width * height) >> 4;
					break;
				}

				case GS_PSMZ_32:
				case GS_PSMZ_24: {
					qwords = (width * height) >> 2;
					break;
				}

				case GS_PSMZ_16:
				case GS_PSMZ_16S: {
					qwords = (width * height) >> 3;
					break;
				}

				case GS_PSM_4HL:
				case GS_PSM_4HH: {
					qwords = (width * height) >> 5;
					break;
				}
			}
			break;
		}
	}

	// Determine number of iterations based on the number of qwords
	// that can be handled per dmatag
	i = qwords / GIF_BLOCK_SIZE;

	// Now calculate the remaining image data left over
	remaining = qwords % GIF_BLOCK_SIZE;

	// Setup the transfer
	DMATAG_CNT(q, 5, 0, 0, 0);
	q++;
	PACK_GIFTAG(q, GIF_SET_TAG(4, 0, 0, 0, GIF_FLG_PACKED, 1), GIF_REG_AD);
	q++;
	PACK_GIFTAG(q, GS_SET_BITBLTBUF(0, 0, 0, dest >> 6, dest_width >> 6, psm), GS_REG_BITBLTBUF);
	q++;
	PACK_GIFTAG(q, GS_SET_TRXPOS(0, 0, 0, 0, 0), GS_REG_TRXPOS);
	q++;
	PACK_GIFTAG(q, GS_SET_TRXREG(width, height), GS_REG_TRXREG);
	q++;
	PACK_GIFTAG(q, GS_SET_TRXDIR(0), GS_REG_TRXDIR);
	q++;


	while (i-- > 0) {

		// Setup image data dma chain
		DMATAG_CNT(q, 1, 0, 0, 0);
		q++;
		PACK_GIFTAG(q, GIF_SET_TAG(GIF_BLOCK_SIZE, 0, 0, 0, 2, 0), 0);
		q++;
		DMATAG_REF(q, GIF_BLOCK_SIZE, (unsigned int)src, 0, 0, 0);
		q++;

		//Now increment the address by the number of qwords in bytes
		src += (GIF_BLOCK_SIZE * 16);
	}

	if (remaining) {

		// Setup remaining image data dma chain
		DMATAG_CNT(q, 1, 0, 0, 0);
		q++;
		PACK_GIFTAG(q, GIF_SET_TAG(remaining, 0, 0, 0, 2, 0), 0);
		q++;
		DMATAG_REF(q, remaining, (unsigned int)src, 0, 0, 0);
		q++;
	}

	return q;
}

unsigned char draw_log2(unsigned int x)
{

	unsigned char res;

	__asm__ __volatile__("plzcw %0, %1\n\t"
	                     : "=r"(res)
	                     : "r"(x));

	res = 31 - (res + 1);
	res += (x > (1 << res) ? 1 : 0);

	return res;
}
