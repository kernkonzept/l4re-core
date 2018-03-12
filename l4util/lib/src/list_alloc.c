/**
 * \file   l4util/lib/src/list_alloc.c
 * \brief  Simple list-based allocator. Taken from the Fiasco kernel.
 *
 * \author Alexander Warg <aw11@os.inf.tu-dresden.de>
 *         Frank Mehnert <fm3@os.inf.tu-dresden.de>
 */
/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/sys/l4int.h>
#include <l4/sys/kdebug.h>
#include <l4/util/list_alloc.h>

#include <stdio.h>
#include <assert.h>

#define ALIGN_MASK (sizeof(l4la_free_t) - 1)

#define DEBUG
#ifdef DEBUG

static void 
__check_overlap(l4la_free_t **first, void *b, l4_size_t s)
{
  l4la_free_t *c = *first;
  for (; c; c = c->next)
    {
      l4_addr_t x_s = (l4_addr_t)b;
      l4_addr_t x_e = x_s + s;
      l4_addr_t b_s = (l4_addr_t)c;
      l4_addr_t b_e = b_s + c->size;

      if (   (x_s >= b_s && x_s <  b_e)
	  || (x_e >  b_s && x_e <= b_e)
	  || (b_s >= x_s && b_s <  x_e)
	  || (b_e >  x_s && b_e <= x_e))
	{
	  printf("trying to free memory that is already free: \n"
	         "  [%lx-%lx) overlaps [%lx-%lx)\n",
		 x_s, x_e, b_s, b_e );
	  enter_kdebug("l4la");
	}
    }
}

static void 
__sanity_check_list(l4la_free_t **first, char const *func, char const *info)
{
  l4la_free_t *c = *first;
  for (;c ; c = c->next)
    {
      if (c->next)
	{
	  if (c >= c->next)
	    {
	      printf("%s: %s(%s): list order violation\n",
		     __FILE__, func, info);
	      enter_kdebug("l4la");
	    }

	  if (((l4_addr_t)c) + c->size > (l4_addr_t)c->next)
	    {
	      printf("%s: %s(%s): overlapping blocks\n",
		     __FILE__, func, info);
	      enter_kdebug("l4la");
	    }
	}
    }
}

#else

static inline void
__check_overlap(l4la_free_t **first, void *b, l4_size_t s) {};

static inline void
__sanity_check_list(l4la_free_t **first, char const *f, char const *i) {};

#endif

static void
__merge(l4la_free_t **first)
{
  __sanity_check_list(first, __FUNCTION__, "entry");

  l4la_free_t *c = *first;
  while (c && c->next)
    {
      l4_addr_t f_start = (l4_addr_t)c;
      l4_addr_t f_end   = f_start + c->size;
      l4_addr_t n_start = (l4_addr_t)c->next;
      
      if (f_end == n_start)
	{
	  c->size += c->next->size;
	  assert(c->size >= sizeof(l4la_free_t));
	  c->next = c->next->next;
	  continue;
	}

      c = c->next;
    }

  __sanity_check_list(first, __FUNCTION__, "exit");
}

L4_CV void
l4la_free(l4la_free_t **first, void *block, l4_size_t size)
{
  l4la_free_t **c = first;
  l4la_free_t *next = 0;

  __sanity_check_list(first, __FUNCTION__, "entry");
  __check_overlap(first, block, size);

  size = (size + ALIGN_MASK) & ~ALIGN_MASK;

  if (*c)
    {
      while (*c && *c < (l4la_free_t*)block)
	c = &(*c)->next;

      next = *c;
    }

  assert(*c != block);
  *c = (l4la_free_t*)block;
  assert(*c != next);

  (*c)->next = next;
  (*c)->size = size;

  assert((!next) || ((l4_addr_t)(*c) + size <= (l4_addr_t)next));

  __merge(first);

  __sanity_check_list(first, __FUNCTION__, "exit");
}

L4_CV void *
l4la_alloc(l4la_free_t **first, l4_size_t size, unsigned align)
{
  void *ret = 0;
  l4_addr_t almask = (1 << align) - 1;
  l4la_free_t **c = first;
  void *b = 0;

  __sanity_check_list(first, __FUNCTION__, "entry");

  size = (size + ALIGN_MASK) & ~ALIGN_MASK;

  for (; *c; c=&(*c)->next)
    {
      l4_addr_t n_start = (l4_addr_t)(*c);
      l4_addr_t a_start;
      l4_addr_t a_size;
      
      if ((*c)->size < size)
	continue;
      
      if (!(n_start & almask))
	{
	  if ((*c)->size >= size)
	    {
	      if ((*c)->size == size)
		{
		  b = *c;
		  *c = (*c)->next;
		  ret = b;
		  goto done;
		}
	      
	      l4la_free_t *m = (l4la_free_t*)(n_start + size);
	      m->next = (*c)->next;
	      m->size = (*c)->size - size;
	      assert(m->size >= sizeof(l4la_free_t));
	      b = *c;
	      *c = m;
	      ret = b;
	      goto done;
	    }
	  
	  continue;
	}

      a_start = (n_start & ~almask) + 1 + almask;
      if (a_start - n_start >= (*c)->size)
	continue;

      a_size = (*c)->size - a_start + n_start;

      if (a_size >= size)
	{
	  if (a_size == size)
	    {
	      (*c)->size -= a_size;
	      ret = (void*)a_start;
	      goto done;
	    }
	  l4la_free_t *m = (l4la_free_t*)(a_start + size);
	  m->next = (*c)->next;
	  m->size = a_size - size;
	  assert(m->size >= sizeof(l4la_free_t));
	  (*c)->size -= a_size;
	  assert((*c)->size >= sizeof(l4la_free_t));
	  (*c)->next = m;
	  ret = (void*)a_start;
	  goto done;
	}
    }

done:
  return ret;
}

L4_CV l4_size_t
l4la_avail(l4la_free_t **first)
{
  __sanity_check_list(first, __FUNCTION__, "entry");

  l4la_free_t *c = *first;
  l4_addr_t a = 0;
  while (c)
    {
      a += c->size;
      c = c->next;
    }

  return a;
}

L4_CV void
l4la_dump(l4la_free_t **first)
{
  printf("List_alloc [first=%p]\n", *first);
  l4la_free_t *c = *first;
  for (;c && c!=c->next ; c = c->next)
    printf("  mem_block_t [this=%p size=0x%lx (%ldkB) next=%p]\n", c,
	   (l4_addr_t)c->size, 
	   (l4_addr_t)(c->size+1023)/1024, c->next);

  if (c && c == c->next)
    printf("  BUG: loop detected\n");
}

L4_CV void
l4la_init(l4la_free_t **first)
{
  *first = 0;
}
