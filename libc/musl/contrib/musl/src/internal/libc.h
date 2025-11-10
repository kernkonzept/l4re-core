#ifndef LIBC_H
#define LIBC_H

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

struct __locale_map;

struct __locale_struct {
	const struct __locale_map *cat[6];
};

struct tls_module {
	struct tls_module *next;
	void *image;
	size_t len, size, align, offset;
};

struct __libc {
	char can_do_threads;
	char threaded;
	char secure;
	volatile signed char need_locks;
	/**
	 * Number of alive threads (minus 1).
	 *
	 * The -1 is the main thread, which is not part of the count, but part of the
	 * pthread list.
	 *
	 * The L4re-specific pthread manager thread is not accounted for, since
	 * unlike the main thread it is not part of the pthread list.
	 *
	 * Original musl access semantics:
	 * According to 8f11e6127fe93093f81a52b15bb1537edc3fc8af, writes to
	 * threads_minus_1 are guarded by tl_lock.
	 * The read in load_library() from dlopen() is guaranteed to be accurate or an
	 * overstimate, because dlopen() acquires __inhibit_ptc(), so no new threads
	 * can be created concurrently.
	 *
	 * L4re access semantics:
	 * Since we have no thread list lock (tl_lock) on L4Re, all accesses to
	 * threads_minus_1 must be atomic.
	 * When running under __inhibit_ptc(), threads_minus_1 is guaranteed to
	 * be accurate or an overstimate.
	 * When running under __inhibit_ptc() and in addition executing in the
	 * context of the pthread manager thread (this is our replacement for
	 * the tl_lock), e.g. through pthread_l4_exec_in_manager, the observed
	 * threads_minus_1 is guaranteed to be accurate.
	 */
	int threads_minus_1;
	size_t *auxv;
	struct tls_module *tls_head;
	size_t tls_size, tls_align, tls_cnt;
	size_t page_size;
	struct __locale_struct global_locale;
};

#ifndef PAGE_SIZE
#define PAGE_SIZE libc.page_size
#endif

extern hidden struct __libc __libc;
#define libc __libc

hidden void __init_libc(char **, char *);
hidden void __init_tls(size_t *);
hidden void __init_ssp(void *);
hidden void __libc_start_init(void);
hidden void __funcs_on_exit(void);
hidden void __funcs_on_quick_exit(void);
hidden void __libc_exit_fini(void);
hidden void __fork_handler(int);

extern hidden size_t __hwcap;
extern hidden size_t __sysinfo;
extern char *__progname, *__progname_full;

extern hidden const char __libc_version[];

hidden void __synccall(void (*)(void *), void *);
hidden int __setxid(int, int, int, int);

#endif
