/**
 * \internal
 * \file
 * Timeout definitions.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
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
#ifndef L4_SYS_TIMEOUT_H__
#define L4_SYS_TIMEOUT_H__

#include <l4/sys/l4int.h>

/**
 * \defgroup l4_timeout_api Timeouts
 * \ingroup l4_ipc_api
 * All kinds of timeouts and time related functions.
 */

/**
 * Basic timeout specification.
 * \ingroup l4_timeout_api
 *
 * Basically a floating point number with 10 bits mantissa and
 * 5 bits exponent (t = m*2^e).
 *
 * If bit 15 == 1 the timeout is absolute and the lower 6 bits encode the index
 * of the UTCB buffer register(s) holding the absolute 64-bit timeout value. On
 * 32-bit systems, two consecutive UTCB buffer registers are used.
 */
typedef struct l4_timeout_s {
  l4_uint16_t t;                           /**< timeout value */
} __attribute__((packed)) l4_timeout_s;


/**
 * Timeout pair.
 * \ingroup l4_timeout_api
 *
 * For IPC there are usually a send and a receive timeout.
 * So this structure contains a pair of timeouts.
 */
typedef union l4_timeout_t {
  l4_uint32_t raw;                 /**< raw value */
  struct
  {
#ifdef __BIG_ENDIAN__
    l4_timeout_s snd;              /**< send timeout */
    l4_timeout_s rcv;              /**< receive timeout */
#else
    l4_timeout_s rcv;              /**< receive timeout */
    l4_timeout_s snd;              /**< send timeout */
#endif
  } p;                             /**< combined timeout */
} l4_timeout_t;


/**
 * Timeout constants.
 * \ingroup l4_timeout_api
 */
/*@{*/
#define L4_IPC_TIMEOUT_0 ((l4_timeout_s){0x0400})           /**< 0 timeout */
#define L4_IPC_TIMEOUT_NEVER ((l4_timeout_s){0})            /**< never timeout */
#define L4_IPC_NEVER_INITIALIZER {0}                        /**< never timeout, init */
#define L4_IPC_NEVER ((l4_timeout_t){0})                    /**< never timeout */
#define L4_IPC_RECV_TIMEOUT_0 ((l4_timeout_t){0x00000400})  /**< 0 receive timeout */
#define L4_IPC_SEND_TIMEOUT_0 ((l4_timeout_t){0x04000000})  /**< 0 send timeout */
#define L4_IPC_BOTH_TIMEOUT_0 ((l4_timeout_t){0x04000400})  /**< 0 receive and send timeout */
/*@}*/

/**
 * Get relative timeout consisting of mantissa and exponent.
 * \ingroup l4_timeout_api
 *
 * \param man  Mantissa of timeout
 * \param exp  Exponent of timeout
 *
 * \return timeout value
 */
L4_INLINE
l4_timeout_s l4_timeout_rel(unsigned man, unsigned exp) L4_NOTHROW;


/**
 * Convert explicit timeout values to l4_timeout_t type.
 * \ingroup l4_timeout_api
 *
 * \param  snd_man    Mantissa of send timeout.
 * \param  snd_exp    Exponent of send timeout.
 * \param  rcv_man    Mantissa of receive timeout.
 * \param  rcv_exp    Exponent of receive timeout.
 */
L4_INLINE
l4_timeout_t l4_ipc_timeout(unsigned snd_man, unsigned snd_exp,
                            unsigned rcv_man, unsigned rcv_exp) L4_NOTHROW;

/**
 * Combine send and receive timeout in a timeout.
 * \ingroup l4_timeout_api
 *
 * \param  snd    Send timeout
 * \param  rcv    Receive timeout
 *
 * \return L4 timeout
 */
L4_INLINE
l4_timeout_t l4_timeout(l4_timeout_s snd, l4_timeout_s rcv) L4_NOTHROW;

/**
 * Set send timeout in given to timeout.
 * \ingroup l4_timeout_api
 *
 * \param  snd    Send timeout
 * \retval to     L4 timeout
 */
L4_INLINE
void l4_snd_timeout(l4_timeout_s snd, l4_timeout_t *to) L4_NOTHROW;

/**
 * Set receive timeout in given to timeout.
 * \ingroup l4_timeout_api
 *
 * \param  rcv    Receive timeout
 * \retval to     L4 timeout
 */
L4_INLINE
void l4_rcv_timeout(l4_timeout_s rcv, l4_timeout_t *to) L4_NOTHROW;

/**
 * Get clock value of out timeout.
 * \ingroup l4_timeout_api
 *
 * \param to     L4 timeout
 *
 * \return Clock value
 */
L4_INLINE
l4_kernel_clock_t l4_timeout_rel_get(l4_timeout_s to) L4_NOTHROW;


/**
 * Return whether the given timeout is absolute or not.
 * \ingroup l4_timeout_api
 *
 * \param to     L4 timeout
 *
 * \return != 0 if absolute, 0 if relative
 */
L4_INLINE
unsigned l4_timeout_is_absolute(l4_timeout_s to) L4_NOTHROW;

/**
 * Get clock value for a clock + a timeout.
 * \ingroup l4_timeout_api
 *
 * \param cur    Clock value
 * \param to     L4 timeout
 *
 * \return Clock sum
 */
L4_INLINE
l4_kernel_clock_t l4_timeout_get(l4_kernel_clock_t cur, l4_timeout_s to) L4_NOTHROW;


/*
 * Implementation
 */

L4_INLINE
l4_timeout_t l4_ipc_timeout(unsigned snd_man, unsigned snd_exp,
    unsigned rcv_man, unsigned rcv_exp) L4_NOTHROW
{
  l4_timeout_t t;
  t.p.snd.t = (snd_man & 0x3ff) | ((snd_exp << 10) & 0x7c00);
  t.p.rcv.t = (rcv_man & 0x3ff) | ((rcv_exp << 10) & 0x7c00);
  return t;
}


L4_INLINE
l4_timeout_t l4_timeout(l4_timeout_s snd, l4_timeout_s rcv) L4_NOTHROW
{
  l4_timeout_t t;
  t.p.snd = snd;
  t.p.rcv = rcv;
  return t;
}


L4_INLINE
void l4_snd_timeout(l4_timeout_s snd, l4_timeout_t *to) L4_NOTHROW
{
  to->p.snd = snd;
}


L4_INLINE
void l4_rcv_timeout(l4_timeout_s rcv, l4_timeout_t *to) L4_NOTHROW
{
  to->p.rcv = rcv;
}


L4_INLINE
l4_timeout_s l4_timeout_rel(unsigned man, unsigned exp) L4_NOTHROW
{
  return (l4_timeout_s){(l4_uint16_t)((man & 0x3ff) | ((exp << 10) & 0x7c00))};
}


L4_INLINE
l4_kernel_clock_t l4_timeout_rel_get(l4_timeout_s to) L4_NOTHROW
{
  if (to.t == 0)
    return ~0ULL;
  return (l4_kernel_clock_t)(to.t & 0x3ff) << ((to.t >> 10) & 0x1f);
}


L4_INLINE
unsigned l4_timeout_is_absolute(l4_timeout_s to) L4_NOTHROW
{
  return to.t & 0x8000;
}


L4_INLINE
l4_kernel_clock_t l4_timeout_get(l4_kernel_clock_t cur, l4_timeout_s to) L4_NOTHROW
{
  if (l4_timeout_is_absolute(to))
    return 0; /* We cannot retrieve the value ... */
  else
    return cur + l4_timeout_rel_get(to);
}


#endif
