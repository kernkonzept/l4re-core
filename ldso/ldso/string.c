#undef _LIBC
#include "string.h"

void *memset(void *dst, int v, size_t s)
{
  char *d = (char*)dst;
  for (; s > 0; --s, ++d)
    *d = v;

  return dst;
}

void *memcpy(void *dst, void const *src, size_t si)
{
  char *d = (char *)dst;
  char const *s = (char const *)src;
  for (; si > 0; --si, ++s, ++d)
    *d = *s;

  return d;
}

size_t strlen(char const *s)
{
  long i = 0;
  for (; *s; ++s, ++i)
    ;

  return i;
}

