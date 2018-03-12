/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <l4/sys/kip.h>
#include <l4/util/kip.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


L4_CV int
l4util_kip_kernel_is_ux(l4_kernel_info_t *k)
{
  const char *s = l4_kip_version_string(k);

  if (s && strstr(s, "(ux)"))
    return 1;
  return 0;
}

L4_CV int
l4util_kip_kernel_has_feature(l4_kernel_info_t *k, const char *str)
{
  const char *s = l4_kip_version_string(k);

  if (!s)
    return 0;

  l4util_kip_for_each_feature(s)
    {
      if (strcmp(s, str) == 0)
	return 1;
    }

  return 0;
}

L4_CV unsigned long
l4util_kip_kernel_abi_version(l4_kernel_info_t *k)
{
  const char *s = l4_kip_version_string(k);

  if (!s)
    return 0;

  l4util_kip_for_each_feature(s)
    {
      if (strncmp(s, "abiver:", 7) == 0)
	return strtoul(s + 7, NULL, 0);
    }

  return 0;
}
