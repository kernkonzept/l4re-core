
.global setjmp
.type setjmp,%function

setjmp:
	mov	x1, #1
	b	__sigsetjmp

.size setjmp,.-setjmp
