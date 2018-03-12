#include <sys/regdef.h>
#include <sys/asm.h>

LEAF(loader_thread)
	.cpload $25
	SETUP_GPX64($25, $0)
	PTR_LA	$25, loader_thread_c
	PTR_ADDIU	$29, $29, -(NARGSAVE * SZREG)
	jr	$25
	  nop
END(loader_thread)
