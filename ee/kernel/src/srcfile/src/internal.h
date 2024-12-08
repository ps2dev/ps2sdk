/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

extern void Intc12Handler(void);
extern void InvokeUserModeCallback(void *dispatcher, void *callback, int id, u32 target, void *common);
extern void ResumeIntrDispatch(void);
