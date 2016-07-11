/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <l4/sys/consts.h>
#include <l4/re/env.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <sched.h>

int __ctype_b_loc(void);
int getloadavg(double loadavg[], int nelem);

/* Implementations */
int __ctype_b_loc(void)
{
  printf("%s: implement me \n", __func__);
  return 0;
}

int __sched_cpucount(size_t __setsize, const cpu_set_t *__setp)
{
  (void)__setsize;
  (void)__setp;
  return 4; // just some number
}

long sysconf(int name)
{
  switch (name)
  {
  case _SC_NPROCESSORS_ONLN:
    return __sched_cpucount(0, NULL);
  case _SC_PAGE_SIZE:
    return L4_PAGESIZE;
  case _SC_CLK_TCK:
    return 1000;
  case _SC_MONOTONIC_CLOCK:
    return 200112L;
  default:
    break;
  }
  printf("%s: unknown command, name=%d\n", __func__, name);
  return 0;
}

int getloadavg(double loadavg[], int nelem)
{
  (void)nelem;
  loadavg[0] = 0.7;
  loadavg[1] = 0.7;
  loadavg[2] = 0.7;
  return 3;
}

pid_t getpid(void)
{
  return 2;
}

pid_t fork(void)
{
  printf("Unimplemented: fork()\n");
  errno = -ENOMEM;
  return -1;
}

int daemon(int nochdir, int noclose)
{
  printf("Unimplemented: daemon(%d, %d)\n", nochdir, noclose);
  errno = -ENOMEM;
  return -1;
}

int kill(pid_t pid, int sig)
{
  printf("Unimplemented: kill(%d, %d)\n", pid, sig);
  errno = -EINVAL;
  return -1;
}

#include <time.h>

int timer_delete(timer_t timer_id)
{
  printf("Unimplemented: %s(timer_id)\n", __func__);
  (void)timer_id;
  errno = -EINVAL;
  return -1;
}

int timer_gettime(timer_t timer_id, struct itimerspec *setting)
{
  printf("Unimplemented: %s(timer_id)\n", __func__);
  (void)timer_id;
  (void)setting;
  errno = -EINVAL;
  return -1;
}

int timer_settime(timer_t timer_id, int __flags,
                  __const struct itimerspec *__restrict __value,
                  struct itimerspec *__restrict __ovalue)
{
  printf("Unimplemented: %s(timer_id)\n", __func__);
  (void)timer_id;
  (void)__value;
  (void)__ovalue;
  (void)__flags;
  errno = -EINVAL;
  return -1;
}

int timer_create (clockid_t __clock_id,
                  struct sigevent *__restrict __evp,
                  timer_t *__restrict __timerid)
{
  printf("Unimplemented: %s(clock_id)\n", __func__);
  (void)__clock_id;
  (void)__evp;
  (void)__timerid;
  errno = -EINVAL;
  return -1;
}

#include <sys/times.h>

clock_t times(struct tms *buf)
{
  // some arbitrary values
  buf->tms_utime = (clock_t)l4_kip_clock(l4re_kip());
  buf->tms_stime = 10;
  buf->tms_cutime = 0;
  buf->tms_cstime = 0;
  return (clock_t)l4_kip_clock(l4re_kip());
}


#include <stdlib.h>
char *ptsname(int fd);
char *ptsname(int fd)
{
  printf("unimplemented: %s(%d)\n", __func__, fd);
  return "unimplemented-ptsname";
}

#include <pty.h>

int setuid(uid_t uid)
{
  printf("Unimplemented: %s(%d)\n", __func__, uid);
  return -1;
}

pid_t setsid(void)
{
  printf("Unimplemented: %s()\n", __func__);
  return -1;
}

int setgid(gid_t gid)
{
  printf("Unimplemented: %s(%d)\n", __func__, gid);
  return -1;
}


mode_t umask(mode_t mask)
{
  printf("Unimplemented: %s(%d)\n", __func__, mask);
  return 0;
}

int pipe(int pipefd[2])
{
  printf("Unimplemented: %s()\n", __func__);
  printf("    Caller %p\n", __builtin_return_address(0));
  (void)pipefd;
  errno = EINVAL;
  return -1;
}

#include <sys/wait.h>
pid_t waitpid(pid_t pid, int *status, int options)
{
  printf("Unimplemented: %s(%d)\n", __func__, pid);
  (void)status;
  (void)options;
  errno = EINVAL;
  return -1;
}

FILE *popen(const char *command, const char *type)
{
  printf("Unimplemented: %s(%s, %s)\n", __func__, command, type);
  return NULL;
}

int pclose(FILE *stream)
{
  printf("Unimplemented: %s(..)\n", __func__);
  (void)stream;
  return 0;
}



int execv(const char *path, char *const argv[])
{
  printf("Unimplemented: %s(%s)\n", __func__, path);
  (void)argv;
  errno = EINVAL;
  return -1;
}

int execvp(const char *file, char *const argv[])
{
  printf("Unimplemented: %s(%s)\n", __func__, file);
  (void)argv;
  errno = EINVAL;
  return -1;
}

int execve(const char *filename, char *const argv[],
           char *const envp[])
{
  printf("Unimplemented: %s(%s)\n", __func__, filename);
  (void)argv;
  (void)envp;
  errno = EINVAL;
  return -1;
}

int execl(const char *path, const char *arg, ...)
{
  printf("Unimplemented: %s(%s)\n", __func__, path);
  (void)arg;
  errno = EINVAL;
  return -1;
}

