#ifndef _LINUX_PARAM_H
#define _LINUX_PARAM_H

#define MAXHOSTNAMELEN	64

// compile time check
#ifdef L4_PAGESIZE
#define EXEC_PAGESIZE L4_PAGESIZE
#else
#define EXEC_PAGESIZE 4096
#endif

#endif
