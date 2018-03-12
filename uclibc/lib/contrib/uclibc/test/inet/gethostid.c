#include <unistd.h>
#include <stdio.h>
int main(void) {
	printf("hostid=%ld\n", gethostid());
	return 0;
}
