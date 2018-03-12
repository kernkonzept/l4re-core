/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <l4/libloader/elf>

char const *Ldr::Elf_phdr::phdr_type() const
{
  static char const *low[] = {
      "NULL", "LOAD", "DYNAMIC", "INTERP", "NOTE", "SHLIB", "PHDR", "TLS",
      "NUM"};

  if (type() <= PT_NUM)
    return low[type()];

  switch (type())
    {
    case PT_L4_STACK: return "L4_STACK";
    case PT_L4_KIP:   return "L4_KIP";
    case PT_L4_AUX:   return "L4_AUX";
    case PT_GNU_EH_FRAME: return "GNU_EH_FRAME";
    case PT_GNU_STACK:    return "GNU_STACK";
    case PT_GNU_RELRO:    return "GNU_RELRO";
    default: return 0;
    }
}

