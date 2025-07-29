/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Carsten Weinhold <weinhold@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include "mem_man.h"
#include "globals.h"

#include <l4/cxx/iostream>
#include <l4/sys/assert.h>

Mem_man Mem_man::_ram;

Region const *
Mem_man::find(Region const &r, bool force) const
{
  if (!r.valid())
    return 0;

  Tree::Const_iterator n = _tree.find(r);
  if (n == _tree.end())
    return 0;

  if (n->contains(r) || force)
    return &(*n);

  return 0;
}

bool
Mem_man::add(Region const &r)
{
  /* try to merge with prev region */
  Region rs = r;
  if (rs.start() > 0)
    {
      rs.start(rs.start()-1);

      Tree::Node n = _tree.find_node(rs);
      if (n && n->owner() == r.owner() && n->rights() == r.rights())
        {
          r.start(n->start());
          int err = _tree.remove(*n);
          if (err < 0)
            {
              L4::cout << "err=" << err << " dump:\n";
              dump();
              l4_assert(!"BUG");
            }
        }
    }

  /* try to merge with next region */
  rs = r;
  if (rs.end() + 1 != 0)
    {
      rs.end(rs.end()+1);

      Tree::Node n = _tree.find_node(rs);
      if (n && n->owner() == r.owner() && n->rights() == r.rights())
        {
          r.end(n->end());
          int err = _tree.remove(*n);
          if (err < 0)
            {
              L4::cout << "err=" << err << " dump:\n";
              dump();
              l4_assert(!"BUG");
            }
        }
    }

  /* do throw away regions owned by myself */
  if (r.owner() == sigma0_taskno)
    return true;

  while (_tree.insert(r).second == -_tree.E_nomem)
    if (!ram()->morecore())
      {
        if (debug_errors)
          L4::cout << PROG_NAME": Out of memory\n";
        return false;
      }

  return true;
}

bool
Mem_man::add_free(Region const &r)
{
  if (!r.valid())
    return true;

  // calculate the combined set of all overlapping regions within the tree
  while (1)
    {
      Tree::Node n = _tree.find_node(r);

      if (!n)
        break;

      if (n->start() < r.start())
        r.start(n->start());
      if (n->end() > r.end())
        r.end(n->end());

      int err = _tree.remove(*n);
      if (err < 0)
        {
          L4::cout << "err=" << err << " dump:\n";
          dump();
          l4_assert(!"BUG");
        }
    }

  return add(r);
}

bool
Mem_man::alloc_from(Region const *r2, Region const &_r)
{
  if (!_r.valid())
    return false;

  Region r(_r);
  if (r2->owner() && r2->owner() != r.owner())
    return false;

  if (r2->owner() == r.owner() && r2->rights() == r.rights())
    return true;

  if (r == *r2)
    {
      if (0)
        {
          L4::cout << "dump " << r << " " << *r2 << "\n";
          dump();
        }
      int err = _tree.remove(*r2);
      if (err < 0)
        {
          L4::cout << "err=" << err << " dump:\n";
          dump();
          l4_assert(!"BUG");
        }
      return add(r);
    }

  Region r2_orig = *r2;

  if (r.start() == r2->start())
    {
      r2->start(r.end() + 1);
      if (0)
        L4::cout << "move start to " << *r2 << '\n';
    }
  else if (r.end() == r2->end())
    {
      r2->end(r.start() - 1);
      if (0)
        L4::cout << "shrink end to " << *r2 << '\n';
    }
  else
    {
      Region const nr(r.end() + 1, r2->end(), r2->owner(), r2->rights());
      r2->end(r.start() - 1);
      if (0)
        L4::cout << "split to " << *r2 << "; " << nr << '\n';

      if (nr.valid() && !add(nr))
        {
          r2->restore_range_from(r2_orig);
          return false;
        }
    }

  if (!add(r))
    {
      r2->restore_range_from(r2_orig);
      return false;
    }

  return true;
}

unsigned long
Mem_man::alloc(Region const &r)
{
  if (!r.valid())
    return ~0UL;
  Region const *r2 = find(r);
  if (!r2)
    return ~0UL;

  if (0)
    L4::cout << "alloc_from(" << *r2 << ", " << r << ")\n";
  if (!alloc_from(r2, r))
    return ~0UL;

  return r.start();
}

/**
 * Allocate region from its containing region and inherit its rights.
 *
 * @param[in] r        Region to allocate. Its rights are the necessary minimal
 *                     rights of the containing region.
 * @param[out] rights  Address of the variable that will receive the full rights
 *                     of the containing region.
 *
 * @retval true   The region was allocated.
 * @retval false  The region was not allocated.
 */
