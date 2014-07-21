.global _GetGsDxDyOffset
.ent _GetGsDxDyOffset
_GetGsDxDyOffset:
	li	$v1, 0x80
	syscall
	jr	$ra
	nop
.end _GetGsDxDyOffset
