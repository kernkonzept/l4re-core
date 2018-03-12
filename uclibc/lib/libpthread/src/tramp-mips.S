#include <sys/asm.h>
#include <sys/regdef.h>

.text
.set push
.set noreorder
.align 2
.global __pthread_new_thread_entry
.hidden __pthread_new_thread_entry
__pthread_new_thread_entry:
	PTR_L a0, (SZREG * 2)(sp)
	PTR_L t9, 0(sp)
	PTR_ADDIU sp, sp, (SZREG * 3)
	ins   sp, $0, 0, 4 # align to 16bytes
	PTR_ADDIU sp, sp, -(SZREG * NARGSAVE)
	jalr  t9
	  nop

LEAF(pthread_handle_sigcancel)
.hidden pthread_handle_sigcancel
	PTR_LA	t9, _pthread_handle_sigcancel
	jr t9
	  nop
END(pthread_handle_sigcancel)

.set pop
