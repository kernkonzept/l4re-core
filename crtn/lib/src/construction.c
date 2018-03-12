/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <l4/crtn/crt0.h>

//#define DEBUG

#ifdef DEBUG
#include <l4/sys/kdebug.h>
#endif

int __cxa_atexit(void (*function)(void*), void *arg, void *dso_handle);

#define BEG		{ (crt0_hook) ~1UL }
#define END		{ (crt0_hook)  0UL }

// make sure that unused symbols are not discarded
#if (__GNUC__ == 3 && __GNUC_MINOR__ >= 3) || __GNUC__ >= 4
#define SECTION(x)	__attribute__((used, section( x )))
#else
#define SECTION(x)	__attribute__((section( x )))
#endif

typedef void (*const crt0_hook)(void);

static crt0_hook const __CTOR_BEG__[1]   SECTION(".mark_beg_ctors")   = BEG;
static crt0_hook const __CTOR_END__[1]   SECTION(".mark_end_ctors")   = END;
static crt0_hook const __C_CTOR_BEG__[1] SECTION(".mark_beg_c_ctors") = BEG;
static crt0_hook const __C_CTOR_END__[1] SECTION(".mark_end_c_ctors") = END;
static crt0_hook const __DTOR_BEG__[1]   SECTION(".mark_beg_dtors")   = BEG;
static crt0_hook const __DTOR_END__[1]   SECTION(".mark_end_dtors")   = END;
static crt0_hook const __C_DTOR_BEG__[1] SECTION(".mark_beg_c_dtors") = BEG;
static crt0_hook const __C_DTOR_END__[1] SECTION(".mark_end_c_dtors") = END;


static void
run_hooks_forward(crt0_hook *list, const char *name)
{
  (void)name;
#ifdef DEBUG
  outstring("list (forward) ");
  outstring(name);
  outstring(" @ ");
  outhex32((unsigned)list);
  outchar('\n');
#endif
  list++;
  while (*list)
    {
#ifdef DEBUG
      outstring("  calling ");
      outhex32((unsigned)*list);
      outchar('\n');
#endif
      (**list)();
      list++;
    }
}

static void
run_hooks_backward(crt0_hook *list, const char *name)
{
  (void)name;
#ifdef DEBUG
  outstring("list (backward) ");
  outstring(name);
  outstring(" @ ");
  outhex32((unsigned)list);
  outchar('\n');
#endif
  list--;
  while (*list != (crt0_hook)~1UL)
    {
#ifdef DEBUG
      outstring("  calling ");
      outhex32((unsigned)*list);
      outchar('\n');
#endif
      (**list)();
      list--;
    }
}

static void
static_construction(void)
{
  /* call constructors made with L4_C_CTOR */
  run_hooks_forward(__C_CTOR_BEG__, "__C_CTOR_BEG__");

  /* call constructors made with __attribute__((constructor))
   * and static C++ constructors */
  run_hooks_backward(__CTOR_END__, "__CTOR_END__");
}

static void
static_destruction(void)
{
  /* call destructors made with __attribute__((destructor))
   * and static C++ destructors */
  run_hooks_forward(__DTOR_BEG__, "__DTOR_BEG__");

  /* call destructors made with L4_C_DTOR except system destructors */
  run_hooks_backward(__C_DTOR_END__, "__C_DTOR_END__");
}

extern void *__dso_handle __attribute__((weak));

/* is called by crt0 immediately before calling __main() */
void
crt0_construction(void)
{
  static_construction();
  __cxa_atexit((void (*)(void*))&static_destruction, 0,
      &__dso_handle == 0 ? 0 : __dso_handle);
}

asm (".hidden _init");

/* this special function is called for initializing static libraries */
void _init(void) __attribute__((section(".init")));
void
_init(void)
{
  static_construction();
}
