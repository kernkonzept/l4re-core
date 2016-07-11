	.file	"initfini.c"
	
	.section .init
	.global	_init
	.type	_init, %function
	.align	2
	ldp	x29, x30, [sp], #16
	ret

	.section .fini
	.global	_fini
	.type	_fini, %function
	.align	2
	ldp	x29, x30, [sp], #16
	ret

