/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#pragma once

#include "mem_region_list.h"

#include <l4/cxx/static_vector>
#include <l4/cxx/string>
#include <l4/sys/l4int.h>

#include <cstring>

namespace Moe
{

class Mem_region
{
public:
  static constexpr unsigned Max_name_len = 23;

  Mem_region() : _name_len(0) {}

  static Mem_region untyped(Mem_range const &range)
  {
    Mem_region region;
    region.range = range;
    return region;
  }

  static Mem_region typed(Mem_range const &range, cxx::String const &name)
  {
    Mem_region region;
    region.range = range;
    assert(name.len() <= int{Max_name_len});
    region._name_len = cxx::min<unsigned>(name.len(), Max_name_len);
    memcpy(region._name, name.start(), region._name_len);
    return region;
  }

  bool valid() const { return range.valid(); }
  cxx::String name() const { return cxx::String(_name, _name_len); }
  size_t size() const { return range.size(); }
  bool is_typed() const { return _name_len > 0; }

  friend bool operator < (Mem_region const &lhs, Mem_region const &rhs)
  {
      // First compare by name, and only if equal compare by range.
      if (lhs._name_len != rhs._name_len)
        return lhs._name_len < rhs._name_len;

      int name_cmp = memcmp(lhs._name, rhs._name, lhs._name_len);
      if (name_cmp != 0)
        return name_cmp < 0;

      return lhs.range < rhs.range;
  }

  bool overlaps(Mem_region const &o) const
  { return name() == o.name() && range.overlaps(o.range); }

  bool contains(Mem_region const &o) const
  { return name() == o.name() && range.contains(o.range); }

  bool adjacent(Mem_region const &o) const
  { return name() == o.name() && range.adjacent(o.range); }

  void merge(Mem_region const &r)
  {
    assert(name() == r.name());
    range.merge(r.range);
  }

  Mem_region intersect(Mem_region const &o) const
  {
    if (!overlaps(o))
      return Mem_region();

    Mem_region res = *this;
    res.range = range.intersect(o.range);
    return res;
  }

  template<typename DBG>
  void
  dump(DBG &out) const
  {
    out.printf("[%.*s] ", _name_len, _name);
    range.dump(out, false);
  }

  Mem_range range;

private:
  l4_uint8_t _name_len;
  char _name[Max_name_len]; // not null-terminated
};

struct Factory_config
{
  static constexpr unsigned Max_regions = 15;

  enum Permissions : unsigned
  {
    Phys_sub_factory = 1 << 0, // p = sub-factories with phys constraints
    Dma = 1 << 1,              // d = DMA constraints
    Scheduler_proxy = 1 << 2,  // s = allow creating scheduler proxies
  };

  using Region_list = Mem_region_list<Mem_region, Max_regions>;
  class Region_list_view : public cxx::static_vector<Mem_region const>
  {
  public:
    using cxx::static_vector<Mem_region const>::static_vector;

    bool contains(Mem_range const &range) const
    {
      for (auto const &candidate : *this)
        {
          if (candidate.range.contains(range))
            return true;
        }

        return false;
    }
  };

  Region_list regions;
  unsigned permissions = 0;

  bool has_permission(Permissions permission) const
  {
    return permissions & permission;
  }

  /**
   * Validate that sub-factory config adheres to the constraints and permissions
   * of its parent.
   *
   * \param parent  Factory that creates the sub-factory.
   *
   * \return true if the sub-factory config is valid, false if not.
   */
  [[nodiscard]] bool validate(Factory_config const &parent) const;

  /// Return all regions with the given type name.
  Region_list_view regions_with_type(cxx::String type) const;

  /// Return all untyped regions.
  Region_list_view untyped_regions() const { return regions_with_type(""); }

  /// Return all typed regions.
  Region_list_view typed_regions() const
  {
    Region_list_view untyped = untyped_regions();
    unsigned num_typed = regions.len() - untyped.size();
    if (num_typed > 0)
      return {untyped.end(), num_typed};
    return {};
  }

  template<typename DBG>
  void
  dump(DBG &out) const
  {
    out.printf("Permissions: %s%s%s\n",
               permissions & Phys_sub_factory ? "p" : "",
               permissions & Dma ? "d" : "",
               permissions & Scheduler_proxy ? "s" : "");
    out.printf("Untyped regions:\n");
    for (auto const &region : untyped_regions())
      region.dump(out);
    out.printf("Typed regions:\n");
    for (auto const &region : typed_regions())
      region.dump(out);
  }
};

[[nodiscard]] bool parse_factory_config(cxx::String const &arg, Factory_config *config);

} // namespace Moe
