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
 * License: see LICENSE.spdx (in this directory or the directories above)
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
 * \ingroup l4_debugger_api
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
 * \retval ~0UL       Capability is not valid.
 * \retval otherwise  Global debugger id.
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
 * \retval ~0UL       The capability or the kobject pointer are invalid.
 * \retval otherwise  The globally unique id.
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
 * \ingroup l4_debugger_api
 *
 * \param cap    Debugger capability
 * \param name   Name to query for.
 * \param idx    Idx to start searching, start with 0
 *
 * \return positive ID, or negative error code
 *
 * This is a debugging facility, the call might be invalid.
 */
L4_INLINE long
l4_debugger_query_log_typeid(l4_cap_idx_t cap, const char *name,
                             unsigned idx) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE long
l4_debugger_query_log_typeid_u(l4_cap_idx_t cap, const char *name,
                               unsigned idx, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Query the name of a log type given the ID
 * \ingroup l4_debugger_api
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
L4_INLINE long
l4_debugger_query_log_name(l4_cap_idx_t cap, unsigned idx,
                           char *name, unsigned namelen,
                           char *shortname, unsigned shortnamelen) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE long
l4_debugger_query_log_name_u(l4_cap_idx_t cap, unsigned idx,
                             char *name, unsigned namelen,
                             char *shortname, unsigned shortnamelen,
                             l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Set or unset log.
 * \ingroup l4_debugger_api
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

/**
 * Add loaded image information for a task.
 * \ingroup l4_debugger_api
 *
 * \param cap     Capability which refers to the task object.
 * \param base    Load base address of image.
 * \param name    Image base name.
 *
 * This is a debugging facility, the call might be invalid.
 */
L4_INLINE l4_msgtag_t
l4_debugger_add_image_info(l4_cap_idx_t cap, l4_addr_t base,
                           const char *name) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_debugger_add_image_info_u(l4_cap_idx_t cap, l4_addr_t base, const char *name,
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
  L4_DEBUGGER_ADD_IMAGE_INFO_OP   = 7UL,
};

enum
{
  L4_DEBUGGER_SWITCH_LOG_ON  = 1,
  L4_DEBUGGER_SWITCH_LOG_OFF = 0,
};

/* IMPLEMENTATION -----------------------------------------------------------*/

#include <l4/sys/kernel_object.h>

/**
 * \internal
 * Copy a number of characters from the C string \b src to the C string \b dst.
 *
 * The resulting string \b dst is always '\0'-terminated unless \b maxlen is 0.
 * If the C string in \b src is shorter than the buffer \b dst then the
 * remaining bytes in \b dst are NOT initialized.
 *
 * \param dst     Target buffer.
 * \param src     Source buffer.
 * \param maxlen  Maximum number of bytes written to the target buffer.
 * \return The number of bytes written (including the terminating '\0'.
 */
L4_INLINE unsigned
__strcpy_maxlen(char *dst, char const *src, unsigned maxlen)
{
  unsigned i;
  if (!maxlen)
    return 0;

  for (i = 0; i < maxlen - 1 && src[i]; ++i)
    dst[i] = src[i];
  dst[i] = '\0';

  return i + 1;
}

L4_INLINE l4_msgtag_t
l4_debugger_set_object_name_u(l4_cap_idx_t cap,
                              const char *name, l4_utcb_t *utcb) L4_NOTHROW
{
  unsigned i;
  l4_utcb_mr_u(utcb)->mr[0] = L4_DEBUGGER_NAME_SET_OP;
  i = __strcpy_maxlen((char *)&l4_utcb_mr_u(utcb)->mr[1], name,
                      (L4_UTCB_GENERIC_DATA_SIZE - 2) * sizeof(l4_umword_t));
  i = l4_bytes_to_mwords(i);
  return l4_invoke_debugger(cap, l4_msgtag(0, 1 + i, 0, 0), utcb);
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

L4_INLINE long
l4_debugger_query_log_typeid_u(l4_cap_idx_t cap, const char *name,
                               unsigned idx,
                               l4_utcb_t *utcb) L4_NOTHROW
{
  unsigned i;
  long e;
  l4_utcb_mr_u(utcb)->mr[0] = L4_DEBUGGER_QUERY_LOG_TYPEID_OP;
  l4_utcb_mr_u(utcb)->mr[1] = idx;
  i = __strcpy_maxlen((char *)&l4_utcb_mr_u(utcb)->mr[2], name, 32);
  i = l4_bytes_to_mwords(i);
  e = l4_error_u(l4_invoke_debugger(cap, l4_msgtag(0, 2 + i, 0, 0), utcb), utcb);
  if (e < 0)
    return e;
  return l4_utcb_mr_u(utcb)->mr[0];
}

L4_INLINE long
l4_debugger_query_log_name_u(l4_cap_idx_t cap, unsigned idx,
                             char *name, unsigned namelen,
                             char *shortname, unsigned shortnamelen,
                             l4_utcb_t *utcb) L4_NOTHROW
{
  long e;
  char const *n;
  l4_utcb_mr_u(utcb)->mr[0] = L4_DEBUGGER_QUERY_LOG_NAME_OP;
  l4_utcb_mr_u(utcb)->mr[1] = idx;
  e = l4_error_u(l4_invoke_debugger(cap, l4_msgtag(0, 2, 0, 0), utcb), utcb);
  if (e < 0)
    return e;
  n = (char const *)&l4_utcb_mr_u(utcb)->mr[0];
  __strcpy_maxlen(name, n, namelen);
  __strcpy_maxlen(shortname, n + __builtin_strlen(n) + 1, shortnamelen);
  return 0;
}


L4_INLINE l4_msgtag_t
l4_debugger_switch_log_u(l4_cap_idx_t cap, const char *name, int on_off,
                         l4_utcb_t *utcb) L4_NOTHROW
{
  unsigned i;
  l4_utcb_mr_u(utcb)->mr[0] = L4_DEBUGGER_SWITCH_LOG_OP;
  l4_utcb_mr_u(utcb)->mr[1] = on_off;
  i = __strcpy_maxlen((char *)&l4_utcb_mr_u(utcb)->mr[2], name, 32);
  i = l4_bytes_to_mwords(i);
  return l4_invoke_debugger(cap, l4_msgtag(0, 2 + i, 0, 0), utcb);
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
  __strcpy_maxlen(name, (char const *)&l4_utcb_mr_u(utcb)->mr[0], size);
  return t;
}

L4_INLINE l4_msgtag_t
l4_debugger_add_image_info_u(l4_cap_idx_t cap, l4_addr_t base,
                             const char *name, l4_utcb_t *utcb) L4_NOTHROW
{
  unsigned i;
  l4_utcb_mr_u(utcb)->mr[0] = L4_DEBUGGER_ADD_IMAGE_INFO_OP;
  l4_utcb_mr_u(utcb)->mr[1] = base;
  i = __strcpy_maxlen((char *)&l4_utcb_mr_u(utcb)->mr[2], name,
                      (L4_UTCB_GENERIC_DATA_SIZE - 3) * sizeof(l4_umword_t));
  i = l4_bytes_to_mwords(i);
  return l4_invoke_debugger(cap, l4_msgtag(0, 2 + i, 0, 0), utcb);
}


L4_INLINE l4_msgtag_t
l4_debugger_set_object_name(l4_cap_idx_t cap,
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

L4_INLINE long
l4_debugger_query_log_typeid(l4_cap_idx_t cap, const char *name,
                             unsigned idx) L4_NOTHROW
{
  return l4_debugger_query_log_typeid_u(cap, name, idx, l4_utcb());
}

L4_INLINE long
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

L4_INLINE l4_msgtag_t
l4_debugger_add_image_info(l4_cap_idx_t cap, l4_addr_t base,
                           const char *name) L4_NOTHROW
{
  return l4_debugger_add_image_info_u(cap, base, name, l4_utcb());
}
