
#include <stdlib.h>

char *secure_getenv(const char *name) {
	if (issetugid()) return NULL;
	return getenv(name);
}

