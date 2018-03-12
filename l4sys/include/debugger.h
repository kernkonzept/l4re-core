#pragma once
/**
 * \file
 * Debugger related definitions.
 * \ingroup l4_api
 */
/*
 * (c) 2008-2011 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
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

#include <l4/sys/compiler.h>
#include <l4/sys/utcb.h>
#include <l4/sys/ipc.h>

/**
 * \defgroup l4_debugger_api Kernel Debugger
 * \ingroup api_calls_fiasco
 * Kernel debugger related functionality.
 * \attention This API is subject to change!
 *
 * This is a debugging facility, any call to any function might be invalid.
 * Do not rely on it in any real code.
 *
 * \includefile{l4/sys/debugger.h}
 *
 */

/**
 * Set the name of a kernel object.
 * \ingroup l4_debugger_api
 *
 * \param cap     Capability which refers to the kernel object.
 * \param name    Name of the kernel object that is e.g. displayed in the
 *                kernel debugger.
 *
 * This is a debugging facility, the call might be invalid.
 */
L4_INLINE l4_msgtag_t
l4_debugger_set_object_name(l4_cap_idx_t cap, const char *name) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_debugger_set_object_name_u(l4_cap_idx_t cap, const char *name, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Get name of the kernel object with Id `id`.
 *
 * \param      cap   Capability of the debugger object.
 * \param      id    Global id of the object whose name is asked.
 * \param[out] name  Buffer to copy the name into. The buffer must be
 *                   allocated by the caller.
 * \param      size  Length of the `name` buffer.
 *
 * \return Syscall return tag
 */
L4_INLINE l4_msgtag_t
l4_debugger_get_object_name(l4_cap_idx_t cap, unsigned id,
                            char *name, unsigned size) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_debugger_get_object_name_u(l4_cap_idx_t cap, unsigned id,
                              char *name, unsigned size,
                              l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Get the globally unique ID of the object behind a capability.
 * \ingroup l4_debugger_api
 *
 * \param cap    Capability
 *
 * \retval ~0UL  Capability is not valid.
 * \retval >=0   Global debugger id.
 *
 * This is a debugging facility, the call might be invalid.
 */
L4_INLINE unsigned long
l4_debugger_global_id(l4_cap_idx_t cap) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE unsigned long
l4_debugger_global_id_u(l4_cap_idx_t cap, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Get the globally unique ID of the object behind the kobject pointer.
 * \ingroup l4_debugger_api
 *
 * \param cap    Capability
 * \param kobjp  Kobject pointer
 *
 * \retval ~0UL  The capability or the kobject pointer are invalid.
 * \retval >=0   The globally unique id.
 *
 * This is a debugging facility, the call might be invalid.
 */
L4_INLINE unsigned long
l4_debugger_kobj_to_id(l4_cap_idx_t cap, l4_addr_t kobjp) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE unsigned long
l4_debugger_kobj_to_id_u(l4_cap_idx_t cap, l4_addr_t kobjp, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Query the log-id for a log type
 *
 * \param cap    Debugger capability
 * \param name   Name to query for.
 * \param idx    Idx to start searching, start with 0
 *
 * \return positive ID, or negative error code
 *
 * This is a debugging facility, the call might be invalid.
 */
L4_INLINE int
l4_debugger_query_log_typeid(l4_cap_idx_t cap, const char *name,
                             unsigned idx) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE int
l4_debugger_query_log_typeid_u(l4_cap_idx_t cap, const char *name,
                               unsigned idx, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Query the name of a log type given the ID
 *
 * \param cap           Debugger capability.
 * \param idx           ID to query.
 * \param name          Buffer to copy name to.
 * \param namelen       Buffer length of name.
 * \param shortname     Buffer to copy `shortname` to.
 * \param shortnamelen  Buffer length of `shortname`.
 *
 * \retval 0   Success
 * \retval <0  Error
 *
 * This is a debugging facility, the call might be invalid.
 */
L4_INLINE int
l4_debugger_query_log_name(l4_cap_idx_t cap, unsigned idx,
                           char *name, unsigned namelen,
                           char *shortname, unsigned shortnamelen) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE int
l4_debugger_query_log_name_u(l4_cap_idx_t cap, unsigned idx,
                             char *name, unsigned namelen,
                             char *shortname, unsigned shortnamelen,
                             l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Set or unset log.
 *
 * \param cap     Debugger object.
 * \param name    Name of the log type.
 * \param on_off  1: turn log on, 0: turn log off
 *
 * \return Syscall return tag
 */
L4_INLINE l4_msgtag_t
l4_debugger_switch_log(l4_cap_idx_t cap, const char *name,
                       int on_off) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_debugger_switch_log_u(l4_cap_idx_t cap, const char *name, int on_off,
                         l4_utcb_t *utcb) L4_NOTHROW;

enum
{
  L4_DEBUGGER_NAME_SET_OP         = 0UL,
  L4_DEBUGGER_GLOBAL_ID_OP        = 1UL,
  L4_DEBUGGER_KOBJ_TO_ID_OP       = 2UL,
  L4_DEBUGGER_QUERY_LOG_TYPEID_OP = 3UL,
  L4_DEBUGGER_SWITCH_LOG_OP       = 4UL,
  L4_DEBUGGER_NAME_GET_OP         = 5UL,
  L4_DEBUGGER_QUERY_LOG_NAME_OP   = 6UL,
};

enum
{
  L4_DEBUGGER_SWITCH_LOG_ON  = 1,
  L4_DEBUGGER_SWITCH_LOG_OFF = 0,
};

/* IMPLEMENTATION -----------------------------------------------------------*/

#include <l4/sys/kernel_object.h>

L4_INLINE l4_msgtag_t
l4_debugger_set_object_name_u(unsigned long cap,
                              const char *name, l4_utcb_t *utcb) L4_NOTHROW
{
  unsigned int i;
  char *s = (char *)&l4_utcb_mr_u(utcb)->mr[1];
  l4_utcb_mr_u(utcb)->mr[0] = L4_DEBUGGER_NAME_SET_OP;
  for (i = 0;
       *name && i < (L4_UTCB_GENERIC_DATA_SIZE - 2) * sizeof(l4_umword_t) - 1;
       ++i, ++name, ++s)
    *s = *name;
  *s = 0;
  i = (i + sizeof(l4_umword_t) - 1) / sizeof(l4_umword_t);
  return l4_invoke_debugger(cap, l4_msgtag(0, i + 1, 0, 0), utcb);
}

L4_INLINE unsigned long
l4_debugger_global_id_u(l4_cap_idx_t cap, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_utcb_mr_u(utcb)->mr[0] = L4_DEBUGGER_GLOBAL_ID_OP;
  if (l4_error_u(l4_invoke_debugger(cap, l4_msgtag(0, 1, 0, 0), utcb), utcb))
    return ~0UL;
  return l4_utcb_mr_u(utcb)->mr[0];
}

L4_INLINE unsigned long
l4_debugger_kobj_to_id_u(l4_cap_idx_t cap, l4_addr_t kobjp, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_utcb_mr_u(utcb)->mr[0] = L4_DEBUGGER_KOBJ_TO_ID_OP;
  l4_utcb_mr_u(utcb)->mr[1] = kobjp;
  if (l4_error_u(l4_invoke_debugger(cap, l4_msgtag(0, 2, 0, 0), utcb), utcb))
    return ~0UL;
  return l4_utcb_mr_u(utcb)->mr[0];
}

L4_INLINE int
l4_debugger_query_log_typeid_u(l4_cap_idx_t cap, const char *name,
                               unsigned idx,
                               l4_utcb_t *utcb) L4_NOTHROW
{
  unsigned l;
  int e;
  l4_utcb_mr_u(utcb)->mr[0] = L4_DEBUGGER_QUERY_LOG_TYPEID_OP;
  l4_utcb_mr_u(utcb)->mr[1] = idx;
  l = __builtin_strlen(name);
  l = l > 31 ? 31 : l;
  __builtin_strncpy((char *)&l4_utcb_mr_u(utcb)->mr[2], name, 31);
  l = (l + 1 + sizeof(l4_umword_t) - 1) / sizeof(l4_umword_t);
  e = l4_error_u(l4_invoke_debugger(cap, l4_msgtag(0, 2 + l, 0, 0), utcb), utcb);
  if (e < 0)
    return e;
  return l4_utcb_mr_u(utcb)->mr[0];
}

L4_INLINE int
l4_debugger_query_log_name_u(l4_cap_idx_t cap, unsigned idx,
                             char *name, unsigned namelen,
                             char *shortname, unsigned shortnamelen,
                             l4_utcb_t *utcb) L4_NOTHROW
{
  int e;
  char *n;
  l4_utcb_mr_u(utcb)->mr[0] = L4_DEBUGGER_QUERY_LOG_NAME_OP;
  l4_utcb_mr_u(utcb)->mr[1] = idx;
  e = l4_error_u(l4_invoke_debugger(cap, l4_msgtag(0, 2, 0, 0), utcb), utcb);
  if (e < 0)
    return e;
  n = (char *)&l4_utcb_mr_u(utcb)->mr[0];
  __builtin_strncpy(name, n, namelen);
  name[namelen - 1] = 0;
  __builtin_strncpy(shortname, n + __builtin_strlen(n) + 1, shortnamelen);
  shortname[shortnamelen - 1] = 0;
  return 0;
}


L4_INLINE l4_msgtag_t
l4_debugger_switch_log_u(l4_cap_idx_t cap, const char *name, int on_off,
                         l4_utcb_t *utcb) L4_NOTHROW
{
  unsigned l;
  l4_utcb_mr_u(utcb)->mr[0] = L4_DEBUGGER_SWITCH_LOG_OP;
  l4_utcb_mr_u(utcb)->mr[1] = on_off;
  l = __builtin_strlen(name);
  l = l > 31 ? 31 : l;
  __builtin_strncpy((char *)&l4_utcb_mr_u(utcb)->mr[2], name, 31);
  l = (l + 1 + sizeof(l4_umword_t) - 1) / sizeof(l4_umword_t);
  return l4_invoke_debugger(cap, l4_msgtag(0, 2 + l, 0, 0), utcb);
}

L4_INLINE l4_msgtag_t
l4_debugger_get_object_name_u(l4_cap_idx_t cap, unsigned id,
                              char *name, unsigned size,
                              l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msgtag_t t;
  l4_utcb_mr_u(utcb)->mr[0] = L4_DEBUGGER_NAME_GET_OP;
  l4_utcb_mr_u(utcb)->mr[1] = id;
  t = l4_invoke_debugger(cap, l4_msgtag(0, 2, 0, 0), utcb);
  __builtin_strncpy(name, (char *)&l4_utcb_mr_u(utcb)->mr[0], size);
  name[size - 1] = 0;
  return t;
}


L4_INLINE l4_msgtag_t
l4_debugger_set_object_name(unsigned long cap,
                            const char *name) L4_NOTHROW
{
  return l4_debugger_set_object_name_u(cap, name, l4_utcb());
}

L4_INLINE unsigned long
l4_debugger_global_id(l4_cap_idx_t cap) L4_NOTHROW
{
  return l4_debugger_global_id_u(cap, l4_utcb());
}

L4_INLINE unsigned long
l4_debugger_kobj_to_id(l4_cap_idx_t cap, l4_addr_t kobjp) L4_NOTHROW
{
  return l4_debugger_kobj_to_id_u(cap, kobjp, l4_utcb());
}

L4_INLINE int
l4_debugger_query_log_typeid(l4_cap_idx_t cap, const char *name,
                             unsigned idx) L4_NOTHROW
{
  return l4_debugger_query_log_typeid_u(cap, name, idx, l4_utcb());
}

L4_INLINE int
l4_debugger_query_log_name(l4_cap_idx_t cap, unsigned idx,
                           char *name, unsigned namelen,
                           char *shortname, unsigned shortnamelen) L4_NOTHROW
{
  return l4_debugger_query_log_name_u(cap, idx, name, namelen,
                                      shortname, shortnamelen, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_debugger_switch_log(l4_cap_idx_t cap, const char *name,
                       int on_off) L4_NOTHROW
{
  return l4_debugger_switch_log_u(cap, name, on_off, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_debugger_get_object_name(l4_cap_idx_t cap, unsigned id,
                            char *name, unsigned size) L4_NOTHROW
{
  return l4_debugger_get_object_name_u(cap, id, name, size, l4_utcb());
}
