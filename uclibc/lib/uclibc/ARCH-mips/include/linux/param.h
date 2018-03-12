#ifndef _LINUX_PARAM_H
#define _LINUX_PARAM_H

#include <asm/param.h>

// compile time check
#undef EXEC_PAGESIZE
#ifdef L4_PAGESIZE
#define EXEC_PAGESIZE L4_PAGESIZE
#else
#define EXEC_PAGESIZE 4096
#endif

#endif
