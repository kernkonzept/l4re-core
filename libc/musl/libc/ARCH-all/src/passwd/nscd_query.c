#include <errno.h>
#include <stdio.h>
#include "nscd.h"

FILE *__nscd_query(int32_t req, const char *key, int32_t *buf, size_t len, int *swap)
{
	errno = ENOENT;
	return 0;
}
