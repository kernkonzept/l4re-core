/**
 * \file
 * \brief Log C interface.
 */
/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
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

/**
 * \defgroup api_l4re_c_log Log interface
 * \ingroup api_l4re_c
 * \brief Log C interface.
 */

#include <l4/re/env.h>

EXTERN_C_BEGIN

/**
 * \ingroup api_l4re_c_log
 * \brief Write a null terminated string to the default log.
 *
 * \param string     Text to print, null terminated.
 *
 * \return 0 for success, <0 on error
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
 * \return 0 for success, <0 on error
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
 * \return 0 for success, <0 on error
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
 * \return 0 for success, <0 on error
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

EXTERN_C_END
