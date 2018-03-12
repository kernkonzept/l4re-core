/**
 * \file
 * Low-level assert implementation.
 */
/*
 * (c) 2015 Adam Lackorzynski <adam@l4re.org>
 *
 * This file is part of L4Re and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#pragma once

#ifdef NDEBUG

#define l4_assert(x) do { } while (0)
#define l4_check(x) do { (void)(x); } while (0)

#else

#include <l4/sys/compiler.h>
#include <l4/sys/thread.h>
#include <l4/sys/vcon.h>

/**
 * Low-level assert.
 *
 * \param expr  Expression to be evaluate for the assertion.
 *
 * This assertion is a low-level implementation that directly uses
 * kernel primitives. Only use l4_assert() when the standard assert()
 * functionality is not available.
 */
#define l4_assert(expr) \
  l4_assert_fn(expr, __FILE__ ":" L4_stringify(__LINE__) ": Assertion \"" \
                     L4_stringify(expr) "\" failed.\n")

#define l4_check(expr) l4_assert(expr)

/**
 * \internal
 */
L4_ALWAYS_INLINE
void l4_assert_fn(bool expr, const char *text) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE L4_NORETURN
void l4_assert_abort(const char *text) L4_NOTHROW;


/* IMPLEMENTATION -----------------------------------------------------------*/

L4_INLINE L4_NORETURN
void l4_assert_abort(const char *text) L4_NOTHROW
{
  l4_vcon_write(L4_BASE_LOG_CAP, text, __builtin_strlen(text));
  for (;;)
    l4_thread_ex_regs(L4_INVALID_CAP, ~0UL, ~0UL,
                      L4_THREAD_EX_REGS_TRIGGER_EXCEPTION);
}

L4_ALWAYS_INLINE
void l4_assert_fn(bool expr, const char *text) L4_NOTHROW
{
  if (L4_LIKELY(expr))
    return;

  l4_assert_abort(text);
}

#endif /* NDEBUG */
