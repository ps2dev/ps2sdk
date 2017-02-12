void InitKernel(void);     //ExecPS2 patch only
void InitKernelFull(void); //ExecPS2 + System Configuration patches. Please refer to the comments within libkrnlupd_osd.c

//Helper functions
int kCopy(u32 *dest, const u32 *src, int size);
int PatchIsNeeded(void);
int Copy(u32 *dest, const u32 *src, int size);
void setup(int syscall_num, void *handler); //alias of "SetSyscall"
void *GetEntryAddress(int syscall);
