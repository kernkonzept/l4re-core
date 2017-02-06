.text
.global __pthread_new_thread_entry
__pthread_new_thread_entry:
	sub sp, sp, #8   /* __pthread_mgr_create_thread puts 3 * 8 bytes
	                  * onto the stack, make sure we run with a 16byte
	                  * aligned stack pointer */
	ldr x0, [sp, #24]
	ldr x1, [sp, #8]
	add x9, sp, #32
	bic x9, x9, #0xf
	mov sp, x9
	mov x30, #0
	br x1
