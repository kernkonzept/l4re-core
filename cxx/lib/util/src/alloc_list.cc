/*
 * (c) 2004-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
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

#include <l4/cxx/alloc.h>

namespace L4 {

  void Alloc_list::free( void *blk, unsigned long size )
  {
    Elem *n = reinterpret_cast<Elem*>(blk);
    Elem **c = &_free;
    while (*c) 
      {
	if (reinterpret_cast<char*>(*c) + (*c)->size == blk)
	  {
	    (*c)->size += size;
	    blk = 0;
	    break;
	  }
	  
	if (reinterpret_cast<char*>(*c) > blk)
	  break;
	
	c = &((*c)->next);
      }

    if (blk)
      {
        n->next = *c;
	n->size = size;
	*c = n;
      }

    while (*c && reinterpret_cast<char*>(*c)+(*c)->size == (char*)((*c)->next))
      {
	(*c)->size += (*c)->next->size;
	(*c)->next = (*c)->next->next;
      }
  }

  void *Alloc_list::alloc( unsigned long size )
  {
    if (!_free) 
      return 0;

    // best fit;
    Elem **min = 0;
    Elem **c = &_free;

    while(*c) 
      {
	if ((*c)->size >= size && (!min || (*min)->size > (*c)->size))
	  min = c;

	c = &((*c)->next);
      }

    if (!min)
      return 0;


    void *r;
    if ( (*min)->size > size )
      {
	r = reinterpret_cast<char*>(*min) + ((*min)->size - size);
	(*min)->size -= size;  
      }
    else
      {
	r = *min;
	*min = (*min)->next;
      }

    return r;
  }

}