int execlp(const char *file, const char *arg, ...)
{
  printf("Unimplemented: %s(%s)\n", __func__, file);
  (void)arg;
  errno = EINVAL;
  return -1;
}

long fpathconf(int fd, int name)
{
  printf("Unimplemented: %s(%d, %d)\n", __func__, fd, name);
  errno = EINVAL;
  return -1;
}

long pathconf(const char *path, int name)
{
  printf("Unimplemented: %s(%s, %d)\n", __func__, path, name);
  errno = EINVAL;
  return -1;
}





#include <termios.h>

int tcsendbreak(int fd, int duration)
{
  printf("Unimplemented: %s()\n", __func__);
  (void)fd; (void)duration;
  errno = EINVAL;
  return -1;
}

void cfmakeraw(struct termios *termios_p)
{
  printf("Unimplemented: %s()\n", __func__);
  (void)termios_p;
}

int openpty(int *amaster, int *aslave, char *name,
            struct termios *termp, struct winsize *winp)
{
  printf("Unimplemented: %s(.., .., %s, ..)\n", __func__, name);
  (void)amaster; (void)aslave;
  (void)termp; (void)winp;
  errno = EINVAL;
  return -1;
}





uid_t getuid(void) { return 123; }
uid_t getgid(void) { return 123; }
uid_t geteuid(void) { return 123; }
uid_t getegid(void) { return 123; }
pid_t getpgrp(void) { return 7; }
pid_t getppid(void) { return 122; }

int setpgid(pid_t pid, pid_t pgid)
{
  printf("Unimplemented: %s(%d, %d)\n", __func__, pid, pgid);
  errno = EPERM;
  return -1;
}


int getgroups(int size, gid_t list[])
{
  (void)size; (void)list;
  printf("Unimplemented: %s()\n", __func__);
  errno = EPERM;
  return -1;
}

pid_t wait3(int *status, int options, struct rusage *rusage)
{
  printf("Unimplemented: %s(%p, %d, %p)\n", __func__,
         status, options, rusage);

  errno = EPERM;
  return -1;
}



#include <sys/types.h>

int getrlimit(__rlimit_resource_t resource, struct rlimit *rlim)
{
  printf("Unimplemented: %s(%d, %p)\n", __func__, resource, rlim);
  errno = EINVAL;
  return -1;
}

int setrlimit(__rlimit_resource_t resource, const struct rlimit *rlim)
{
  printf("Unimplemented: %s(%d, %p)\n", __func__,
         resource, rlim);
  errno = EINVAL;
  return -1;
}

#ifdef __USE_LARGEFILE64
int setrlimit64(__rlimit_resource_t resource, const struct rlimit64 *rlim)
{
  printf("Unimplemented: %s(%d, %p)\n", __func__,
         resource, rlim);
  errno = EINVAL;
  return -1;
}
#else
#warning No large-file support enabled?
#endif

char *getpass( const char *prompt)
{
  printf("This would be the prompt: '%s', delivering something static\n",
         prompt);
  return "THE FAMOUS PASSWORD";
}

int system(const char *path)
{
  (void)path;
  errno = EINVAL;
  return -1;
}

#include <string.h>

int uname(struct utsname *u)
{
  strncpy(u->sysname, "Fiasco.OC/L4Re", sizeof(u->sysname));
  u->sysname[sizeof(u->sysname) - 1] = 0;

  strncpy(u->nodename, "localhost", sizeof(u->nodename));
  u->nodename[sizeof(u->nodename) - 1] = 0;

  strncpy(u->release, "L4Re", sizeof(u->release));
  u->release[sizeof(u->release) - 1] = 0;

  strncpy(u->version, "0.x", sizeof(u->version));
  u->version[sizeof(u->version) - 1] = 0;

#ifdef ARCH_arm
  strncpy(u->machine, "arm", sizeof(u->machine));
#elif defined(ARCH_arm64)
  strncpy(u->machine, "aarch64", sizeof(u->machine));
#elif defined(ARCH_x86)
  strncpy(u->machine, "i686", sizeof(u->machine));
#elif defined(ARCH_amd64)
  strncpy(u->machine, "x86_64", sizeof(u->machine));
#elif defined(ARCH_ppc32)
  strncpy(u->machine, "ppc32", sizeof(u->machine));
#elif defined(ARCH_sparc)
  strncpy(u->machine, "sparcv8", sizeof(u->machine));
#elif defined(ARCH_mips)
  strncpy(u->machine, "mips", sizeof(u->machine));
#else
#error Add your arch.
#endif
  u->machine[sizeof(u->machine) - 1] = 0;

  return 0;
}

#include <sys/shm.h>

int shmget(key_t key, size_t size, int shmflg)
{
  printf("%s(%d, %zd, %d)\n", __func__, key, size, shmflg);
  errno = ENOSYS;
  return -1;
}

void *shmat(int shmid, const void *shmaddr, int shmflg)
{
  printf("%s(%d, %p, %d)\n", __func__, shmid, shmaddr, shmflg);
  errno = ENOSYS;
  return (void *)-1;
}

int shmctl(int shmid, int cmd, struct shmid_ds *buf)
{
  printf("%s(%d, %d, %p)\n", __func__, shmid, cmd, buf);
  errno = ENOSYS;
  return -1;
}

int shmdt(const void *shmaddr)
{
  printf("%s(%p)\n", __func__, shmaddr);
  errno = ENOSYS;
  return -1;
}

int getrusage(int who, struct rusage* usage)
{
	(void)who; (void)usage;
	errno = EINVAL;
	return -1;
}
