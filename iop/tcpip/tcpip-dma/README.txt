A clean port of LWIP v1.1.1 for use with smap-new of PS2ETH.

Unlike the original LWIP v1.1.1 port (PS2IP module), this one doesn't expose the TCP/IP thread's message box and hence doesn't require any modifications to the original LWIP sources. It will hence only support a threaded SMAP driver (without NETMAN), which is why SMAP-NEW is the only driver that will work.