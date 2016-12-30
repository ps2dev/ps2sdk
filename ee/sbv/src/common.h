#define SMEM_BUF_SIZE	0x300	//Must be large enough to accommodate all operations.

struct smem_buf {
	union {
		u8 bytes[SMEM_BUF_SIZE / sizeof(u8)];
		u32 words[SMEM_BUF_SIZE / sizeof(u32)];
		smod_mod_info_t mod_info;
		slib_exp_lib_t exp_lib;
	};
};

int smem_write_word(void *address, u32 value);
int __memcmp(const void *s1, const void *s2, unsigned int length);
