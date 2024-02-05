/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "init_mem.h"
#include "memmap.h"
#include "globals.h"

#include <l4/cxx/iostream>
#include <l4/sys/kip>
#include <l4/sys/kdebug.h>

using L4::Kip::Mem_desc;

void
init_memory(l4_kernel_info_t *info)
{
  if (info->version >> 24 != 0x87 /*KIP_VERSION_FIASCO*/ )
    {
      L4::cout << PROG_NAME": is designed to run on FIASCO only\n";
      enter_kdebug("FATAL");
    }

#ifdef ARCH_x86
  char kip_syscalls = info->kip_sys_calls;

  L4::cout << "  Found Fiasco: KIP syscalls: "
           << (kip_syscalls ? "yes\n" : "no\n");
#endif

  iomem.add_free(Region(0, ~0UL, 0, L4_FPAGE_RW));

  Region mismatch = Region::invalid();
  for (auto const &md: L4::Kip::Mem_desc::all(info))
    {
      if (md.is_virtual())
        continue;

      Mem_desc::Mem_type type = md.type();
      unsigned long start, end;
      if (type == Mem_desc::Conventional)
        {
          start = l4_round_page(md.start());
          end = l4_trunc_page(md.end() + 1) - 1;
        }
      else
        {
          start = l4_trunc_page(md.start());
          end = l4_round_page(md.end()) - 1;
        }

      switch (type)
        {
        case Mem_desc::Conventional:
          Mem_man::ram()->add_free(Region(start, end));
          if (!iomem.reserve(Region(start, end, sigma0_taskno, L4_FPAGE_RW)))
            mismatch = Region(start, end, sigma0_taskno, L4_FPAGE_RW);
          continue;
        case Mem_desc::Reserved:
        case Mem_desc::Dedicated:
          if (!iomem.reserve(Region(start, end, sigma0_taskno, L4_FPAGE_RW))
              || !Mem_man::ram()->reserve(Region(start, end, sigma0_taskno)))
            mismatch = Region(start, end, sigma0_taskno, L4_FPAGE_RW);
          break;
        case Mem_desc::Bootloader:
          if (!iomem.reserve(Region(start, end, sigma0_taskno, L4_FPAGE_RW))
              || !Mem_man::ram()->reserve(
                   Region(start, end, root_taskno,
                          static_cast<L4_fpage_rights>(md.sub_type()
                                                       & L4_FPAGE_RIGHTS_MASK))))
            mismatch = Region(start, end, root_taskno);
          break;
        case Mem_desc::Info:
        case Mem_desc::Arch:
        case Mem_desc::Shared:
          iomem.add_free(Region(start, end, 0, L4_FPAGE_RW));
          if (!Mem_man::ram()->reserve(
                Region(start, end, sigma0_taskno, L4_FPAGE_RW)))
            mismatch = Region(start, end, root_taskno);
          break;
        default:
          break;
        }

      if (mismatch.valid())
        {
          L4::cout << PROG_NAME": Could not reserve memory\n"
            << mismatch << "\n";
          dump_all();
          abort();
        }
    }
}
