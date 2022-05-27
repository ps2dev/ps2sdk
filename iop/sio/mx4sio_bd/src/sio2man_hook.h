#ifndef SIO2MAN_HOOK_H
#define SIO2MAN_HOOK_H


int sio2man_hook_init();
void sio2man_hook_deinit();

// Lock all communication to SIO2MAN
// this is needed for drivers that communicate directly to sio2, like mx4sio.
// Do NOT lock for long duration, only for single (high speed) transfers
void sio2man_hook_sio2_lock();
void sio2man_hook_sio2_unlock();


#endif
