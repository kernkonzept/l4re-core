/**
 * \file
 * Environment interface
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/consts.h>
#include <l4/sys/types.h>
#include <l4/sys/kip.h>
#include <l4/sys/compiler.h>

#include <l4/re/consts.h>

/**
 * \defgroup api_l4re_env Initial Environment
 * \ingroup api_l4re_c
 *
 * C interface of the initial environment that is provided to an L4 task.
 *
 * \includefile{l4/re/env.h}
 *
 * For an explanation of the default task capabilites see \ref
 * l4_default_caps_t.
 *
 * For the C++ interface refer to L4Re::Env.
 */

/**
 * Entry in the L4Re environment array for the named inital objects.
 * \ingroup api_l4re_env
 */
typedef struct l4re_env_cap_entry_t
{
  /**
   * The capability selector for the object.
   */
  l4_cap_idx_t cap;

  /**
   * Flags for the object.
   * \note Currently unused, except as an end marker for the entry list.
   */
  l4_umword_t flags;

  /**
   * The name of the object.
   */
  char name[16];
#ifdef __cplusplus

  /**
   * Create an invalid entry.
   */
  l4re_env_cap_entry_t() L4_NOTHROW : cap(L4_INVALID_CAP), flags(~0UL) {}

  /**
   * Create an entry with the name \a n, capability \a c, and flags \a f.
   *
   * \param n is the name of the initial object.
   * \param c is the capability selector that refers the initial object.
   * \param f are the additional flags for the object.
   */
  l4re_env_cap_entry_t(char const *n, l4_cap_idx_t c, l4_umword_t f = 0) L4_NOTHROW
  : cap(c), flags(f)
  {
    for (unsigned i = 0; n && i < sizeof(name); ++i, ++n)
      {
        name[i] = *n;
	if (!*n)
	  break;
      }
  }

  static bool is_valid_name(char const *n) L4_NOTHROW
  {
    for (unsigned i = 0; *n; ++i, ++n)
      if (i > sizeof(name))
        return false;

    return true;
  }
#endif
} l4re_env_cap_entry_t;


/**
 * Initial environment data structure
 *
 * \see \link api_l4re_env Initial environment \endlink
 */
typedef struct l4re_env_t
{
  l4_cap_idx_t parent;         /**< Parent object-capability */
  l4_cap_idx_t rm;             /**< Region map object-capability */
  l4_cap_idx_t mem_alloc;      /**< Memory allocator object-capability */
  l4_cap_idx_t log;            /**< Logging object-capability */
  l4_cap_idx_t main_thread;    /**< Object-capability of the first user thread */
  l4_cap_idx_t factory;        /**< Object-capability of the factory available to the task */
  l4_cap_idx_t scheduler;      /**< Object capability for the scheduler set to use */
  l4_cap_idx_t itas;           /**< ITAS services object-capability */
  l4_cap_idx_t dbg_events;     /**< Object-capability of the debug events service */
  l4_cap_idx_t first_free_cap; /**< First capability index available to the application */
  l4_fpage_t utcb_area;        /**< UTCB area of the task */
  l4_addr_t first_free_utcb;   /**< First UTCB within the UTCB area available to the application */
  /**
   * Pointer to the first entry in the initial objects array which contains
   * #l4re_env_cap_entry_t elements. The array is terminated by an invalid
   * entry with a `flags` value of `~0ul`.
   */
  l4re_env_cap_entry_t *caps;
} l4re_env_t;

/**
 * \internal
 * Pointer to L4Re initial environment.
 * \ingroup api_l4re_env
 */
extern l4re_env_t *l4re_global_env;


/**
 * Get L4Re initial environment.
 * \ingroup api_l4re_env
 * \return Pointer to L4Re initial environment.
 */
L4_INLINE l4re_env_t *l4re_env(void) L4_NOTHROW;

/*
 * FIXME: this seems to be at the wrong place here
 */
/**
 * Get Kernel Info Page.
 * \ingroup api_l4re_env
 * \return Pointer to Kernel Info Page (KIP) structure.
 */
L4_INLINE l4_kernel_info_t const *l4re_kip(void) L4_NOTHROW;


/**
 * Get the capability selector for the object named \a name.
 * \ingroup api_l4re_env
 * \param name is the name of the object to lookup in the initial objects.
 * \return A valid capability selector if the object exists or an invalid
 *         capability selector if not (l4_is_invalid_cap()).
 */
L4_INLINE l4_cap_idx_t
l4re_env_get_cap(char const *name) L4_NOTHROW;

/**
 * Get the capability selector for the object named \a name.
 * \ingroup api_l4re_env
 * \param name is the name of the object to lookup in the initial objects.
 * \param e is the environment structure to use for the operation.
 * \return A valid capability selector if the object exists or an invalid
 *         capability selector if not (l4_is_invalid_cap()).
 */
L4_INLINE l4_cap_idx_t
l4re_env_get_cap_e(char const *name, l4re_env_t const *e) L4_NOTHROW;

/**
 * Get the full l4re_env_cap_entry_t for the object named \a name.
 * \ingroup api_l4re_env
 * \param name is the name of the object to lookup in the initial objects.
 * \param l is the length of the name string, thus \a name might not be zero
 *          terminated.
 * \param e is the environment structure to use for the operation.
 * \return A pointer to an l4re_env_cap_entry_t if the object exists or
 *         NULL if not.
 */
L4_INLINE l4re_env_cap_entry_t const *
l4re_env_get_cap_l(char const *name, unsigned l, l4re_env_t const *e) L4_NOTHROW;

L4_INLINE
l4re_env_t *l4re_env(void) L4_NOTHROW
{ return l4re_global_env; }

L4_INLINE
l4_kernel_info_t const *l4re_kip(void) L4_NOTHROW
{ return l4_kip(); }

L4_INLINE l4re_env_cap_entry_t const *
l4re_env_get_cap_l(char const *name, unsigned l, l4re_env_t const *e) L4_NOTHROW
{
  l4re_env_cap_entry_t const *c = e->caps;
  for (; c && c->flags != ~0UL; ++c)
    {
      unsigned i;
      for (i = 0;
           i < sizeof(c->name) && i < l && c->name[i] && name[i] && name[i] == c->name[i];
           ++i)
	;

      if (i == l && (i == sizeof(c->name) || !c->name[i]))
	return c;
    }
  return NULL;
}

L4_INLINE l4_cap_idx_t
l4re_env_get_cap_e(char const *name, l4re_env_t const *e) L4_NOTHROW
{
  unsigned l;
  l4re_env_cap_entry_t const *r;
  for (l = 0; name[l]; ++l) ;
  r = l4re_env_get_cap_l(name, l, e);
  if (r)
    return r->cap;

  return L4_INVALID_CAP;
}

L4_INLINE l4_cap_idx_t
l4re_env_get_cap(char const *name) L4_NOTHROW
{ return l4re_env_get_cap_e(name, l4re_env()); }

