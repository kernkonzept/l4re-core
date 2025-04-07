/* glibc's sysdeps/xtensa/elf/initfini.c used for reference [EPILOG] */

	.section .init
#if defined(__XTENSA_WINDOWED_ABI__)
	retw
#elif defined(__XTENSA_CALL0_ABI__)
#ifdef __FDPIC__
	l32i	a12, sp, 4
#endif
	l32i	a0, sp, 0
	addi	sp, sp, 16
	ret
#else
#error Unsupported Xtensa ABI
#endif

	.section .fini
#if defined(__XTENSA_WINDOWED_ABI__)
	retw
#elif defined(__XTENSA_CALL0_ABI__)
#ifdef __FDPIC__
	l32i	a12, sp, 4
#endif
	l32i	a0, sp, 0
	addi	sp, sp, 16
	ret
#else
#error Unsupported Xtensa ABI
#endif
