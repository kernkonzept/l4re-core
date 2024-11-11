/**
 * \file
 * \brief Split a range in log2 aligned and size-aligned chunks.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#ifndef __L4UTIL__INCLUDE__SPLITLOG2_H__
#define __L4UTIL__INCLUDE__SPLITLOG2_H__

#include <l4/sys/linkage.h>
#include <l4/sys/err.h>
#include <l4/util/bitops.h>

__BEGIN_DECLS

/**
 * \brief Split a range into log2 base and size aligned chunks.
 * \ingroup l4util_api
 *
 * \param start   Start of range
 * \param end     End of range (inclusive) (e.g. 2-4 is len 3)
 * \param handler Handler function that is called with start and end
 *                (both inclusive) of the chunk. On success, the handler
 *                must return 0, if it returns !=0 the function will
 *                immediately return with the return code of the handler.
 * \return 0 on success, != 0 otherwise
 */
L4_INLINE long
l4util_splitlog2_hdl(l4_addr_t start, l4_addr_t end,
                     long (*handler)(l4_addr_t s, l4_addr_t e, int log2size));

/**
 * \brief Return log2 base and size aligned length of a range.
 * \ingroup l4util_api
 *
 * \param start   Start of range
 * \param end     End of range (inclusive) (e.g. 2-4 is len 3)
 * \return length of elements in log2size (length is 1 << log2size)
 */
L4_INLINE l4_addr_t
l4util_splitlog2_size(l4_addr_t start, l4_addr_t end);

__END_DECLS

/* Implementation */

L4_INLINE long
l4util_splitlog2_hdl(l4_addr_t start, l4_addr_t end,
                     long (*handler)(l4_addr_t s, l4_addr_t e, int log2size))
{
  if (end < start)
    return -L4_EINVAL;

  while (start <= end)
    {
      long retval;
      int len2 = l4util_splitlog2_size(start, end);
      l4_addr_t len = 1UL << len2;
      if ((retval = handler(start, start + len - 1, len2)))
	return retval;
      start += len;
    }
  return 0;
}

L4_INLINE l4_addr_t
l4util_splitlog2_size(l4_addr_t start, l4_addr_t end)
{
  int start_bits = l4util_bsf(start);
  int len_bits = l4util_bsr(end - start + 1);
  if (start_bits != -1 && len_bits > start_bits)
    len_bits = start_bits;

  return len_bits;
}

#endif /* ! __L4UTIL__INCLUDE__SPLITLOG2_H__ */
