#include <gs_psm.h>

#include <graph_vram.h>

static int graph_vram_pointer = 0;

int graph_vram_allocate(int width, int height, int psm, int alignment)
{

	int size;

	// Calculate the size and increment the pointer
	size = graph_vram_size(width,height,psm,alignment);

	graph_vram_pointer += size;

	// If the pointer overflows the vram size
	if (graph_vram_pointer > GRAPH_VRAM_MAX_WORDS)
	{

		graph_vram_pointer -= size;
		return -1;

	}

	return graph_vram_pointer - size;

}

void graph_vram_free(int address)
{

	graph_vram_pointer = address;

}

void graph_vram_clear(void)
{

	graph_vram_pointer = 0;

}

int graph_vram_size(int width, int height, int psm, int alignment)
{

	int size = 0;

	// First correct the buffer width to be a multiple of 64 or 128
	switch (psm)
	{

		case GS_PSM_8:
		case GS_PSM_4:
		case GS_PSM_8H:
		case GS_PSM_4HL:
		case GS_PSM_4HH:	width = -128 & (width + 127); break;
		default:			width = -64  & (width + 63);  break;

	}

	// Texture storage size is in pixels/word
	switch (psm)
	{

		case GS_PSM_4:		size = width*(height>>3); break;
		case GS_PSM_8:		size = width*(height>>2); break;
		case GS_PSM_24:
		case GS_PSM_32:
		case GS_PSM_8H:
		case GS_PSM_4HL:
		case GS_PSM_4HH:
		case GS_PSMZ_24:
		case GS_PSMZ_32:	size = width*height; break;
		case GS_PSM_16:
		case GS_PSM_16S:
		case GS_PSMZ_16:
		case GS_PSMZ_16S:	size = width*(height>>1); break;
		default: return 0;

	}

	// The buffer size is dependent on alignment
	size = -alignment & (size + (alignment-1));

	return size;

}
