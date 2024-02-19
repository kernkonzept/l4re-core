
.global _setjmp
.type _setjmp,%function

_setjmp:
	mov	x1, #0
	b	__sigsetjmp

.size _setjmp,.-_setjmp

