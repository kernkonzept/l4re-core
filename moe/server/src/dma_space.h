/*
 * (c) 2014 Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <cstddef>
#include <l4/sys/types.h>
#include <l4/re/dma_space>
#include <l4/sys/cxx/ipc_epiface>
#include <l4/cxx/hlist>
#include <l4/cxx/avl_tree>
#include <l4/cxx/ref_ptr>

#include "quota.h"
#include "server_obj.h"
#include "globals.h"

namespace Moe {

class Dma_space;
class Name_space;

namespace Dma {

static constexpr L4Re::Dma_space::Dma_addr Last_dma_addr = -1;

struct Mapping;

struct Region
{
  L4Re::Dma_space::Dma_addr start;
  L4Re::Dma_space::Dma_addr end;
  bool operator < (Region const &r) const
  { return end < r.start; }

  Region() = default;
  Region(L4Re::Dma_space::Dma_addr s) : start(s), end(s) {}
  Region(L4Re::Dma_space::Dma_addr s, L4Re::Dma_space::Dma_addr e)
  : start(s), end(e) {}
};

class Mapper : public cxx::Ref_obj
{
public:
  Mapper(Mapper const &) = delete;
  Mapper() = default;
  Mapper &operator = (Mapper const &) = delete;

  virtual l4_ret_t add_dma_space(Dma_space *dma_space);
  virtual void remove_dma_space(Dma_space *dma_space);

  virtual l4_ret_t map(Dataspace *ds, L4Re::Dataspace::Offset offset,
                       L4Re::Dma_space::Dma_size *size, unsigned char align,
                       L4Re::Dataspace::Flags flags,
                       L4Re::Dma_space::Attributes attrs,
                       L4Re::Dma_space::Dma_addr *dma_addr,
                       L4Re::Dma_space::Dma_addr dma_max) = 0;

  virtual void unmap(L4Re::Dma_space::Dma_addr start,
                     L4Re::Dma_space::Dma_size size) = 0;
  void unmap(Region const &r)
  { unmap(r.start, r.end - r.start + 1); }

  virtual l4_ret_t check_blocking_area(L4Re::Dma_space::Dma_addr *start,
                                       L4Re::Dma_space::Dma_addr max,
                                       L4Re::Dma_space::Dma_size size,
                                       bool search, unsigned char align) = 0;

  virtual l4_ret_t set_limits(Dma_space *dma_space,
                              L4Re::Dma_space::Dma_addr min_addr,
                              L4Re::Dma_space::Dma_addr max_addr) = 0;

  virtual ~Mapper() = default;

protected:
  typedef cxx::H_list_t<Dma_space> Dma_space_list;

  Dma_space_list _dma_spaces;
};

struct Mapping : cxx::Avl_tree_node
{
  typedef Region Key_type;
  typedef cxx::Avl_tree<Mapping, Mapping, cxx::Lt_functor<Region>> Map;

  Region key;

  /**
   * Number of claims that own a kernel DMA mapping.
   *
   * When this counter reaches 0, the underlying kernel DMA mapping (if any)
   * must be released via Mapper::unmap().
   */
  unsigned mapcnt = 0;

  bool add_mapping()
  {
    if (mapcnt + 1U < mapcnt) // prevent overflow
      return false;
    ++mapcnt;
    return true;
  }

  bool del_mapping()
  {
    if (mapcnt == 0)  // prevent underflow
      return false;
    return --mapcnt == 0;
  }

  /// Number of reservations covering that region.
  unsigned rsvcnt = 0;

  bool add_reservation()
  {
    if (rsvcnt + 1U < rsvcnt) // prevent overflow
      return false;
    ++rsvcnt;
    return true;
  }

  bool del_reservation()
  {
    if (rsvcnt == 0)  // prevent underflow
      return false;
    return --rsvcnt == 0;
  }

  /**
   * Indicator if region is blocked from all mappings or reservations.
   *
   * Blockings cannot be removed. So this counter saturates at two. The reason
   * why it is a counter in the first place is Dma_space::add_region(). If the
   * allocation of a new node fails, the previously added nodes must be removed
   * but existing blockings shall be retained.
   */
  unsigned char blocked = 0;

  bool add_block()
  {
    if (blocked < 2)
      ++blocked;
    return true;
  }

  bool del_block()
  {
    if (blocked == 1)
      {
        blocked = 0;
        return true;
      }
    return false;
  }

  bool is_referenced() const    { return blocked || mapcnt || rsvcnt; }
  bool has_mappings() const     { return mapcnt > 0; }
  bool has_reservations() const { return rsvcnt > 0; }
  bool is_blocked() const       { return blocked > 0; }

  static Key_type key_of(Mapping const *m) { return m->key; }

  Mapping(Mapping const &) = delete;
  Mapping &operator = (Mapping const &) = delete;
  Mapping() = default;
};

}

