.text
.global __l4_sys_syscall
.type __l4_sys_syscall, #function
__l4_sys_syscall:
	svc	#0
	bx	lr
.size __l4_sys_syscall, . - __l4_sys_syscall