bool
Mem_man::alloc_get_rights(Region const &r, L4_fpage_rights *rights)
{
  Region q = r;
  auto const *p = find(q);
  if (!p)
    return false;
  if ((p->rights() & q.rights()) != q.rights())
    return false;

  *rights = p->rights();
  q.rights(p->rights());
  return alloc_from(p, q);
}

/**
 * Add a reserved memory region into the map.
 *
 * \return true if the region could be reserved, or false in the case
 * of an error. Note, any error is considered fatal and might create
 * an inconsistent / incorrect memory map.
 */
bool
Mem_man::reserve(Region const &r)
{
  if (!r.valid())
    return false;

  for (;;)
    {
      auto r2 = _tree.find(r);

      //Region const *r2 = find(r, true);
      if (r2 == _tree.end())
        {
          if (0)
            L4::cout << this << ": ADD: " << r << "\n";
          return add(r);
        }

      if (0)
        L4::cout << this << ":  RESERVE: " << r << " from " << (*r2) << "\n";

      if (r2->owner() && r2->owner() != r.owner())
        return false;

      // allow exact matches to update owner and rights of the reserved region
      if (*r2 == r && (r2->owner() != r.owner() || r2->rights() != r.rights()))
        {
          int err = _tree.remove(*r2);
          if (err < 0)
            {
              L4::cout << "err=" << err << " dump:\n";
              dump();
              l4_assert(!"BUG");
            }
          return add(r);
        }

      // contained region will not get any updated rights
      if (r2->contains(r) && r2->owner() == r.owner())
        return true;

      if (r2->contains(r))
        {
          Region r2_orig = *r2;

          if (r2->start() == r.start())
            r2->start(r.end() + 1);
          else
            {
              Region const nr(r.end() + 1, r2->end(), r2->owner(), r2->rights());
              r2->end(r.start() - 1);
              if (0)
                L4::cout << this << ": ADDnr: " << nr << "\n";
              // FIXME: we could avoid the merge code for this add
              //        because this region is per definition not mergeable
              if (nr.valid() && !add(nr))
                {
                  r2->restore_range_from(r2_orig);
                  return false;
                }
            }

          if (0)
            L4::cout << this << ": ADD: " << r << "\n";

          if (!add(r))
            {
              r2->restore_range_from(r2_orig);
              return false;
            }
          return true;
        }

      if (r.contains(*r2))
        {
          if (0)
            L4::cout << this << ": REMOVE: " << *r2 << "\n";
          _tree.remove(*r2);
          continue;
        }

      if (r2->start() < r.start())
        {
          if (r2->owner())
            r.start(r2->end() + 1);
          else
            r2->end(r.start() - 1);
        }
      else
        {
          if (r2->owner())
            r.end(r2->start() - 1);
          else
            r2->start(r.end() + 1);
        }
    }
}


bool
Mem_man::morecore()
{
  Tree::Item_type *n = 0;
  for (Tree::Rev_iterator i = _tree.rbegin(); i != _tree.rend(); ++i)
    {
      if (i->owner())
        continue;

      l4_addr_t st = l4_round_page(i->start());

      if (st < i->end() && i->end() - st >= L4_PAGESIZE - 1)
        {
          n = &(*i);
          break;
        }
    }

  if (!n)
    {
      if (debug_memory_maps)
        L4::cout << PROG_NAME": morecore did not find more free memory\n";
      return false;
    }

  Region a = Region::bs(l4_round_page(n->end() - L4_PAGESIZE - 1),
                        L4_PAGESIZE, sigma0_taskno);

  Page_alloc_base::free(reinterpret_cast<void*>(a.start()));

  alloc_from(n, a);

  if (debug_memory_maps)
    L4::cout << PROG_NAME": morecore: total=" << Page_alloc_base::total() / 1024
             << " KB avail=" << Page_alloc_base::allocator()->avail() / 1024
             << " KB: added " << a << '\n';
  return true;
}

unsigned long
Mem_man::alloc_first(unsigned long size, unsigned owner)
{
  Tree::Item_type *n = 0;

  for (Tree::Iterator i = _tree.begin(); i != _tree.end(); ++i)
    {
      if (i->owner())
        continue;

      // wrap-around?
      if ((i->start() + size - 1) < i->start())
        continue;

      l4_addr_t st = (i->start() + size - 1) & ~(size - 1);
      if (0)
        L4::cout << "test: " << (void*)st << " - " << i->end() << '\n';

      if (st < i->end() && i->end() - st >= size - 1)
        {
          n = &(*i);
          break;
        }
    }

  if (!n)
    return ~0UL;

  Region a = Region::bs((n->start() + size - 1) & ~(size - 1), size, owner);

  if (!alloc_from(n, a))
    return ~0UL;

  return a.start();
}

void
Mem_man::dump()
{
  for (Tree::Iterator i = _tree.begin(); i != _tree.end(); ++i)
    L4::cout << *i << '\n';
}
