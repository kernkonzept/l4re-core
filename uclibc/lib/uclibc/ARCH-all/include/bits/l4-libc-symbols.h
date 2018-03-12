#ifndef L4_UCLIBC_LIBC_SYMBOLS_H
#define L4_UCLIBC_LIBC_SYMBOLS_H

#define L4_UCLIBC

#define L4_UCLIBC_EXTERNAL_accept
#define L4_UCLIBC_EXTERNAL_alarm

#define L4_UCLIBC_EXTERNAL_bind
#define L4_UCLIBC_EXTERNAL_brk

#define L4_UCLIBC_EXTERNAL_chdir
#define L4_UCLIBC_EXTERNAL_chown
#define L4_UCLIBC_EXTERNAL_clock_getres
#define L4_UCLIBC_EXTERNAL_close
#define L4_UCLIBC_EXTERNAL_connect

#define L4_UCLIBC_EXTERNAL_dup2

#define L4_UCLIBC_EXTERNAL_execve
#define L4_UCLIBC_EXTERNAL__exit

#define L4_UCLIBC_EXTERNAL_fchdir
#define L4_UCLIBC_EXTERNAL_fcntl
#define L4_UCLIBC_EXTERNAL_fcntl64
#define L4_UCLIBC_EXTERNAL_fork
#define L4_UCLIBC_EXTERNAL_fstat
#define L4_UCLIBC_EXTERNAL_fstat64

#define L4_UCLIBC_EXTERNAL_getcwd
#define L4_UCLIBC_EXTERNAL_getegid
#define L4_UCLIBC_EXTERNAL_geteuid
#define L4_UCLIBC_EXTERNAL_getgid
#define L4_UCLIBC_EXTERNAL_getgroups
#define L4_UCLIBC_EXTERNAL_getpid
#define L4_UCLIBC_EXTERNAL_getuid
#define L4_UCLIBC_EXTERNAL_getsockname
#define L4_UCLIBC_EXTERNAL_gettimeofday

#define L4_UCLIBC_EXTERNAL_ioctl

#define L4_UCLIBC_EXTERNAL_kill

#define L4_UCLIBC_EXTERNAL_listen
#define L4_UCLIBC_EXTERNAL_lockf
#define L4_UCLIBC_EXTERNAL_lockf64
#define L4_UCLIBC_EXTERNAL_lseek
#define L4_UCLIBC_EXTERNAL_lseek64
#define L4_UCLIBC_EXTERNAL_lstat
#define L4_UCLIBC_EXTERNAL_lstat64

#define L4_UCLIBC_EXTERNAL_mkdir
#define L4_UCLIBC_EXTERNAL_mknod
#define L4_UCLIBC_EXTERNAL_mknodat
#define L4_UCLIBC_EXTERNAL_mmap
#define L4_UCLIBC_EXTERNAL_munmap
#define L4_UCLIBC_EXTERNAL_mremap

#define L4_UCLIBC_EXTERNAL_nanosleep

#define L4_UCLIBC_EXTERNAL_open
#define L4_UCLIBC_EXTERNAL_open64
#define L4_UCLIBC_EXTERNAL_openat
#define L4_UCLIBC_EXTERNAL_openat64

#define L4_UCLIBC_EXTERNAL_pipe

#define L4_UCLIBC_EXTERNAL_read
#define L4_UCLIBC_EXTERNAL_readlink
#define L4_UCLIBC_EXTERNAL_recv
#define L4_UCLIBC_EXTERNAL_recvfrom
#define L4_UCLIBC_EXTERNAL_recvmsg
#define L4_UCLIBC_EXTERNAL_rmdir

#define L4_UCLIBC_EXTERNAL_sbrk
#define L4_UCLIBC_EXTERNAL_send
#define L4_UCLIBC_EXTERNAL_sendmsg
#define L4_UCLIBC_EXTERNAL_sendto
#define L4_UCLIBC_EXTERNAL_select
#define L4_UCLIBC_EXTERNAL_setpid
#define L4_UCLIBC_EXTERNAL_setpgid
#define L4_UCLIBC_EXTERNAL_seteuid
#define L4_UCLIBC_EXTERNAL_setreuid
#define L4_UCLIBC_EXTERNAL_setregid
#define L4_UCLIBC_EXTERNAL_setresuid
#define L4_UCLIBC_EXTERNAL_setresgid
#define L4_UCLIBC_EXTERNAL_setsockopt
#define L4_UCLIBC_EXTERNAL_settimeofday
#define L4_UCLIBC_EXTERNAL_settimer
#define L4_UCLIBC_EXTERNAL_sigaction
#define L4_UCLIBC_EXTERNAL_signal
#define L4_UCLIBC_EXTERNAL_sigprocmask
#define L4_UCLIBC_EXTERNAL_sigsuspend
#define L4_UCLIBC_EXTERNAL_sigtimedwait
#define L4_UCLIBC_EXTERNAL_sigwaitinfo
#define L4_UCLIBC_EXTERNAL_sleep
#define L4_UCLIBC_EXTERNAL_socket
#define L4_UCLIBC_EXTERNAL_stat
#define L4_UCLIBC_EXTERNAL_stat64
#define L4_UCLIBC_EXTERNAL_sysconf

#define L4_UCLIBC_EXTERNAL_time
#define L4_UCLIBC_EXTERNAL_truncate

#define L4_UCLIBC_EXTERNAL_uname
#define L4_UCLIBC_EXTERNAL_unlink
#define L4_UCLIBC_EXTERNAL_utimensat

#define L4_UCLIBC_EXTERNAL_vfork

#define L4_UCLIBC_EXTERNAL_waitpid
#define L4_UCLIBC_EXTERNAL_wait4
#define L4_UCLIBC_EXTERNAL_write



#define __L4_UCLIBC_HIDDEN_PROTO_PLH_ 0,
#define L4_UCLIBC_HIDDEN_PROTO(x) _L4_UCLIBC_HIDDEN_PROTO(L4_UCLIBC_EXTERNAL_##x, x)
#define _L4_UCLIBC_HIDDEN_PROTO(a, b) __L4_UCLIBC_HIDDEN_PROTO(a, b)
#define __L4_UCLIBC_HIDDEN_PROTO(x, val) ___L4_UCLIBC_HIDDEN_PROTO(__L4_UCLIBC_HIDDEN_PROTO_PLH_##x ,hidden_proto(val))
#define ___L4_UCLIBC_HIDDEN_PROTO(y, z, ...) ____L4_UCLIBC_HIDDEN_PROTO(y, z)
#define ____L4_UCLIBC_HIDDEN_PROTO(y, z, ...) z

#ifndef libc_hidden_proto
#define libc_hidden_proto(x...)
#else
#if !defined NOT_IN_libc
#undef libc_hidden_proto
#define libc_hidden_proto(x) L4_UCLIBC_HIDDEN_PROTO(x)
#endif
#endif


#ifndef libm_hidden_proto
#define libm_hidden_proto(x)
#endif
#ifndef librt_hidden_proto
#define librt_hidden_proto(x)
#endif

#endif
