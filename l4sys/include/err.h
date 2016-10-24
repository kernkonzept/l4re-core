/**
 * \file
 * Error codes.
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

#include <l4/sys/compiler.h>

/**
 * \defgroup l4_error_api Error codes
 * Common error codes.
 * \ingroup l4_api
 *
 * \includefile{l4/sys/err.h}
 */

/**
 * L4 error codes.
 * \ingroup l4_error_api
 *
 * Those error codes are used by both the kernel and the user programs.
 */
enum l4_error_code_t
{
  L4_EOK           =  0,     /**< Ok. */
  L4_EPERM         =  1,     /**< No permission. */
  L4_ENOENT        =  2,     /**< No such entity. */
  L4_EIO           =  5,     /**< I/O error. */
  L4_ENXIO         =  6,     /**< No such device or address */
  L4_E2BIG         =  7,     /**< Argument value too big */
  L4_EAGAIN        = 11,     /**< Try again. */
  L4_ENOMEM        = 12,     /**< No memory. */
  L4_EACCESS       = 13,     /**< Permission denied. */
  L4_EFAULT        = 14,     /**< Invalid memory address. */
  L4_EBUSY         = 16,     /**< Object currently busy, try later. */
  L4_EEXIST        = 17,     /**< Already exists. */
  L4_ENODEV        = 19,     /**< No such thing. */
  L4_EINVAL        = 22,     /**< Invalid argument. */
  L4_ENOSPC        = 28,     /**< No space left on device */
  L4_ERANGE        = 34,     /**< Range error. */
  L4_ENAMETOOLONG  = 36,     /**< Name too long. */
  L4_ENOSYS        = 38,     /**< No sys. */
  L4_EBADPROTO     = 39,     /**< Unsupported protocol. */
  L4_EADDRNOTAVAIL = 99,     /**< Address not available. */
  L4_ERRNOMAX      = 100,    /**< Maximum error value. */

  L4_ENOREPLY      = 1000,   /**< No reply. */
  L4_EMSGTOOSHORT  = 1001,   /**< Message too short. */
  L4_EMSGTOOLONG   = 1002,   /**< Message too long. */
  L4_EMSGMISSARG   = 1003,   /**< Message has invalid capability. */

  L4_EIPC_LO       = 2000,   /**< Communication error-range low. */
  L4_EIPC_HI       = 2000 + 0x1f,   /**< Communication error-range high. */
};

__BEGIN_DECLS
L4_CV char const *l4sys_errtostr(long err) L4_NOTHROW;
__END_DECLS


