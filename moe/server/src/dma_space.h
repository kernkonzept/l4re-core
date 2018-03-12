/*
 * (c) 2014 Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
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

namespace Dma {

struct Mapping;

class Mapper : public cxx::Ref_obj
{
public:
  Mapper(Mapper const &) = delete;
  Mapper() = default;

  typedef L4Re::Dma_space::Attributes Attributes;
  typedef L4Re::Dma_space::Direction Direction;
  typedef L4Re::Dma_space::Dma_addr Dma_addr;

  virtual Mapping *map(Dataspace *ds, Q_alloc *,
                       l4_addr_t offset, l4_size_t *size,
                       Attributes attrs, Direction dir,
                       Dma_addr *dma_addr) = 0;

  virtual int unmap(Dma_addr dma_addr, l4_size_t size,
                    Attributes attrs, Direction dir) = 0;

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
  typedef L4Re::Dma_space::Direction Direction;
  typedef L4Re::Dma_space::Attributes Attributes;

  Region key;
  Mapper *mapper = 0;
  Attributes attrs = Attributes::None;
  Direction dir = Direction::None;

  static Key_type key_of(Mapping const *m) { return m->key; }

  Mapping(Mapping const &) = delete;
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
  typedef L4Re::Dma_space::Dma_addr Dma_addr;
  typedef L4Re::Dma_space::Direction Direction;
  typedef L4Re::Dma_space::Attributes Attributes;
  typedef L4Re::Dma_space::Space_attribs Space_attribs;

  long op_map(L4Re::Dma_space::Rights rights,
              L4::Ipc::Snd_fpage src_ds, l4_addr_t offset,
              l4_size_t &size, Attributes attrs, Direction dir,
              Dma_addr &dma_addr);

  long op_unmap(L4Re::Dma_space::Rights rights,
                Dma_addr dma_addr,
                l4_size_t size, Attributes attrs, Direction dir);

  long op_associate(L4Re::Dma_space::Rights rights,
                    L4::Ipc::Snd_fpage dma_task,
                    Space_attribs attr);

  long op_disassociate(L4Re::Dma_space::Rights rights);

  void delete_all_mappings();

  ~Dma_space() { delete_all_mappings(); }

private:
  Space_attribs _attr = Space_attribs::None;
  cxx::Ref_ptr<Dma::Mapper> _mapper;
  Dma::Mapping::List _mappings;
};

}
