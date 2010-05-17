/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# PS2 TCP/IP SOCKET EXPORT FUNCTIONS.
*/



	.text
	.set	noreorder
	.global func_dec
   	.global iop_module

	.extern lwip_accept
	.extern lwip_bind
	.extern lwip_close
	.extern	lwip_connect	
	.extern lwip_listen
	.extern lwip_recv
	.extern lwip_recvfrom
	.extern	lwip_send
	.extern	lwip_sendto
	.extern	lwip_socket
	.extern lwip_select
	.extern lwip_ioctl
	.extern lwip_getpeername
	.extern	lwip_getsockname
	.extern lwip_getsockopt	
	.extern lwip_setsockopt
	.extern ps2ip_getconfig	
	.extern ps2ip_setconfig

	.extern netif_add
	.extern	netif_find
	.extern	netif_set_default
	.extern	netif_set_ipaddr
	.extern netif_set_netmask
	.extern netif_set_gw
	.extern tcpip_input

	.extern	pbuf_alloc
 	.extern pbuf_realloc
	.extern	pbuf_header
	.extern pbuf_ref
	.extern pbuf_ref_chain
	.extern pbuf_free
	.extern pbuf_clen
	.extern	pbuf_chain
	.extern pbuf_dechain
	.extern pbuf_take

	.extern start

iop_module:
	.word	0x41c00000
	.word	0
	.word	0x00000102
	.ascii	"ps2ip\0\0\0"

	.align	2

func_dec:
	.word   _start
	.word   do_nothing
	.word   do_nothing
	.word   do_nothing
	.word   lwip_accept			#004
	.word	lwip_bind
	.word	lwip_close			#006
	.word	lwip_connect
	.word	lwip_listen			#008
	.word	lwip_recv
	.word	lwip_recvfrom		#00A
	.word	lwip_send
	.word	lwip_sendto			#00C
	.word	lwip_socket
	.word	lwip_select			#00E
	.word	lwip_getpeername
	.word	lwip_getsockname		#010
	.word	lwip_getsockopt	
	.word	lwip_setsockopt			#012
	.word	lwip_ioctl			
	.word	ps2ip_getconfig			#014
	.word	ps2ip_setconfig
	.word	do_nothing			#016
	.word	do_nothing
	.word	do_nothing			#018
	.word	do_nothing
	.word	netif_add			#01A
	.word	netif_find
	.word	netif_set_default	#01C
	.word	netif_set_ipaddr
	.word	netif_set_netmask	#01E
	.word	netif_set_gw
	.word	tcpip_input			#020
	.word	pbuf_alloc
	.word	pbuf_realloc		#022
	.word	pbuf_header
	.word	pbuf_ref			#024
	.word	pbuf_ref_chain
	.word	pbuf_free			#026
	.word	pbuf_clen
	.word	pbuf_chain			#028
	.word	pbuf_dechain
	.word	pbuf_take			#02A

	.word   0

do_nothing:
	.word   0x03e00008
	.word   0


