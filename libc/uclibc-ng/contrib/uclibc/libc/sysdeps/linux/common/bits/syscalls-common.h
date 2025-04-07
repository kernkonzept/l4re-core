/*
 * Common syscall type defines
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _SYSCALLS_COMMON_H
#define _SYSCALLS_COMMON_H	1

#ifndef _SYSCALL_H
# error "Never use <bits/syscalls-common.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef SYS_ify
# define SYS_ify(syscall_name) (__NR_##syscall_name)
#endif

#ifndef __ASSEMBLER__

#include <errno.h>

#ifndef INTERNAL_SYSCALL_DECL
# define INTERNAL_SYSCALL_DECL(err)         do { } while (0)
#endif
#ifndef INTERNAL_SYSCALL_ERROR_P
# define INTERNAL_SYSCALL_ERROR_P(val, err) ((unsigned long)val >= (unsigned long)(-4095))
#endif
#ifndef INTERNAL_SYSCALL_ERRNO
# define INTERNAL_SYSCALL_ERRNO(val, err)   (-(val))
#endif

/* Define a macro which expands into the inline wrapper code for a system call */
#ifndef INLINE_SYSCALL
# define INLINE_SYSCALL(name, nr, args...) INLINE_SYSCALL_NCS(__NR_##name, nr, args)
#endif
#ifndef INLINE_SYSCALL_NOERR
# define INLINE_SYSCALL_NOERR(name, nr, args...) INLINE_SYSCALL_NOERR_NCS(__NR_##name, nr, args)
#endif

/* Just like INLINE_SYSCALL(), but take a non-constant syscall (NCS) argument */
#ifndef INLINE_SYSCALL_NCS
# define INLINE_SYSCALL_NCS(num, nr, args...)				\
(__extension__								\
 ({									\
	long __res;							\
	INTERNAL_SYSCALL_DECL(__err);					\
	(__extension__							\
	 ({								\
	   __res = INTERNAL_SYSCALL_NCS(num, __err, nr, args);		\
	   if (unlikely(INTERNAL_SYSCALL_ERROR_P(__res, __err))) {	\
		__set_errno(INTERNAL_SYSCALL_ERRNO(__res, __err));	\
		__res = -1L;						\
	   }								\
	   __res;							\
	  })								\
	);								\
  })									\
)
#endif
#ifndef INLINE_SYSCALL_NOERR_NCS
# define INLINE_SYSCALL_NOERR_NCS(num, nr, args...)			\
({									\
	long __res;							\
	INTERNAL_SYSCALL_DECL(__err);					\
	__res = INTERNAL_SYSCALL_NCS(num, __err, nr, args);		\
	__res;								\
})
#endif

/* No point in forcing people to implement both when they only need one */
#ifndef INTERNAL_SYSCALL
# define INTERNAL_SYSCALL(name, err, nr, args...) INTERNAL_SYSCALL_NCS(__NR_##name, err, nr, args)
#endif

#ifndef INTERNAL_SYSCALL_NCS
# error your port needs to define INTERNAL_SYSCALL_NCS in bits/syscalls.h
#endif

#ifndef _syscall0

#define C_DECL_ARGS_0()			void
#define C_DECL_ARGS_1(t, v)		t v
#define C_DECL_ARGS_2(t, v, args...)	t v, C_DECL_ARGS_1(args)
#define C_DECL_ARGS_3(t, v, args...)	t v, C_DECL_ARGS_2(args)
#define C_DECL_ARGS_4(t, v, args...)	t v, C_DECL_ARGS_3(args)
#define C_DECL_ARGS_5(t, v, args...)	t v, C_DECL_ARGS_4(args)
#define C_DECL_ARGS_6(t, v, args...)	t v, C_DECL_ARGS_5(args)

#define C_ARGS_0()
#define C_ARGS_1(t, v)			v
#define C_ARGS_2(t, v, args...)		v, C_ARGS_1(args)
#define C_ARGS_3(t, v, args...)		v, C_ARGS_2(args)
#define C_ARGS_4(t, v, args...)		v, C_ARGS_3(args)
#define C_ARGS_5(t, v, args...)		v, C_ARGS_4(args)
#define C_ARGS_6(t, v, args...)		v, C_ARGS_5(args)

