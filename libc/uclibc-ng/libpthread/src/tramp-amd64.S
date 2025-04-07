.text
.global __pthread_new_thread_entry
__pthread_new_thread_entry:
	mov   16(%rsp), %rdi
	mov    0(%rsp), %rsi
	addq       $24, %rsp
	andq      $~15, %rsp
	pushq       $0
	jmp      *%rsi
