#include <stdlib.h>

_Noreturn void abort(void)
{
  _Exit(128);
}

