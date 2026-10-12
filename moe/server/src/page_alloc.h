/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include "factory_config.h"

#include <l4/cxx/exceptions>
#include <l4/cxx/static_vector>

#include <cstddef>

class Dbg;

/**
 * Base page allocator.
 *
 * Manages the physical memory pages available to Moe.
 */
class Single_page_alloc_base
{
public:
  enum Nothrow { nothrow };

  struct Config
  {
    Moe::Factory_config::Region_list_view regions = {nullptr, 0};

    Config() = default;
    constexpr Config(Moe::Factory_config::Region_list_view regions)
    : regions(regions)
    {}
  };

  static Config default_mem_cfg;

protected:
  Single_page_alloc_base();

public:
  static void *_alloc_max(size_t min, size_t *max, size_t align,
                          size_t granularity, Config cfg);
  static void *_alloc(Nothrow, size_t size, size_t align,
                      Config cfg = default_mem_cfg);
  static void *_alloc(size_t size, size_t align, Config cfg = default_mem_cfg)
  {
    void *r = _alloc(nothrow, size, align, cfg);
    if (!r)
      throw L4::Out_of_memory();
    return r;
  }
  static void _free(void *p, size_t size);
  static void _free_internal(void *p, size_t size);
  static size_t _avail();

  static void _add_mem(void *p, size_t size);

  /**
   * Upper bound on the number of allocator banks.
   *
   * Bounded so that the banks can live in a fixed array, which matters because
   * they have to be known before there is an allocator to allocate them from.
   * Platforms have a handful of memory areas at most; if there are ever more
   * large holes than this, the cheapest ones are given up again.
   */
  static constexpr unsigned Max_ram_banks = 8;

  /**
   * A contiguous span of physical memory with an allocator of its own.
   *
   * The buddy allocator holds approximately two bytes metadata per L4_PAGESIZE
   * of the memory range it spans. When the physical address space has large
   * holes this wastes a lot of memory. To avoid that we split the physical
   * address space into its contiguous chunks, with a separate allocator each.
   */
  using Ram_bank = Moe::Mem_range;

  using Ram_bank_list =
    Moe::Mem_region_list<Ram_bank, Single_page_alloc_base::Max_ram_banks>;

  /**
   * Total metadata needed for `banks`, as one chunk.
   */
  static size_t _metadata_bytes(Ram_bank_list const &banks);

  /**
   * Set up one allocator per bank, carving their metadata out of one chunk.
   *
   * \param banks          The banks, sorted and non-overlapping.
   * \param metadata_addr  Chunk for the metadata of all banks.
   * \param metadata_size  Size of that chunk, at least _metadata_bytes().
   */
  static void _init(Ram_bank_list const &banks, unsigned char *metadata_addr,
                    size_t metadata_size);

#ifndef NDEBUG
  static void _dump_free(Dbg &dbg);
#endif

  static bool can_free;
};

class Single_page_unique_ptr
{
private:
  void *_p = 0;
  size_t _s;

public:
  void *release()
  {
    void *p = _p;
    _p = 0;
    return p;
  }

  void reset(void *n = 0, size_t size = 0)
  {
    if (n == _p)
      return;

    void *p = _p;
    size_t s = _s;

    _p = n;
    if (n)
      _s = size;

    if (p)
      Single_page_alloc_base::_free(p, s);
  }

  size_t size() const { return _s; }

  void *get() const { return _p; }
  void *operator * () const { return _p; }

  Single_page_unique_ptr() = default;
  Single_page_unique_ptr(void *p, size_t size) : _p(p), _s(size) {}

  ~Single_page_unique_ptr()
  { reset(); }

  Single_page_unique_ptr &operator = (Single_page_unique_ptr &&o)
  {
    if (this == &o)
      return *this;

    reset(o._p, o._s);
    o._p = 0;
    return *this;
  }
};
