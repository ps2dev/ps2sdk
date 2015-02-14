A clean port of LWIP v1.1.1 for use with smap-new of PS2ETH. smap of PS2ETH is not compatible with this stack because incoming frames will be passed to the TCP/IP thread via LWIP's input functions (Called from a thread context).

Unlike the original LWIP v1.1.1 port (PS2IP module), this one doesn't expose the TCP/IP thread's message box and hence doesn't require any modifications to the original LWIP sources.
It will hence only support a SMAP driver (without NETMAN) that does not input frames from an interrupt context, which is why SMAP-NEW is the only driver that will work.
