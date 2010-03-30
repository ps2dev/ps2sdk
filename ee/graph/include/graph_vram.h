#ifndef __GRAPH_VRAM_H__
#define __GRAPH_VRAM_H__

// Each word is 1 32-bit pixel
#define GRAPH_VRAM_MAX_WORDS		1048576

// Frame Buffer and Z Buffer
#define GRAPH_ALIGN_PAGE			2048

// Texture Buffer and CLUT Buffer
#define GRAPH_ALIGN_BLOCK			64

#ifdef __cplusplus
extern "C" {
#endif

	// Allocates vram and returns vram base pointer
	int graph_vram_allocate(int width, int height, int psm, int alignment);

	// Frees in FIFO order...
	void graph_vram_free(int address);

	// Clears the vram status
	void graph_vram_clear(void);

	// Calculate the size in vram of a texture or buffer
	int graph_vram_size(int width, int height, int psm, int alignment);

#ifdef __cplusplus
}
#endif

#endif /* __GRAPH_VRAM_H__ */
