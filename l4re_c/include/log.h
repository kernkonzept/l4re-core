/**
 * \file
 * \brief Log C interface.
 */
/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

/**
 * \defgroup api_l4re_c_log Log interface
 * \ingroup api_l4re_c
 * \brief Log C interface.
 */

#include <l4/re/env.h>

__BEGIN_DECLS

/**
 * \ingroup api_l4re_c_log
 * \brief Write a null terminated string to the default log.
 *
 * \param string     Text to print, null terminated.
 *
 * \see L4Re::Log::print
 */
L4_CV L4_INLINE void
l4re_log_print(char const *string) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_log
 * \brief Write a string of a given length to the default log.
 *
 * \param string     Text to print, null terminated.
 * \param len        Length of string in bytes.
 *
 * \see L4Re::Log::printn
 */
L4_CV L4_INLINE void
l4re_log_printn(char const *string, int len) L4_NOTHROW;




/**
 * \ingroup api_l4re_c_log
 * \brief Write a null terminated string to a log.
 *
 * \param logcap     Log capability (service).
 * \param string     Text to print, null terminated.
 *
 * \see L4Re::Log::print
 */
L4_CV void
l4re_log_print_srv(const l4_cap_idx_t logcap,
                   char const *string) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_log
 * \brief Write a string of a given length to a log.
 *
 * \param logcap     Log capability (service).
 * \param string     Text to print, null terminated.
 * \param len        Length of string in bytes.
 *
 * \see L4Re::Log::printn
 */
L4_CV void
l4re_log_printn_srv(const l4_cap_idx_t logcap,
                    char const *string, int len) L4_NOTHROW;


/********** Implementations ***************************/

L4_CV L4_INLINE void
l4re_log_print(char const *string) L4_NOTHROW
{
  l4re_log_print_srv(l4re_global_env->log, string);
}

L4_CV L4_INLINE void
l4re_log_printn(char const *string, int len) L4_NOTHROW
{
  l4re_log_printn_srv(l4re_global_env->log, string, len);
}

__END_DECLS
