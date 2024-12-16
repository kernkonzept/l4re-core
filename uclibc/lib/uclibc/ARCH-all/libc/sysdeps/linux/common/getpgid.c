#include <features.h>

#if defined __USE_UNIX98
#include <unistd.h>
pid_t __getpgid(pid_t pid)
{
        return 1;
}
#ifdef __USE_XOPEN_EXTENDED
weak_alias(__getpgid,getpgid)
#endif
#endif
