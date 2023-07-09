
#include "irx_imports.h"
#include "ssbusc.h"

extern struct irx_export_table _exp_ssbusc;

#ifdef _IOP
IRX_ID("ssbus_service", 1, 1);
#endif

int _start(int argc, char *argv[])
{
	int state;

	(void)argc;
	(void)argv;

	CpuSuspendIntr(&state);
	if (RegisterLibraryEntries(&_exp_ssbusc)) {
		CpuResumeIntr(state);
		return MODULE_NO_RESIDENT_END;
	}
	else {
		CpuResumeIntr(state);
		return MODULE_RESIDENT_END;
	}
}

static vu32 *delay_table[13] =
{
	(vu32 *)0xBF801008,
	(vu32 *)0xBF80100C,
	(vu32 *)0xBF801010,
	(vu32 *)0,
	(vu32 *)0xBF801014,
	(vu32 *)0xBF801018,
	(vu32 *)0,
	(vu32 *)0,
	(vu32 *)0xBF80101C,
	(vu32 *)0xBF801414,
	(vu32 *)0xBF801418,
	(vu32 *)0xBF80141C,
	(vu32 *)0xBF801420,
};

int SetDelay(int device, unsigned int value)
{
	vu32 *v1;

	if (device >= (sizeof(delay_table) / sizeof(delay_table[0]))) {
		return -1;
	}
	v1 = delay_table[device];
	if (v1 == NULL) {
		return -1;
	}
	*v1 = value;
	return value;
}

int GetDelay(int device)
{
	vu32 *v1;

	if (device >= (sizeof(delay_table) / sizeof(delay_table[0]))) {
		return -1;
	}
	v1 = delay_table[device];
	if (v1 == NULL) {
		return -1;
	}
	return *v1;
}

static vu32 *base_address_table[13] =
{
	(vu32 *)0xBF801000,
	(vu32 *)0xBF801400,
	(vu32 *)0,
	(vu32 *)0,
	(vu32 *)0xBF801404,
	(vu32 *)0xBF801408,
	(vu32 *)0,
	(vu32 *)0,
	(vu32 *)0xBF801004,
	(vu32 *)0xBF80140C,
	(vu32 *)0,
	(vu32 *)0xBF801410,
	(vu32 *)0,
};

int SetBaseAddress(int device, unsigned int value)
{
	vu32 *v1;

	if (device >= (sizeof(base_address_table) / sizeof(base_address_table[0]))) {
		return -1;
	}
	v1 = base_address_table[device];
	if (v1 == NULL) {
		return -1;
	}
	*v1 = value;
	return value;
}

int GetBaseAddress(int device)
{
	vu32 *v1;

	if (device >= (sizeof(base_address_table) / sizeof(base_address_table[0]))) {
		return -1;
	}
	v1 = base_address_table[device];
	if (v1 == NULL) {
		return -1;
	}
	return *v1;
}

int SetRecoveryTime(unsigned int value)
{
	unsigned int result;

	result = (*((vu32 *)0xBF801020) & (~0xF)) | (value & 0xF);
	*((vu32 *)0xBF801020) = result;
	return result;
}

int GetRecoveryTime(void)
{
	return *((vu32 *)0xBF801020) & 0xF;
}

int SetHoldTime(unsigned int value)
{
	unsigned int result;

	result = (*((vu32 *)0xBF801020) & (~0xF0)) | ((value << 4) & 0xF0);
	*((vu32 *)0xBF801020) = result;
	return result;
}

int GetHoldTime(void)
{
	return (*((vu32 *)0xBF801020) >> 4) & 0xF;
}

int SetFloatTime(unsigned int value)
{
	unsigned int result;

	result = (*((vu32 *)0xBF801020) & (~0xF00)) | ((value << 8) & 0xF00);
	*((vu32 *)0xBF801020) = result;
	return result;
}

int GetFloatTime(void)
{
	return (*((vu32 *)0xBF801020) >> 8) & 0xF;
}

int SetStrobeTime(unsigned int value)
{
	unsigned int result;

	result = (*((vu32 *)0xBF801020) & (~0xF000)) | ((value << 12) & 0xF000);
	*((vu32 *)0xBF801020) = result;
	return result;
}

int GetStrobeTime(void)
{
	return (*((vu32 *)0xBF801020) >> 12) & 0xF;
}

int SetCommonDelay(unsigned int value)
{
	unsigned int result;

	result = value;
	*((vu32 *)0xBF801020) = value;
	return result;
}

int GetCommonDelay(void)
{
	return *((vu32 *)0xBF801020);
}
