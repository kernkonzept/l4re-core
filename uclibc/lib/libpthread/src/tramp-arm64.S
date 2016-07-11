.text
.global __pthread_new_thread_entry
__pthread_new_thread_entry:
	ldr x0, [sp, #16]
	ldr x1, [sp, #0]
	add x9, sp, #24
	bic x9, x9, #0xf
	mov sp, x9
	mov x30, #0
	br x1
