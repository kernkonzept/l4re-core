/**
 * \file
 * \brief Alloc list
 */
/*
 * (c) 2004-2009 Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */

#ifndef L4_CXX_ALLOC_H__
#define L4_CXX_ALLOC_H__

namespace L4 {

  /**
   * \brief A simple list-based allocator.
   * \ingroup cxx_api 
   */
  class Alloc_list
  {
  public:
    Alloc_list() : _free(0) {}
    Alloc_list( void *blk, unsigned long size ) : _free(0) 
    { free( blk, size ); }

    void free( void *blk, unsigned long size );
    void *alloc( unsigned long size );
    
  private:
    struct Elem 
    {
      Elem *next;
      unsigned long size;
    };

    Elem *_free;
  };
};

#endif // L4_CXX_ALLOC_H__

