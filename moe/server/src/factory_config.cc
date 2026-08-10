/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include "factory_config.h"
#include "debug.h"

#include <cctype>

namespace Moe
{

namespace
{

Dbg const info(Dbg::Info);

bool
parse_mem_range(cxx::String const &value, Mem_range *range)
{
  cxx::String::Index delimiter = value.find("@");
  if (delimiter == value.end())
    return false;

  cxx::String length_str = value.head(delimiter);
  if (length_str.starts_with("0x"))
    length_str = length_str.substr(2);
  cxx::String base_str = value.substr(delimiter + 1);
  if (base_str.starts_with("0x"))
    base_str = base_str.substr(2);

  l4_addr_t length;
  l4_addr_t base;
  if (length_str.from_hex(&length) != length_str.len()
      || base_str.from_hex(&base) != base_str.len())
    return false;

  // Zero-length or wrap-around are considered invalid
  if (length <= 0 || base + length - 1 < base)
    return false;

  range->start = base;
  range->end = base + length - 1;
  return true;
}

bool
parse_factory_arg_untyped(cxx::String const &value, Factory_config *config)
{
  Mem_range range;
  if (!parse_mem_range(value, &range))
    return false;

  return config->regions.add(Mem_region::untyped(range));
}

bool
parse_factory_arg_typed(cxx::String const &value, Factory_config *config)
{
  cxx::String::Index delimiter = value.find("=");
  if (delimiter == value.end())
    return false;

  cxx::String name = value.head(delimiter);
  if (name.empty())
    return false;

  // Name must only contain alpha-numeric characters.
  for (int i = 0; i < name.len(); i++)
    if (!isalnum(name[i]))
      return false;

  if (name.len() > int{Mem_region::Max_name_len})
    return false;

  cxx::String region = value.substr(delimiter + 1);
  Mem_range range;
  if (!parse_mem_range(region, &range))
    return false;

  return config->regions.add(Mem_region::typed(range, name));
}

bool
parse_factory_arg_allow(cxx::String const &value, Factory_config *config)
{
  for (int i = 0; i < value.len(); i++)
    switch (value[i])
      {
      case 'p': config->permissions |= Factory_config::Phys_sub_factory; break;
      case 'd': config->permissions |= Factory_config::Dma; break;
      case 's': config->permissions |= Factory_config::Scheduler_proxy; break;
      default: return false;
      }

  return true;
}

} // namespace

bool Factory_config::validate(Factory_config const &parent) const
{
  // Specifying typed and untyped memory regions requires the parent factory to
  // have the `Phys_sub_factory` permission.
  if (!regions.empty()
      && !parent.has_permission(Moe::Factory_config::Phys_sub_factory))
    return false;

  // Typed and untyped memory regions that are not backed by the parent factory
  // physical memory regions are rejected with EPERM.
  bool valid = true;
  for (auto const &region : regions)
    {
      // TODO: What if the region is covered by the combination of multiple
      //       different untyped/typed regions in the parent?
      if (!parent.untyped_regions().contains(region.range)
          && !parent.typed_regions().contains(region.range))
        {
          info.printf("Region not contained in parent:\n");
          region.dump(info);
          valid = false;
        }
    }

  // Trying to grant more permissions to a sub-factory than the parent factory
  // has is rejected with EPERM.
  if ((permissions & parent.permissions) != permissions)
    {
      info.printf("Permissions not held by the parent factory.\n");
      valid = false;
    }

  return valid;
}

Moe::Factory_config::Region_list_view
Factory_config::regions_with_type(cxx::String type) const
{
  Mem_region const *first = nullptr;
  unsigned num = 0;
  for (unsigned i = 0; i < regions.len(); i++)
    {
      if (regions[i].name() == type)
        {
          ++num;
          if (first == nullptr)
            first = &regions[i];
        }
      else if (first != nullptr)
        break;
    }

  if (first)
    return {first, num};
  return {};
}

bool
parse_factory_config(cxx::String const &arg, Factory_config *config)
{
  cxx::String::Index delimiter = arg.find("=");
  if (delimiter == arg.end())
    return false;

  cxx::String param = arg.head(delimiter);
  cxx::String value = arg.substr(delimiter + 1);

  if (param == "untyped" || param == "ut")
    return parse_factory_arg_untyped(value, config);

  if (param == "typed" || param == "t")
    return parse_factory_arg_typed(value, config);

  if (param == "allow" || param == "a")
    {
      // Fail if allow is specified multiple times, to prevent accidental
      // misconfiguration.
      if (config->permissions != 0)
        return false;

      return parse_factory_arg_allow(value, config);
    }

  return false;
}

} // namespace Moe
