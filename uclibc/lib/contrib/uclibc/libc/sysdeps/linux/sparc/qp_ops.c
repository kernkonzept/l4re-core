#include <stdio.h>
#include <stdlib.h>

static void fakedef(void)
{
	fputs("Unimplemented _Q* func called, exiting\n", stderr);
	exit(-1);
}

# define fakedef(sym) strong_alias(fakedef, _Q_##sym)

fakedef(fne)
fakedef(feq)
fakedef(div)
fakedef(flt)
fakedef(fgt)
fakedef(mul)
fakedef(fge)
fakedef(qtoux)
fakedef(uxtoq)
fakedef(sub)
fakedef(dtoq)
fakedef(qtod)
fakedef(qtos)
fakedef(stoq)
fakedef(itoq)
fakedef(add)
fakedef(qtou)
fakedef(utoq)
fakedef(cmp)
fakedef(cmpe)
fakedef(fle)
fakedef(lltoq)
fakedef(neg)
fakedef(qtoi)
fakedef(qtoll)
fakedef(qtoull)
fakedef(sqrt)
fakedef(ulltoq)
