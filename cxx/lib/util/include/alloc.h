/**
 * \file
 * \brief Alloc list
 */
/*
 * (c) 2004-2009 Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

namespace L4 {

  /**
   * \brief A simple list-based allocator.
   * \ingroup cxx_api
   */
  class Alloc_list
  {
  public:
    Alloc_list() : _free(0) {}
    Alloc_list(void *blk, unsigned long size) : _free(0)
    { free( blk, size); }

    void free(void *blk, unsigned long size);
    void *alloc(unsigned long size);

  private:
    struct Elem
    {
      Elem *next;
      unsigned long size;
    };

    Elem *_free;
  };
}
