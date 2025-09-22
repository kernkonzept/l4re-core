#define SYSCALL_NO_TLS 1
#include <elf.h>
#include <limits.h>
#include <sys/mman.h>
#include <string.h>
#include <stddef.h>
#include "pthread_impl.h"
#include "libc.h"
#include "atomic.h"
#include "syscall.h"
// TODO: This is kind of dirty...
#include "libc-api.h"

volatile int __thread_list_lock;

int __init_tp(pthread_descr p)
{
	int r = ptlc_set_tp(ptlc_thread_descr_to_tls_tp(p));
	if (r < 0) return -1;
	if (!r) libc.can_do_threads = 1;
	/*
	td->detach_state = DT_JOINABLE;
	td->tid = __syscall(SYS_set_tid_address, &__thread_list_lock);
	td->locale = &libc.global_locale;
	td->robust_list.head = &td->robust_list.head;
	td->sysinfo = __sysinfo;
	td->next = td->prev = td;
	*/
	__pthread_descr_libc_data(p)->locale = &libc.global_locale;

	return 0;
}

static struct builtin_tls {
	char c;
	// TODO: Give musl access to "struct pthread"?
	char pt[512]; // TOOD: The alignment might be wrong...
	// struct pthread pt; // Ooops, this is broken now...
	void *space[16];
} builtin_tls[1];
//#define MIN_TLS_ALIGN 8 //offsetof(struct builtin_tls, pt)
#define MIN_TLS_ALIGN offsetof(struct builtin_tls, pt)

static struct tls_module main_tls;

pthread_descr __copy_tls(unsigned char *mem)
{
	pthread_descr td;
	struct tls_module *p;
	size_t i;
	uintptr_t *dtv;

#ifdef TLS_ABOVE_TP
	// The DTV elements are positioned at the end of the TLS allocation.
	dtv = (uintptr_t*)(mem + libc.tls_size) - (libc.tls_cnt + 1);

	mem += -((uintptr_t)mem + __pthread_struct_size()) & (libc.tls_align-1);
	td = (pthread_descr)mem;
	mem += __pthread_struct_size();

	for (i=1, p=libc.tls_head; p; i++, p=p->next) {
		dtv[i] = (uintptr_t)(mem + p->offset) + DTP_OFFSET;
		memcpy(mem + p->offset, p->image, p->len);
	}
#else
	// The DTV elements are positioned at the start of the TLS allocation.
	dtv = (uintptr_t *)mem;

	mem += libc.tls_size - __pthread_struct_size();
	mem -= (uintptr_t)mem & (libc.tls_align-1);
	td = (pthread_descr)mem;

	for (i=1, p=libc.tls_head; p; i++, p=p->next) {
		dtv[i] = (uintptr_t)(mem - p->offset) + DTP_OFFSET;
		memcpy(mem - p->offset, p->image, p->len);
	}
#endif
	dtv[0] = libc.tls_cnt;
	__pthread_descr_libc_data(td)->dtv = dtv;
	return td;
}

#if ULONG_MAX == 0xffffffff
typedef Elf32_Phdr Phdr;
#else
typedef Elf64_Phdr Phdr;
#endif

extern weak hidden const size_t _DYNAMIC[];

static void static_init_tls(size_t *aux)
{
	unsigned char *p;
	size_t n;
	Phdr *phdr, *tls_phdr=0;
	size_t base = 0;
	void *mem;

	for (p=(void *)aux[AT_PHDR],n=aux[AT_PHNUM]; n; n--,p+=aux[AT_PHENT]) {
		phdr = (void *)p;
		if (phdr->p_type == PT_PHDR)
			base = aux[AT_PHDR] - phdr->p_vaddr;
		if (phdr->p_type == PT_DYNAMIC && _DYNAMIC)
			base = (size_t)_DYNAMIC - phdr->p_vaddr;
		if (phdr->p_type == PT_TLS)
			tls_phdr = phdr;
		if (phdr->p_type == PT_GNU_STACK &&
		    phdr->p_memsz > __default_stacksize)
			__default_stacksize =
				phdr->p_memsz < DEFAULT_STACK_MAX ?
				phdr->p_memsz : DEFAULT_STACK_MAX;
	}

	if (tls_phdr) {
		main_tls.image = (void *)(base + tls_phdr->p_vaddr);
		main_tls.len = tls_phdr->p_filesz;
		main_tls.size = tls_phdr->p_memsz;
		main_tls.align = tls_phdr->p_align;
		libc.tls_cnt = 1;
		libc.tls_head = &main_tls;
	}

	main_tls.size += (-main_tls.size - (uintptr_t)main_tls.image)
		& (main_tls.align-1);
#ifdef TLS_ABOVE_TP
	main_tls.offset = GAP_ABOVE_TP;
	main_tls.offset += (-GAP_ABOVE_TP + (uintptr_t)main_tls.image)
		& (main_tls.align-1);
#else
	main_tls.offset = main_tls.size;
#endif
	if (main_tls.align < MIN_TLS_ALIGN) main_tls.align = MIN_TLS_ALIGN;

	libc.tls_align = main_tls.align;
	libc.tls_size = 2*sizeof(void *) + __pthread_struct_size()
#ifdef TLS_ABOVE_TP
		+ main_tls.offset
#endif
		+ main_tls.size + main_tls.align
		+ MIN_TLS_ALIGN-1 & -MIN_TLS_ALIGN;

	if (libc.tls_size > sizeof builtin_tls) {
#ifndef SYS_mmap2
#define SYS_mmap2 SYS_mmap
#endif
		mem = (void *)__syscall(
			SYS_mmap2,
			0, libc.tls_size, PROT_READ|PROT_WRITE,
			MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
		/* -4095...-1 cast to void * will crash on dereference anyway,
		 * so don't bloat the init code checking for error codes and
		 * explicitly calling a_crash(). */
	} else {
		mem = builtin_tls;
	}

	/* Failure to initialize thread pointer is always fatal. */
	if (__init_tp(__copy_tls(mem)) < 0)
		a_crash();
}

weak_alias(static_init_tls, __init_tls);
