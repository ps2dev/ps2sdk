/**
 * @file
 * Input library
 */

#ifndef __INPUT_H__
#define __INPUT_H__

#include <libpad.h>

#define MODE_DIGITAL 0
#define MODE_ANALOG  1

#define MODE_UNLOCKED 0
#define MODE_LOCKED   1

#define PAD_LEFT      0x0080
#define PAD_DOWN      0x0040
#define PAD_RIGHT     0x0020
#define PAD_UP        0x0010
#define PAD_START     0x0008
#define PAD_R3        0x0004
#define PAD_L3        0x0002
#define PAD_SELECT    0x0001
#define PAD_SQUARE    0x8000
#define PAD_CROSS     0x4000
#define PAD_CIRCLE    0x2000
#define PAD_TRIANGLE  0x1000
#define PAD_R1        0x0800
#define PAD_L1        0x0400
#define PAD_R2        0x0200
#define PAD_L2        0x0100

typedef struct {
	char small;
	unsigned char large;
	char status[6];
} actuator_t;

typedef struct {

	char port;
	char slot;
	char mode;
	char lock;

	int  type;

	char state;
	char last_state;
	char exec_cmd;
	char num_modes;
	char sensitivity;

	struct padButtonStatus *buttons;

	actuator_t *actuator;
	char *buffer;

} pad_t;

#ifdef __cplusplus
extern "C" {
#endif

/** Open and initialize a pad */
pad_t *pad_open(unsigned int port, unsigned int slot, unsigned int mode, unsigned int lock);

/** Get the pad's current button status */
static inline void pad_get_buttons(pad_t *pad)
{
	padRead(pad->port,pad->slot,pad->buttons);
}

/** Close the pad */
void pad_close(pad_t *pad);

/** Wait until the pad is ready to accept commands */
void pad_wait(pad_t *pad);

#ifdef DEBUG
/** Get the pad's current state and print it */
void pad_print_state(pad_t *pad);

/** Print the pad's supported modes */
void pad_print_supported_modes(pad_t *pad);
#endif
/** Set the pad's mode and mode lock */
void pad_set_mode(pad_t *pad, int mode, int lock);

/** Set the pad's pressure sensitivity */
void pad_set_sensitivity(pad_t *pad, int enable);

/** Init actuators */
void pad_init_actuators(pad_t *pad);

/** Set actuators */
void pad_set_actuators(pad_t *pad, int small, unsigned char large);

/** Get the pad's state */
int pad_get_state(pad_t *pad);

/** Get number of modes supported by pad */
int pad_get_num_modes(pad_t *pad);

/** Check if a type is supported by pad */
int pad_has_type(pad_t *pad, int type);

/** Get the pad's type */
int pad_get_type(pad_t *pad);

/** Get the pad's current mode */
int pad_get_mode(pad_t *pad);

#ifdef __cplusplus
};
#endif

#endif /* __INPUT_H__ */
