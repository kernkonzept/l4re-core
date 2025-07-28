/*
 * Copyright (C) 2025 Kernkonzept GmbH.
 * Author(s): Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#pragma once

#include <stddef.h>
#include <l4/sys/compiler.h>

/**
 * Print the human-readable size of a given size.
 *
 * The generate output has the format
 * \code
 * i.f <unit>
 * \endcode
 * with
 * - 'i' being the integer part of 'bytes' with 1-3 digits,
 * - 'f' being the fractional part of 'bytes' with always 1 digit,
 * - 'unit' being the unit of 'v.f', for instance "123.4 MB" or "4.5 GB".
 *
 * \param outstr   The string to print the size.
 * \param outsize  The size of the string to print.
 * \param bytes    The size value to print in human-readable form.
 */
L4_INLINE int l4util_human_readable_size(char *outstr, size_t outsize,
                                         unsigned long long bytes)
{
  static char const *const unitstr = "BKMGT";

  int idx = sizeof(unitstr) - 2;
  int order;

  for (order = idx * 10; order > 10; order -= 10, --idx)
    if (bytes > (1ULL << order))
      break;

  unsigned long long value = bytes >> order;
  unsigned long long fract = (bytes - (value << order))
                             / ((1ULL << order) / 10 + 1);

  return snprintf(outstr, outsize, "%llu.%1llu %ciB",
                  value, fract, unitstr[idx]);
}
