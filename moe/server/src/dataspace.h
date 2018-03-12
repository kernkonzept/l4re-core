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

class Dataspace :
  public L4::Epiface_t<Dataspace, L4Re::Dataspace, Server_object>,
  public Q_object
{
public:
  enum Ds_rw
  {
    Read_only   = L4Re::Dataspace::Map_ro,
    Writable    = L4Re::Dataspace::Map_rw,
    Cow_enabled = 0x100,
  };

  struct Address
  {
    l4_fpage_t fpage;
    l4_addr_t offs;

    Address(long error) throw() : offs(-1UL) { fpage.raw = error; }

    Address(l4_addr_t base, l4_addr_t size, Ds_rw rw = Read_only,
            l4_addr_t offs = 0) throw()
    : fpage(l4_fpage(base, size, rw ? L4_FPAGE_RWX : L4_FPAGE_RX)),
      offs(offs) {}

    unsigned long bs() const throw() { return fpage.raw & L4_FPAGE_ADDR_MASK; }
    unsigned long sz() const throw() { return 1 << l4_fpage_size(fpage); }
    unsigned long of() const throw() { return offs; }
    l4_fpage_t fp() const throw() { return fpage; }

    template< typename T >
    T adr() const throw() { return (T)(bs() + offs); }

    void *adr() const throw() { return (void*)(bs() + offs); }

    bool is_nil() const throw() { return offs == -1UL; }
    /**
     * \brief Get the error code that led to the invalid address.
     * \pre is_nil() must return true.
     */
    long error() const throw() { return fpage.raw; }

  };

  Dataspace(unsigned long size, unsigned short flags,
            unsigned char page_shift) throw()
    : _size(size), _flags(flags), _page_shift(page_shift)
  {}


  unsigned long size() const throw() { return _size; }
  virtual void unmap(bool ro = false) const throw() = 0;
  virtual Address address(l4_addr_t ds_offset,
                          Ds_rw rw = Writable, l4_addr_t hot_spot = 0,
                          l4_addr_t min = 0, l4_addr_t max = ~0) const = 0;

  virtual int pre_allocate(l4_addr_t offset, l4_size_t size, unsigned rights) = 0;

  unsigned long is_writable() const throw() { return _flags & Writable; }
  unsigned long can_cow() const throw() { return _flags & Cow_enabled; }
  unsigned long flags() const throw() { return _flags; }
  virtual ~Dataspace() {}

  unsigned long page_shift() const throw() { return _page_shift; }
  unsigned long page_size() const throw() { return 1UL << _page_shift; }

  virtual bool is_static() const throw() = 0;
  virtual long clear(unsigned long offs, unsigned long size) const throw();

protected:
  void size(unsigned long size) throw() { _size = size; }

public:
  unsigned long round_size() const throw()
  { return l4_round_size(size(), page_shift()); }
  bool check_limit(l4_addr_t offset) const throw()
  { return offset < round_size(); }
  bool check_range(l4_addr_t offset, unsigned long sz) const throw()
  { return offset < round_size() && size() - offset >= sz; }

public:
  int map(l4_addr_t offs, l4_addr_t spot, unsigned long flags,
          l4_addr_t min, l4_addr_t max, L4::Ipc::Snd_fpage &memory);
  int stats(L4Re::Dataspace::Stats &stats);
  //int copy_in(unsigned long dst_offs, Dataspace *src, unsigned long src_offs,
  //    unsigned long size);

  typedef Dma_space::Attributes Dma_attribs;
  virtual int dma_map(Dma_space *dma, l4_addr_t offset, l4_size_t *size,
                      Dma_attribs dma_attrs, Dma_space::Direction dir,
                      Dma_space::Dma_addr *dma_addr);
  virtual int dma_unmap(Dma_space *dma, l4_addr_t offset, l4_size_t size,
                        Dma_attribs dma_attrs, Dma_space::Direction dir);

  long op_map(L4Re::Dataspace::Rights rights,
              unsigned long offset, l4_addr_t spot,
              unsigned long flags, L4::Ipc::Snd_fpage &fp);

  long op_take(L4Re::Dataspace::Rights)
  {
    L4::cout << "Warning: using deprecated take() operation. Ignored.\n";
    return 0;
  }

  long op_release(L4Re::Dataspace::Rights)
  {
    L4::cout << "Warning: using deprecated release() operation. Ignored.\n";
    return 0;
  }

  long op_allocate(L4Re::Dataspace::Rights rights,
                   l4_addr_t offset, l4_size_t size)
  { return pre_allocate(offset, size, rights & 3); }

  long op_phys(L4Re::Dataspace::Rights,
               l4_addr_t offset, l4_addr_t &phys_addr,
               l4_size_t &phys_size)
  {
    phys_size = (l4_size_t)~0;
    L4Re::Dma_space::Dma_addr pa = phys_addr;
    int r = dma_map(0, offset, &phys_size, Dma_attribs::None,
                    L4Re::Dma_space::Bidirectional, &pa);
    if (r < 0)
      return r;
    phys_addr = pa;
    return r;
  }

  long op_copy_in(L4Re::Dataspace::Rights rights, l4_addr_t dst_offs,
                  L4::Ipc::Snd_fpage const &src_cap,
                  l4_addr_t src_offs, unsigned long sz);

  long op_info(L4Re::Dataspace::Rights rights, L4Re::Dataspace::Stats &s)
  {
    s.size = size();
    // only return writable if really writable
    s.flags = flags() & ~Writable;
    if ((rights & L4_CAP_FPAGE_W) && is_writable())
      s.flags |= Writable;
    return L4_EOK;
  }

  long op_clear(L4Re::Dataspace::Rights rights,
                l4_addr_t offset, unsigned long size)
  {
    if (   !(rights & L4_CAP_FPAGE_W)
        || !is_writable())
      return -L4_EACCESS;

    return clear(offset, size);
  }


private:
  unsigned long  _size;
  unsigned short _flags;
  unsigned char  _page_shift;
};

}



