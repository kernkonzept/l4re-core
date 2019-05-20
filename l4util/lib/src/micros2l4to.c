/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>,
 *               Michael Hohmuth <hohmuth@os.inf.tu-dresden.de>,
 *               Jork Löser <jork@os.inf.tu-dresden.de>,
 *               Lars Reuther <reuther@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
/* 
 */

/*****************************************************************************
 * libl4util/src/micros2l4to.c                                               *
 * calculate L4 timeout                                                      *
 *****************************************************************************/

#include <l4/sys/compiler.h> /* for static_assert() */
#include <l4/sys/types.h>
#include <l4/util/util.h>
#include <l4/util/bitops.h>

L4_CV l4_timeout_s
l4util_micros2l4to(unsigned int mus)
{
  static_assert(sizeof(mus) <= 4,
                "Verify the correctness of log2(mus) and the number of bits for e!");
  l4_timeout_s t;
  if (mus == 0)
    t = L4_IPC_TIMEOUT_0;
  else if (mus == ~0U)
    t = L4_IPC_TIMEOUT_NEVER;
  else
    {
      /* Here it is certain that at least one bit in 'mus' is set. */
      int e = l4util_log2(mus) - 7;
      if (e < 0) e = 0;
      /* Here it is certain that '0 <= e <= 24' and '1 <= 2^e <= 2^24'. */

      unsigned m = mus >> e;
      /* Here it is certain that '1 <= m <= 255. Consider the following cases:
       *  o    1 <= mus <=  255: e = 0; 2^e = 1;   1 <= mus/1 <= 255
       *  o  256 <= mus <=  511: e = 1; 2^e = 2; 128 <= mus/2 <= 255
       *  o  512 <= mus <= 1023: e = 2; 2^e = 4; 128 <= mus/4 <= 255
       *  o 1024 <= mus <= 2047: e = 3; 2^e = 8; 128 <= mus/8 <= 255
       *  ...
       *  o 2^31 <= mus <= 2^32-1: e = 24;       128 <= mus/2^24 <= 255
       *
       * Dividing by (1<<e) ensures that for all mus < 2^32: m < 2^8.
       *
       * As we have 10 bits for m we could also use 'e = log2(mus) - 9':
       *  o    1 <= mus <= 1023: e = 0; 2^e = 1;   1 <= mus/1 <= 1023
       *  o 1024 <= mus <= 2047: e = 1; 2^e = 2; 512 <= mus/2 <= 1023
       *  o 2048 <= mus <= 4095: e = 2; 2^e = 4; 512 <= mus/4 <= 1023
       *  ...
       *  o 2^31 <= mus <= 2^32-1: e = 22;       512 <= mus/2^22 <= 1023
       *
       * What about sizeof(mus) == 8? 'e = log2(mus) - 7':
       *  o 2^63 <= mus <= 2^64-1: e = 56;       128 <= mus/2^56 <= 255.
       *
       * That means that this function would even work for 64-bit values of
       * 'mus' as long as l4util_log2(mus) works correctly for that range.
       * But the number of bits available for the exponent is limited:
       *  o bits 0..9 (10 bits) are used for 'm'
       *  o bits 10..14 (5 bits) are used for 'e'
       *  o bit 15 is used to distinguish between absolute timeouts and
       *    relative timeouts (see l4_timeout_is_absolute())
       *
       * That means 'e <= 31' and thus it's not possible to encode timeouts
       * represented by 64-bit values.
       */

      t.t = (e << 10) | m;
    }
  return t;
}
