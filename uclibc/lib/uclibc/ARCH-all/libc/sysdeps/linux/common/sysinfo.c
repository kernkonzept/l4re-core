
#include <sys/sysinfo.h>

/* This is for compatibility */
int sysinfo(struct sysinfo *info)
{
  info->loads[0] = 1;
  info->loads[1] = 1;
  info->loads[2] = 1;

  info->mem_unit  = 1;
  info->totalram  = 1 << 20;
  info->freeram   = 0;
  info->sharedram = 0;
  info->bufferram = 0;
  info->totalswap = 0;
  info->freeswap  = 0;
  info->procs     = 1;
  info->totalhigh = 0;
  info->freehigh  = 0;

  return 0;
}
