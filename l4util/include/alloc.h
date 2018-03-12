/*!
 * \file
 * \brief  Allocator using a bit-array
 *
 * \date   09/14/2004
 * \author Jork Loeser <jork.loeser@inf.tu-dresden.de>
 *
 */
/*
 * (c) 2004-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#ifndef __UTIL_INCLUDE_ALLOC_H_
#define __UTIL_INCLUDE_ALLOC_H_
#include <l4/sys/l4int.h>
#include <l4/util/bitops.h>
#include <l4/sys/compiler.h>

EXTERN_C_BEGIN

typedef struct {
    int base, count, next_elem;
    l4_umword_t *bits;
} l4util_alloc_t;

#define L4UTIL_ALLOC_BITS_SIZE (8 * sizeof(l4_umword_t))

L4_CV l4util_alloc_t *l4util_alloc_init(int count, int base);
L4_CV int l4util_alloc_avail(l4util_alloc_t *alloc, int elem);
L4_CV int l4util_alloc_occupy(l4util_alloc_t *alloc, int elem);
L4_CV int l4util_alloc_alloc(l4util_alloc_t *alloc);
L4_CV int l4util_alloc_free(l4util_alloc_t *alloc, int elem);

EXTERN_C_END
#endif
