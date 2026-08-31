/*
 * Copyright (C) 2025 Kernkonzept GmbH.
 * Author(s): Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#pragma once

#include <stddef.h>
#include <stdio.h>
#include <l4/sys/compiler.h>

/**
 * Print the human-readable size of a given size.
 *
 * The generated output has the format
 * \code
 * i.f <unit>
 * \endcode
 * with
 * - 'i' being the integer part of 'bytes' with 1-4 digits,
 * - 'f' being the fractional part of 'bytes' with always 1 digit,
 * - 'unit' being the unit of 'v.f', for instance "123.4 MiB" or "4.5 GiB".
 * Values <= 1024 are printed as 'v B', for instance "1023 B".
 *
 * \param outstr   The string to print the size.
 * \param outsize  The size of the string to print.
 * \param bytes    The size value to print in human-readable form.
 */
L4_INLINE int l4util_human_readable_size(char *outstr, size_t outsize,
                                         unsigned long long bytes)
{
  static char const unitstr[7] = { 'B', 'K', 'M', 'G', 'T', 'P', 'E' };

  int idx = sizeof(unitstr) - 1;
  int order;

  for (order = idx * 10; order >= 10; order -= 10, --idx)
    if (bytes > (1ULL << order))
      break;

  unsigned long long value = bytes >> order;
  unsigned long long fract = (bytes - (value << order))
                             / ((1ULL << order) / 10 + 1);

  if (idx > 0)
    return snprintf(outstr, outsize, "%llu.%1llu %ciB",
                    value, fract, unitstr[idx]);
  else
    return snprintf(outstr, outsize, "%llu B  ", value);
}

/**
 * Print the human-readable size of a given given region begin..end.
 *
 * This function does the same as l4util_human_readable_size() but it properly
 * handles the size of a region spawning the whole address space 0..~0UL.
 *
 * The generated output has the format
 * \code
 * i.f <unit>
 * \endcode
 * with
 * - 'i' being the integer part of 'bytes' with 1-4 digits,
 * - 'f' being the fractional part of 'bytes' with always 1 digit,
 * - 'unit' being the unit of 'v.f', for instance "123.4 MiB" or "4.5 GiB".
 * Values <= 1024 are printed as 'v B', for instance "1023 B".
 *
 * \param outstr   The string to print the size.
 * \param outsize  The size of the string to print.
 * \param begin    The first byte of the region.
 * \param end      The last byte (inclusive) of the region.
 */
L4_INLINE int l4util_human_readable_size_region(char *outstr, size_t outsize,
                                                unsigned long long begin,
                                                unsigned long long end)
{
  if (end - begin == ~0UL)
    return snprintf(outstr, outsize, "%s",
                    sizeof(long) == 4 ? "4.0 GiB" : "16.0 EiB");
  else
    return l4util_human_readable_size(outstr, outsize, end - begin);
}
