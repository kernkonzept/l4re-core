.text
.global __l4_sys_syscall
.type __l4_sys_syscall, @function
__l4_sys_syscall:
#if defined(__PIC__) || defined(__PIE__)
	adrp	x16, :got:l4_global_kip
	ldr	x16, [x16, #:got_lo12:l4_global_kip]
	ldr	x16, [x16]
#else
	adrp	x16, l4_global_kip
	ldr	x16, [x16, #:lo12:l4_global_kip]
#endif
	add	x16, x16, #0x800
	br	x16
