/**
 * \file
 * Namespace functions, C interface
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

/**
 * \defgroup api_l4re_c_ns Namespace interface
 * \ingroup api_l4re_c
 * Namespace C interface.
 */

#include <l4/re/env.h>
#include <l4/sys/compiler.h>

/**
 * Namespace register flags.
 * \ingroup api_l4re_c_ns
 *
 * \see L4Re::Namespace::Register_flags
 */
enum l4re_ns_register_flags {
  L4RE_NS_REGISTER_RO  = L4_FPAGE_RO,
  L4RE_NS_REGISTER_DIR = 0x10,
  L4RE_NS_REGISTER_RW  = L4_FPAGE_RX,
  L4RE_NS_REGISTER_RWS = L4_FPAGE_RWX,
  L4RE_NS_REGISTER_S   = L4_FPAGE_W,
};

L4_BEGIN_DECLS

/**
 * Namespace type
 * \ingroup api_l4re_c_ns
 */
typedef l4_cap_idx_t l4re_namespace_t;



/**
 * \ingroup api_l4re_c_ns
 * \copybrief l4re_ns_query_srv()
 * \param timeout  Timeout of query in milliseconds. The client will only wait
 *                 if a name already has been registered with the server but no
 *                 object has been attached yet.
 * \copydetails l4re_ns_query_srv()
 */
L4_CV long
l4re_ns_query_to_srv(l4re_namespace_t srv, char const *name,
                     l4_cap_idx_t const cap, int timeout) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_ns
 * Query the name space for the object named by `name`.
 *
 * \param srv   Name space server to use for the query.
 * \param name  String to query.
 * \param cap   Capability slot where the received capability will be stored.
 *
 * \retval 0           Name could be fully resolved.
 * \retval >0          Name could only be partly resolved. The number of
 *                     remaining characters is returned.
 * \retval -L4_ENOENT  Entry could not be found.
 * \retval -L4_EAGAIN  Entry exists but no object is yet attached.
 *                     Try again later.
 * \retval <0          IPC errors, see #l4_error_code_t.
 */
L4_CV L4_INLINE long
l4re_ns_query_srv(l4re_namespace_t srv, char const *name,
                  l4_cap_idx_t const cap) L4_NOTHROW;

/**
 *
 * \ingroup api_l4re_c_ns
 * \copybrief L4Re::Namespace::register_obj
 * \param srv  Name space server to use for the query.
 * \copydetails L4Re::Namespace::register_obj
 */
L4_CV long
l4re_ns_register_obj_srv(l4re_namespace_t srv, char const *name,
                         l4_cap_idx_t const obj, unsigned flags) L4_NOTHROW;



/****** Implementation ***********/

L4_CV L4_INLINE long
l4re_ns_query_srv(l4re_namespace_t srv, char const *name,
                  l4_cap_idx_t const cap) L4_NOTHROW
{
  return l4re_ns_query_to_srv(srv, name, cap, 40000);
}


L4_END_DECLS