#define SYSCALL_FUNC(nargs, type, name, args...)			\
type name(C_DECL_ARGS_##nargs(args)) {					\
	return (type)INLINE_SYSCALL(name, nargs, C_ARGS_##nargs(args));	\
}

#define SYSCALL_FUNC_TIME64(nargs, type, name, args...)			\
type name(C_DECL_ARGS_##nargs(args)) {					\
	return (type)INLINE_SYSCALL(name##_time64, nargs, C_ARGS_##nargs(args));	\
}

#define SYSCALL_FUNC_64(nargs, type, name, args...)			\
type name(C_DECL_ARGS_##nargs(args)) {					\
	return (type)INLINE_SYSCALL(name##64, nargs, C_ARGS_##nargs(args));	\
}


#define SYSCALL_NOERR_FUNC(nargs, type, name, args...)			\
type name(C_DECL_ARGS_##nargs(args)) {					\
	return (type)INLINE_SYSCALL_NOERR(name, nargs, C_ARGS_##nargs(args));	\
}

#define SYSCALL_NOERR_FUNC_TIME64(nargs, type, name, args...)			\
type name(C_DECL_ARGS_##nargs(args)) {					\
	return (type)INLINE_SYSCALL_NOERR(name##_time64, nargs, C_ARGS_##nargs(args));	\
}

#define SYSCALL_NOERR_FUNC_64(nargs, type, name, args...)			\
type name(C_DECL_ARGS_##nargs(args)) {					\
	return (type)INLINE_SYSCALL_NOERR(name##64, nargs, C_ARGS_##nargs(args));	\
}

#define SYSCALL_FUNC_BODY(nargs, type, name, args...)			\
	return (type)INLINE_SYSCALL(name, nargs, C_ARGS_##nargs(args));

#define SYSCALL_FUNC_BODY_TIME64(nargs, type, name, args...)			\
	return (type)INLINE_SYSCALL(name##_time64, nargs, C_ARGS_##nargs(args));

#define SYSCALL_FUNC_BODY_64(nargs, type, name, args...)			\
	return (type)INLINE_SYSCALL(name##64, nargs, C_ARGS_##nargs(args));

#define _syscall0(args...)		SYSCALL_FUNC(0, args)
#define _syscall_noerr0(args...)	SYSCALL_NOERR_FUNC(0, args)
#define _syscall1(args...)		SYSCALL_FUNC(1, args)
#define _syscall_noerr1(args...)	SYSCALL_NOERR_FUNC(1, args)
#define _syscall2(args...)		SYSCALL_FUNC(2, args)
#define _syscall2_body(args...)		SYSCALL_FUNC_BODY(2, args)
#define _syscall3(args...)		SYSCALL_FUNC(3, args)
#define _syscall4(args...)		SYSCALL_FUNC(4, args)
#define _syscall5(args...)		SYSCALL_FUNC(5, args)
#define _syscall6(args...)		SYSCALL_FUNC(6, args)

#define _syscall0_time64(args...)		SYSCALL_FUNC_TIME64(0, args)
#define _syscall_noerr0_time64(args...)	SYSCALL_NOERR_FUNC_TIME64(0, args)
#define _syscall1_time64(args...)		SYSCALL_FUNC_TIME64(1, args)
#define _syscall_noerr1_time64(args...)	SYSCALL_NOERR_FUNC_TIME64(1, args)
#define _syscall2_time64(args...)		SYSCALL_FUNC_TIME64(2, args)
#define _syscall2_body_time64(args...)		SYSCALL_FUNC_BODY_TIME64(2, args)
#define _syscall3_time64(args...)		SYSCALL_FUNC_TIME64(3, args)
#define _syscall4_time64(args...)		SYSCALL_FUNC_TIME64(4, args)
#define _syscall5_time64(args...)		SYSCALL_FUNC_TIME64(5, args)
#define _syscall6_time64(args...)		SYSCALL_FUNC_TIME64(6, args)

#define _syscall0_64(args...)		SYSCALL_FUNC_64(0, args)
#define _syscall_noerr0_64(args...)	SYSCALL_NOERR_FUNC_64(0, args)
#define _syscall1_64(args...)		SYSCALL_FUNC_64(1, args)
#define _syscall_noerr1_64(args...)	SYSCALL_NOERR_FUNC_64(1, args)
#define _syscall2_64(args...)		SYSCALL_FUNC_64(2, args)
#define _syscall2_body_64(args...)		SYSCALL_FUNC_BODY_64(2, args)
#define _syscall3_64(args...)		SYSCALL_FUNC_64(3, args)
#define _syscall4_64(args...)		SYSCALL_FUNC_64(4, args)
#define _syscall5_64(args...)		SYSCALL_FUNC_64(5, args)
#define _syscall6_64(args...)		SYSCALL_FUNC_64(6, args)


#endif /* _syscall0 */

#endif /* __ASSEMBLER__ */

#endif