class Dma_space :
  public L4::Epiface_t<Dma_space, L4Re::Dma_space, Server_object>,
  public cxx::H_list_item_t<Dma_space>,
  public Q_object
{
public:
  l4_ret_t op_map(L4Re::Dma_space::Rights rights,
                  L4::Ipc::Snd_fpage src_ds,
                  L4Re::Dataspace::Offset offset,
                  L4Re::Dma_space::Dma_size &size, unsigned char align,
                  L4Re::Dma_space::Attributes attrs,
                  L4Re::Dma_space::Dma_addr &dma_addr,
                  L4Re::Dma_space::Dma_addr dma_max);

  l4_ret_t op_unmap(L4Re::Dma_space::Rights rights,
                    L4Re::Dma_space::Dma_addr dma_addr,
                    L4Re::Dma_space::Dma_size size,
                    L4Re::Dma_space::Unmap_flags flags);

  ~Dma_space() { disassociate(); }

  /**
   * Return the Dma::Mapping with the lowest address that intersects with the
   * regions `r`.
   */
  Dma::Mapping *find_first_region(Dma::Region r)
  {
    auto *m = _mappings.lower_bound_node(Dma::Region(r.start));
    if (m && m->key.start <= r.end)
      return m;
    else
      return nullptr;
  }

  /**
   * Return the Dma::Mapping node that intersects with region `r`.
   *
   * If multiple regions intersect, it is unspecified which region is returned.
   */
  Dma::Mapping *find_any_region(Dma::Region r)
  { return _mappings.find_node(r); }

  // Dma_space_mgr internal interface
  l4_ret_t associate(cxx::Ref_ptr<Dma::Mapper> const &mapper);
  l4_ret_t disassociate();
  l4_ret_t block_area(L4Re::Dma_space::Dma_addr *start,
                      L4Re::Dma_space::Dma_addr max,
                      L4Re::Dma_space::Dma_size size,
                      bool search, unsigned char align);
  l4_ret_t set_limits(L4Re::Dma_space::Dma_addr min_addr,
                      L4Re::Dma_space::Dma_addr max_addr);

  bool empty() const
  { return _mappings.begin() == _mappings.end(); }

  bool unconstrained() const
  { return _min == 0 && _max == Dma::Last_dma_addr; }

  L4Re::Dma_space::Dma_addr min_addr() const { return _min; }
  L4Re::Dma_space::Dma_addr max_addr() const { return _max; }

private:
  enum class Add { Mapping, Reservation, Block };
  l4_ret_t add_region(L4Re::Dma_space::Dma_addr start,
                      L4Re::Dma_space::Dma_addr end,
                      Add type);

  /// The Dma::Mapper instance (if any) associated with this Moe::Dma_space
  /// instance.
  /// \note Several Moe::Dma_space instances might share the same Dma::Mapper
  ///       instance.
  cxx::Ref_ptr<Dma::Mapper> _mapper;

  /// List of mappings (see Dma::Mapping) created via this Moe::Dma_space
  /// instance.
  Dma::Mapping::Map _mappings;
  L4Re::Dma_space::Dma_addr _min = 0;
  L4Re::Dma_space::Dma_addr _max = Dma::Last_dma_addr;
};

class Dma_space_mgr :
  public L4::Epiface_t<Dma_space_mgr, L4Re::Dma_space_mgr, Server_object>
{
public:
  Dma_space_mgr(Moe::Name_space *ns, char const *name);

  l4_ret_t op_associate(L4Re::Dma_space_mgr::Rights         rights,
                        L4::Ipc::Snd_fpage                  dma_space,
                        L4::Ipc::Snd_fpage                  dma_task,
                        L4Re::Dma_space_mgr::Space_attribs  attr);

  l4_ret_t op_associate_phys(L4Re::Dma_space_mgr::Rights        rights,
                             L4::Ipc::Snd_fpage                 dma_space,
                             L4Re::Dma_space_mgr::Space_attribs attr);

  l4_ret_t op_disassociate(L4Re::Dma_space_mgr::Rights  rights,
                           L4::Ipc::Snd_fpage           dma_space);

  l4_ret_t op_block_area(L4Re::Dma_space_mgr::Rights        rights,
                         L4::Ipc::Snd_fpage                 dma_space_cap,
                         L4Re::Dma_space_mgr::Dma_addr      &start,
                         L4Re::Dma_space_mgr::Dma_size      size,
                         L4Re::Dma_space_mgr::Dma_addr      max,
                         L4Re::Dma_space_mgr::Block_flags   flags,
                         unsigned char                      align);

  l4_ret_t op_set_limits(L4Re::Dma_space_mgr::Rights    rights,
                         L4::Ipc::Snd_fpage             dma_space_cap,
                         L4Re::Dma_space_mgr::Dma_addr  min_addr,
                         L4Re::Dma_space_mgr::Dma_addr  max_addr);

private:
  l4_ret_t check_dma_space(L4::Ipc::Snd_fpage const &dma_space,
                           Moe::Dma_space **res);
};


}
