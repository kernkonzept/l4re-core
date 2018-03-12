#include <stdlib.h>
#include <stdio.h>

int main(void)
{
#ifdef __UCLIBC_HAS_ARC4RANDOM__
	int random_number;
	random_number = arc4random() % 65536;
	printf("%d\n", random_number);
#endif
	return 0;
}
