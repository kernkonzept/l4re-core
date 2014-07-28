#if defined(__arch64__)
#include "sparc64/pthread_spin_trylock.c"
#else
#include "sparc32/pthread_spin_trylock.c"
#endif
