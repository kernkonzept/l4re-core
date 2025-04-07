/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   written by Alexandre Oliva <aoliva@redhat.com>
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

In addition to the permissions in the GNU Lesser General Public
License, the Free Software Foundation gives you unlimited
permission to link the compiled version of this file with other
programs, and to distribute those programs without any restriction
coming from the use of this file.  (The GNU Lesser General Public
License restrictions do apply in other respects; for example, they
cover modification of the file, and distribution when not linked
into another program.)

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, see <http://www.gnu.org/licenses/>.  */

#ifdef __FDPIC__

#include <sys/types.h>
#include <link.h>

/* This file is to be compiled into crt object files, to enable
   executables to easily self-relocate.  */

/* Compute the runtime address of pointer in the range [p,e), and then
   map the pointer pointed by it.  */
static __always_inline void ***
reloc_range_indirect (void ***p, void ***e,
		      const struct elf32_fdpic_loadmap *map)
{
  while (p < e)
    {
      if (*p != (void **)-1)
	{
	  void *ptr = __reloc_pointer (*p, map);

	  if (ptr != (void *)-1)
	    {
	      unsigned long off = ((unsigned long)ptr & 3) * 8;
	      unsigned long *pa = (unsigned long *)((unsigned long)ptr & -4);
	      unsigned long v2;
	      void *pt;

	      if (off)
		{
		  unsigned long v0, v1;
#ifdef __XTENSA_EB__
		  v0 = pa[1]; v1 = pa[0];
		  v2 = (v1 >> (32 - off)) | (v0 << off);
#else /* __XTENSA_EL__ */
		  v0 = pa[0]; v1 = pa[1];
		  v2 = (v0 << (32 - off)) | (v1 >> off);
#endif
		  pt = (void *)((v1 << (32 - off)) | (v0 >> off));
		}
	      else
		pt = *(void**)ptr;
	      pt = __reloc_pointer (pt, map);
	      if (off)
		{
		  unsigned long v = (unsigned long)pt;
#ifdef __XTENSA_EB__
		  pa[0] = (v2 << (32 - off)) | (v >> off);
		  pa[1] = (v << (32 - off)) | (v2 >> off);
#else /* __XTENSA_EL__ */
		  pa[0] = (v2 >> (32 - off)) | (v << off);
		  pa[1] = (v >> (32 - off)) | (v2 << off);
#endif
		}
	      else
		*(void**)ptr = pt;
	    }
	}
      p++;
    }
  return p;
}

/* Call __reloc_range_indirect for the given range except for the last
   entry, whose contents are only relocated.  It's expected to hold
   the GOT value.  */
attribute_hidden void*
__self_reloc (const struct elf32_fdpic_loadmap *map,
	      void ***p, void ***e)
{
  p = reloc_range_indirect (p, e-1, map);

  if (p >= e)
    return (void*)-1;

  return __reloc_pointer (*p, map);
}

#endif /* __FDPIC__ */
