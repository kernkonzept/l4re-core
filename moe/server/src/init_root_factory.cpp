/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include "init_root_factory.h"

#include "debug.h"
#include "globals.h"
#include "mem_region_list.h"
#include "page_alloc.h"

#include <l4/cxx/string>

#include <cstdlib>

Moe::Factory_config Moe::root_factory_config;

void Moe::init_root_factory_config(cxx::String config, cxx::String brk)
{
  if (config.empty())
    {
      l4_addr_t physmin = 0;
      if (!brk.empty() && brk.from_hex(&physmin) <= 0)
        {
          Err(Err::Fatal).printf("Invalid --brk option: '%.*s'\n",
                                 brk.len(),
                                 brk.start());
          exit(1);
        }

      // Use default values if no --root-factory config is provided.
      Moe::Mem_range all_mem{physmin, Moe::Max_phys_addr};
      if (!Moe::root_factory_config.regions.add(Moe::Mem_region::untyped(all_mem)))
        abort(); // should never happen
      Moe::root_factory_config.permissions = Moe::Factory_config::Scheduler_proxy;
      return;
    }

  if (!brk.empty())
    {
      Err(Err::Fatal).printf(
        "The options --brk and --root-factory are are mutually exclusive.\n");
      exit(1);
    }

  cxx::String args = config;
  for (;;)
  {
    cxx::String::Index delimiter = args.find(',');
    cxx::String arg = args.head(delimiter);
    if (arg.empty())
      break;

    args = args.substr(delimiter + 1);
    if (!Moe::parse_factory_config(arg, &Moe::root_factory_config))
      {
        Err(Err::Fatal).printf("Unknown argument for --root-factory: '%.*s'\n",
                               arg.len(), arg.start());
        exit(1);
      }
  }

  // Similar to sub-factory creation, if no region was specified at all, use
  // all memory as untyped memory.
  if (Moe::root_factory_config.regions.empty())
    {
      Moe::Mem_range all_mem{0, Moe::Max_phys_addr};
      if (!Moe::root_factory_config.regions.add(Moe::Mem_region::untyped(all_mem)))
        abort(); // should never happen
    }

  if (Moe::root_factory_config.untyped_regions().empty())
    {
      Err(Err::Fatal).printf(
        "The --root-factory option did not specify any untyped regions.\n");
      exit(1);
    }
}

void Moe::init_default_mem_cfg()
{
  // Moe internal allocations must only come from memory tagged as untyped in
  // the root factory config.
  Single_page_alloc_base::default_mem_cfg.regions =
    Moe::root_factory_config.untyped_regions();
}
