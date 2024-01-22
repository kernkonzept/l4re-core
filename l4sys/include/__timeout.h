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
#include <l4/sys/compiler.h>

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
typedef union l4_timeout_t
{
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
/**@{*/
#define L4_IPC_TIMEOUT_0 ((l4_timeout_s){0x0400})           /**< 0 timeout */
#define L4_IPC_TIMEOUT_NEVER ((l4_timeout_s){0})            /**< never timeout */
#define L4_IPC_NEVER_INITIALIZER {0}                        /**< never timeout, initializer */
#define L4_IPC_NEVER ((l4_timeout_t){0})                    /**< never timeout */
#define L4_IPC_RECV_TIMEOUT_0 ((l4_timeout_t){0x00000400})  /**< 0 receive timeout */
#define L4_IPC_SEND_TIMEOUT_0 ((l4_timeout_t){0x04000000})  /**< 0 send timeout */
#define L4_IPC_BOTH_TIMEOUT_0 ((l4_timeout_t){0x04000400})  /**< 0 receive and send timeout */

/**
 * The waiting period in microseconds which is interpreted as "never" by
 * l4_timeout_from_us().
 */
#define L4_TIMEOUT_US_NEVER (~0U)

/**
 * The longest waiting period in microseconds accepted by l4_timeout_from_us().
 */
#define L4_TIMEOUT_US_MAX   (~0U - 1)

/**@}*/

/**
 * Get relative timeout consisting of mantissa and exponent.
 * \ingroup l4_timeout_api
 *
 * \param man  Mantissa of timeout
 * \param exp  Exponent of timeout
 *
 * \return timeout value
 */
L4_CONSTEXPR L4_INLINE
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
L4_CONSTEXPR L4_INLINE
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
L4_CONSTEXPR L4_INLINE
l4_timeout_t l4_timeout(l4_timeout_s snd, l4_timeout_s rcv) L4_NOTHROW;

/**
 * Set send timeout in given to timeout.
 * \ingroup l4_timeout_api
 *
 * \param snd      Send timeout
 * \param[out] to  L4 timeout
 */
L4_CONSTEXPR L4_INLINE
void l4_snd_timeout(l4_timeout_s snd, l4_timeout_t *to) L4_NOTHROW;

/**
 * Set receive timeout in given to timeout.
 * \ingroup l4_timeout_api
 *
 * \param      rcv    Receive timeout
 * \param[out] to     L4 timeout
 */
L4_CONSTEXPR L4_INLINE
void l4_rcv_timeout(l4_timeout_s rcv, l4_timeout_t *to) L4_NOTHROW;

/**
 * Get clock value of out timeout.
 * \ingroup l4_timeout_api
 *
 * \param to     L4 timeout
 *
 * \return Clock value
 */
L4_CONSTEXPR L4_INLINE
l4_kernel_clock_t l4_timeout_rel_get(l4_timeout_s to) L4_NOTHROW;


/**
 * Return whether the given timeout is absolute or not.
 * \ingroup l4_timeout_api
 *
 * \param to     L4 timeout
 *
 * \return != 0 if absolute, 0 if relative
 */
L4_CONSTEXPR L4_INLINE
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
L4_CONSTEXPR L4_INLINE
l4_kernel_clock_t l4_timeout_get(l4_kernel_clock_t cur, l4_timeout_s to) L4_NOTHROW;

/**
 * Create a relative L4 timeout from a waiting period specified in microseconds.
 *
 * \param us     Waiting period in microseconds.
 *
 * \return Relative L4 timeout according to the specified waiting period.
 */
L4_CONSTEXPR L4_INLINE
l4_timeout_s l4_timeout_from_us(l4_uint32_t us) L4_NOTHROW;

/*
 * Implementation
 */

L4_CONSTEXPR L4_INLINE
l4_timeout_t l4_ipc_timeout(unsigned snd_man, unsigned snd_exp,
                            unsigned rcv_man, unsigned rcv_exp) L4_NOTHROW
{
  l4_uint16_t snd = (snd_man & 0x3ff) | ((snd_exp << 10) & 0x7c00);
  l4_uint16_t rcv = (rcv_man & 0x3ff) | ((rcv_exp << 10) & 0x7c00);
  return l4_timeout((l4_timeout_s){snd}, (l4_timeout_s){rcv});
}


L4_CONSTEXPR L4_INLINE
l4_timeout_t l4_timeout(l4_timeout_s snd, l4_timeout_s rcv) L4_NOTHROW
{
  return (l4_timeout_t){ ((l4_uint32_t){snd.t} << 16) | rcv.t };
}


L4_CONSTEXPR L4_INLINE
void l4_snd_timeout(l4_timeout_s snd, l4_timeout_t *to) L4_NOTHROW
{
  to->p.snd = snd;
}


L4_CONSTEXPR L4_INLINE
void l4_rcv_timeout(l4_timeout_s rcv, l4_timeout_t *to) L4_NOTHROW
{
  to->p.rcv = rcv;
}


L4_CONSTEXPR L4_INLINE
l4_timeout_s l4_timeout_rel(unsigned man, unsigned exp) L4_NOTHROW
{
  return (l4_timeout_s){(l4_uint16_t)((man & 0x3ff) | ((exp << 10) & 0x7c00))};
}


L4_CONSTEXPR L4_INLINE
l4_kernel_clock_t l4_timeout_rel_get(l4_timeout_s to) L4_NOTHROW
{
  if (to.t == 0)
    return ~0ULL;
  return (l4_kernel_clock_t)(to.t & 0x3ff) << ((to.t >> 10) & 0x1f);
}


L4_CONSTEXPR L4_INLINE
unsigned l4_timeout_is_absolute(l4_timeout_s to) L4_NOTHROW
{
  return to.t & 0x8000;
}


L4_CONSTEXPR L4_INLINE
l4_kernel_clock_t l4_timeout_get(l4_kernel_clock_t cur, l4_timeout_s to) L4_NOTHROW
{
  if (l4_timeout_is_absolute(to))
    return 0; /* We cannot retrieve the value ... */
  else
    return cur + l4_timeout_rel_get(to);
}

L4_CONSTEXPR L4_INLINE
l4_timeout_s l4_timeout_from_us(l4_uint32_t us) L4_NOTHROW
{
  static_assert(sizeof(us) <= 4,
                "Verify the correctness of log2(us) and the number of bits for e!");
  if (us == 0)
    return L4_IPC_TIMEOUT_0;
  else if (us == L4_TIMEOUT_US_NEVER)
    return L4_IPC_TIMEOUT_NEVER;
  else
    {
      /* Here it is certain that at least one bit in 'us' is set. */

      l4_uint16_t m = 0; // initialization required by constexpr, optimized away
      l4_uint16_t v = 0; // initialization required by constexpr, optimized away
      int e = (31 - __builtin_clz(us)) - 9;
      if (e < 0)
        e = 0;

      /* Here it is certain that '0 <= e <= 22' and '1 <= 2^e <= 2^22'. */

      m = us >> e;

      /* Here it is certain that '1 <= m <= 1023. Consider the following cases:
       *  o    1 <= us <= 1023: e = 0; 2^e = 1;   1 <= us/1 <= 1023
       *  o 1024 <= us <= 2047: e = 1; 2^e = 2; 512 <= us/2 <= 1023
       *  o 2048 <= us <= 4095: e = 2; 2^e = 4; 512 <= us/4 <= 1023
       *  ...
       *  o 2^31 <= us <= 2^32-1: e = 22;       512 <= us/2^22 <= 1023
       *
       * Dividing by (1<<e) ensures that for all us < 2^32: m < 2^10.
       *
       * What about sizeof(us) == 8? 'e = log2(us) - 9':
       *  o 2^63 <= us <= 2^64-1: e = 54;       512 <= us/2^54 <= 1023.
       *
       * That means that this function would even work for 64-bit values of
       * 'us' as long as __builtin_clz(us) works correctly for that range.
       * But the number of bits available for the exponent is limited:
       *  o bits 0..9 (10 bits) are used for 'm'
       *  o bits 10..14 (5 bits) are used for 'e'
       *  o bit 15 is used to distinguish between absolute timeouts and
       *    relative timeouts (see l4_timeout_is_absolute())
       *
       * That means 'e <= 31' and thus it's not possible to encode timeouts
       * represented by 64-bit values. */

      /* Without introducing 'v' we had to type-cast the expression to
       * l4_uint16_t. This cannot be avoided by declaring m and e_pow_10 as
       * l4_uint16_t due to C++ integer promotion. */
      v = (e << 10) | m;
      return (l4_timeout_s){v};
    }
}

#endif
