.text
.global __pthread_new_thread_entry
__pthread_new_thread_entry:
	ldr r0, [sp, #8]
	ldr r1, [sp, #0]
	add sp, sp, #12
	bic sp, sp, #7
        mov lr, #0
	mov pc, r1
