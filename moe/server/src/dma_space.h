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

struct Mapping;

class Mapper : public cxx::Ref_obj
{
public:
  Mapper(Mapper const &) = delete;
  Mapper() = default;
  Mapper &operator = (Mapper const &) = delete;


  virtual Mapping *map(Dataspace *ds, Q_alloc *,
                       L4Re::Dataspace::Offset offset,
                       L4Re::Dma_space::Dma_size *size,
                       L4Re::Dma_space::Dma_addr *dma_addr) = 0;

  virtual l4_ret_t unmap(L4Re::Dma_space::Dma_addr dma_addr,
                         L4Re::Dma_space::Dma_size size) = 0;

  virtual void remove(Mapping *m) = 0;

  virtual ~Mapper() = default;
};

struct Region
{
  l4_addr_t start;
  l4_addr_t end;
  bool operator < (Region const &r) const
  { return end < r.start; }

  Region() = default;
  Region(l4_addr_t s) : start(s), end(s) {}
  Region(l4_addr_t s, l4_addr_t e) : start(s), end(e) {}
};

struct Mapping : cxx::H_list_item_t<Mapping>, cxx::Avl_tree_node
{
  typedef Region Key_type;
  typedef cxx::Avl_tree<Mapping, Mapping, cxx::Lt_functor<Region>> Map;
  typedef cxx::H_list_t<Dma::Mapping> List;

  Region key;
  Mapper *mapper = 0;

  static Key_type key_of(Mapping const *m) { return m->key; }

  Mapping(Mapping const &) = delete;
  Mapping &operator = (Mapping const &) = delete;
  Mapping() = default;

  ~Mapping()
  {
    if (mapper)
      mapper->remove(this);

    if (0)
      printf("DMA: del mapping: map=%p %lx %lx\n", mapper, key.start, key.end);
  }
};

}

class Dma_space :
  public L4::Epiface_t<Dma_space, L4Re::Dma_space, Server_object>,
  public Q_object
{
public:
  l4_ret_t op_map(L4Re::Dma_space::Rights rights,
                  L4::Ipc::Snd_fpage src_ds,
                  L4Re::Dataspace::Offset offset,
                  L4Re::Dma_space::Dma_size &size,
                  L4Re::Dma_space::Attributes attrs,
                  L4Re::Dma_space::Dma_addr &dma_addr);

  l4_ret_t op_unmap(L4Re::Dma_space::Rights rights,
                    L4Re::Dma_space::Dma_addr dma_addr,
                    L4Re::Dma_space::Dma_size size);

  /**
   * Delete all mappings (see Dma::Mapping) created via *this* Moe::Dma_space
   * instance.
   */
  void delete_all_mappings();

  ~Dma_space() { delete_all_mappings(); }


  // Dma_space_mgr internal interface
  l4_ret_t associate(cxx::Ref_ptr<Dma::Mapper> const &mapper);
  l4_ret_t disassociate();

private:
  /// The Dma::Mapper instance (if any) associated with this Moe::Dma_space
  /// instance.
  /// \note Several Moe::Dma_space instances might share the same Dma::Mapper
  ///       instance.
  cxx::Ref_ptr<Dma::Mapper> _mapper;

  /// List of mappings (see Dma::Mapping) created via this Moe::Dma_space
  /// instance.
  Dma::Mapping::List _mappings;
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


private:
  l4_ret_t check_dma_space(L4::Ipc::Snd_fpage const &dma_space,
                           Moe::Dma_space **res);
};


}
