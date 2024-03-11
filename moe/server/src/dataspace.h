/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <cstddef>
#include <l4/sys/types.h>
#include <l4/cxx/iostream>
#include <l4/cxx/list>
#include <l4/sys/cxx/ipc_epiface>
#include <l4/re/dataspace>

#include "dma_space.h"
#include "server_obj.h"
#include "globals.h"
#include "quota.h"

namespace Moe {

/**
 * Base class for all dataspace types.
 *
 * Provides common implementations of the Dataspace interface functions.
 * The behaviour may be adapted by overwriting the provided virtual
 * functions.
 */
class Dataspace :
  public L4::Epiface_t<Dataspace, L4Re::Dataspace, Server_object>,
  public Q_object
{
public:
  using Flags = L4Re::Dataspace::Flags;
  enum { Cow_enabled = 0x100 };

  struct Address
  {
    l4_fpage_t fpage;
    l4_addr_t offs;

    Address(long error) noexcept : offs(-1UL) { fpage.raw = error; }

    Address(l4_addr_t base, l4_addr_t size,
            Flags flags,
            l4_addr_t offs = 0) noexcept
    : fpage(l4_fpage(base, size, flags.fpage_rights())),
      offs(offs) {}

    unsigned long bs() const noexcept { return fpage.raw & L4_FPAGE_ADDR_MASK; }
    unsigned long sz() const noexcept { return 1 << l4_fpage_size(fpage); }
    unsigned long of() const noexcept { return offs; }
    l4_fpage_t fp() const noexcept { return fpage; }

    /**
     * Get the start address of the dataspace. Requires `T` to be a pointer
     * type.
     */
    template< typename T >
    T adr() const noexcept { return reinterpret_cast<T>(bs() + offs); }

    void *adr() const noexcept { return reinterpret_cast<void*>(bs() + offs); }

    bool is_nil() const noexcept { return offs == -1UL; }
    /**
     * \brief Get the error code that led to the invalid address.
     * \pre is_nil() must return true.
     */
    long error() const noexcept { return fpage.raw; }

  };

  Dataspace(unsigned long size, Flags flags,
            unsigned char page_shift) noexcept
    : _size(size), _flags(flags), _page_shift(page_shift)
  {}


  unsigned long size() const noexcept { return _size; }
  virtual Address address(l4_addr_t ds_offset,
                          Flags flags = L4Re::Dataspace::F::RWX,
                          l4_addr_t hot_spot = 0,
                          l4_addr_t min = 0, l4_addr_t max = ~0) const = 0;
  virtual int copy_address(l4_addr_t ds_offset, Flags flags,
                           l4_addr_t *copy_addr, unsigned long *copy_size) const = 0;

  virtual int pre_allocate(l4_addr_t offset, l4_size_t size, unsigned rights) = 0;

  bool can_cow() const noexcept
  {
    return !!(_flags & Flags(Cow_enabled));
  }

  Flags flags() const noexcept { return _flags; }

  Flags map_flags(L4Re::Dataspace::Rights rights = L4_CAP_FPAGE_W) const noexcept
  {
    auto f = (_flags & Flags(L4Re::Dataspace::F::Rights_mask))
             | L4Re::Dataspace::F::Caching_mask;
    if (!(rights & L4_CAP_FPAGE_W))
      f &= ~L4Re::Dataspace::F::W;

    return f;
  }

  virtual ~Dataspace() {}

  unsigned long page_shift() const noexcept { return _page_shift; }
  unsigned long page_size() const noexcept { return 1UL << _page_shift; }

  virtual bool is_static() const noexcept = 0;
  virtual long clear(unsigned long offs, unsigned long size) const noexcept;

protected:
  void size(unsigned long size) noexcept { _size = size; }

public:
  unsigned long round_size() const noexcept
  { return l4_round_size(size(), page_shift()); }
  bool check_limit(l4_addr_t offset) const noexcept
  { return offset < round_size(); }
  bool check_range(l4_addr_t offset, unsigned long sz) const noexcept
  { return offset < size() && size() - offset >= sz; }

public:
  int map(l4_addr_t offs, l4_addr_t spot, Flags flags,
          l4_addr_t min, l4_addr_t max, L4::Ipc::Snd_fpage &memory);

  typedef Dma_space::Attributes Dma_attribs;
  virtual int dma_map(Dma_space *dma, l4_addr_t offset, l4_size_t *size,
                      Dma_attribs dma_attrs, Dma_space::Direction dir,
                      Dma_space::Dma_addr *dma_addr);

  long op_map(L4Re::Dataspace::Rights rights,
              L4Re::Dataspace::Offset offset,
              L4Re::Dataspace::Map_addr spot,
              L4Re::Dataspace::Flags flags, L4::Ipc::Snd_fpage &fp);

  long op_allocate(L4Re::Dataspace::Rights rights,
                   L4Re::Dataspace::Offset offset,
                   L4Re::Dataspace::Size size)
  { return pre_allocate(offset, size, rights & 3); }

  long op_copy_in(L4Re::Dataspace::Rights rights,
                  L4Re::Dataspace::Offset dst_offs,
                  L4::Ipc::Snd_fpage const &src_cap,
                  L4Re::Dataspace::Offset src_offs,
                  L4Re::Dataspace::Size sz);

  long op_info(L4Re::Dataspace::Rights rights, L4Re::Dataspace::Stats &s)
  {
    s.size = size();
    s.flags = flags();
    // only return writable if really writable
    if (!(rights & L4_CAP_FPAGE_W))
      s.flags &= ~L4Re::Dataspace::F::W;

    return L4_EOK;
  }

  long op_clear(L4Re::Dataspace::Rights rights,
                L4Re::Dataspace::Offset offset,
                L4Re::Dataspace::Size size)
  {
    if (!map_flags(rights).w())
      return -L4_EACCESS;

    return clear(offset, size);
  }


private:
  unsigned long  _size;
  Flags _flags;
  unsigned char  _page_shift;
};

}



