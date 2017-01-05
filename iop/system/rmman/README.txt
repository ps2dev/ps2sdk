This is a compatible version of rom1:RMMAN, cloned from ROM v2.20.
This module requires a new version of SIO2MAN (export table v2.x), like rom1:SIO2MAN.

Older consoles may have an older version of RMMAN, version v1.09.
That version has no real differences in functionality. Known differences:
	1. Does not clear the connected status, if a unrecognized response is returned within FindRemote().
	2. The in_buf and out_buf fields of the SIO2 data structure are set to NULL within InitFindRmCmd().
