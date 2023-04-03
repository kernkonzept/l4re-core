.text
.global __l4_sys_syscall
.type __l4_sys_syscall, @function
__l4_sys_syscall:
	adrp	x16, :got:l4_global_kip
	ldr	x16, [x16, #:got_lo12:l4_global_kip]
	ldr	x16, [x16]
	add	x16, x16, #0x800
	br	x16
.size __l4_sys_syscall, . - __l4_sys_syscall
