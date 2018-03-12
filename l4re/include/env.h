/**
 * \file
 * \brief   Environment interface
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
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

#include <l4/sys/consts.h>
#include <l4/sys/types.h>
#include <l4/sys/kip.h>

#include <l4/re/consts.h>

/**
 * \brief Entry in the L4Re environment array for the named inital objects.
 * \ingroup api_l4re_env
 */
typedef struct l4re_env_cap_entry_t
{
  /**
   * \brief The capability selector for the obeject.
   */
  l4_cap_idx_t cap;

  /**
   * \brief Some flags for the object.
   * \note Currently unused.
   */
  l4_umword_t flags;

  /**
   * \brief The name of the object.
   */
  char name[16];
#ifdef __cplusplus

  /**
   * \brief Create an invalid entry.
   */
  l4re_env_cap_entry_t() : cap(L4_INVALID_CAP), flags(~0) {}

  /**
   * \brief Create an entry with the name \a n, capability \a c, and
   *        flags \a f.
   *
   * \param n is the name of the initial object.
   * \param c is the capability selector that refers the initial object.
   * \param f are the additional flags for the object.
   */
  l4re_env_cap_entry_t(char const *n, l4_cap_idx_t c, l4_umword_t f = 0)
  : cap(c), flags(f)
  {
    for (unsigned i = 0; n && i < sizeof(name); ++i, ++n)
      {
        name[i] = *n;
	if (!*n)
	  break;
      }
  }

  static bool is_valid_name(char const *n)
  {
    for (unsigned i = 0; *n; ++i, ++n)
      if (i > sizeof(name))
        return false;

    return true;
  }
#endif
} l4re_env_cap_entry_t;


/**
 * \brief Initial Environment structure (C version)
 * \ingroup api_l4re_env
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
  l4_cap_idx_t first_free_cap; /**< First capability index available to the application */
  l4_fpage_t utcb_area;        /**< UTCB area of the task */
  l4_addr_t first_free_utcb;   /**< First UTCB within the UTCB area available to the application */
  l4re_env_cap_entry_t *caps;
} l4re_env_t;

/**
 * \internal
 * \brief Pointer to L4Re initial environment (C version).
 * \ingroup api_l4re_env
 */
extern l4re_env_t *l4re_global_env;


/**
 * \brief Get L4Re initial environment (C version).
 * \ingroup api_l4re_env
 * \return Pointer to L4Re initial environment (C version).
 */
L4_INLINE l4re_env_t *l4re_env(void) L4_NOTHROW;

/*
 * FIXME: this seems to be at the wrong place here
 */
/**
 * \brief Get Kernel Info Page.
 * \ingroup api_l4re_env
 * \return Pointer to Kernel Info Page (KIP) structure.
 */
L4_INLINE l4_kernel_info_t *l4re_kip(void) L4_NOTHROW;


/**
 * \brief Get the capability selector for the object named \a name.
 * \ingroup api_l4re_env
 * \param name is the name of the object to lookup in the initial objects.
 * \return A valid capability selector if the object exists or an invalid
 *         capability selector if not (l4_is_invalid_cap()).
 */
L4_INLINE l4_cap_idx_t
l4re_env_get_cap(char const *name) L4_NOTHROW;

EXTERN_C l4_cap_idx_t
l4re_get_env_cap(char const *name) L4_NOTHROW
L4_DEPRECATED("Please use l4re_env_get_cap() now.");

/**
 * \brief Get the capability selector for the object named \a name.
 * \ingroup api_l4re_env
 * \param name is the name of the object to lookup in the initial objects.
 * \param e is the environment structure to use for the operation.
 * \return A valid capability selector if the object exists or an invalid
 *         capability selector if not (l4_is_invalid_cap()).
 */
L4_INLINE l4_cap_idx_t
l4re_env_get_cap_e(char const *name, l4re_env_t const *e) L4_NOTHROW;

EXTERN_C l4_cap_idx_t
l4re_get_env_cap_e(char const *name, l4re_env_t const *e) L4_NOTHROW
L4_DEPRECATED("Please use l4re_env_get_cap_e() now.");

/**
 * \brief Get the full l4re_env_cap_entry_t for the object named \a name.
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

EXTERN_C l4re_env_cap_entry_t const *
l4re_get_env_cap_l(char const *name, unsigned l, l4re_env_t const *e) L4_NOTHROW
L4_DEPRECATED("Please use l4re_env_get_cap_l() now.");


L4_INLINE
l4re_env_t *l4re_env() L4_NOTHROW
{ return l4re_global_env; }

L4_INLINE
l4_kernel_info_t *l4re_kip() L4_NOTHROW
{
  extern char __L4_KIP_ADDR__[1];
  return (l4_kernel_info_t *)__L4_KIP_ADDR__;
}

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

