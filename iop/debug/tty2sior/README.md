# TTY2SIOR

IRX driver designed to redirect all IOP printf into EE serial port

## Usage

Make sure your code follows the requirements Listed below and load the IRX.

Since the purpose of this is redirecting printf, the most sane thing to do would be to load this IRX right after IOP reset, so you can catch printf of all loaded drivers

### Requirements

1. Initialize the EE SIO. this can be done via [`sio_init(38400, 0, 0, 0, 0);`](https://github.com/ps2dev/ps2sdk/blob/master/ee/kernel/include/sio.h#L96) or with the SIOCookie library (wich is part of [ps2sdk-ports](https://github.com/ps2dev/ps2sdk-ports))
2. add the sior library to your program from the EE side and start its thread with the following function [`SIOR_Init(int thread_priority);`](https://github.com/ps2dev/ps2sdk/blob/master/ee/rpc/sior/src/sior_rpc.c#L137-L160) (wLaunchELF code uses `0x20` for the priority)
3. Load the sior2tty irx driver.

## Notes

This driver will create a `tty:` device. and file descriptors `0`, `1` and `2` will be closed and re-opened to that device. ensuring all printf on IOP is redirected
