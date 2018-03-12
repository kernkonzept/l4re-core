/**
 * \file
 * \brief  Simple list-based allocator. Taken from the Fiasco kernel.
 *
 * \date   Alexander Warg <aw11os.inf.tu-dresden.de>
 *         Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#ifndef L4UTIL_L4LA_H
#define L4UTIL_L4LA_H

#include <l4/sys/l4int.h>
#include <l4/sys/compiler.h>

typedef struct l4la_free_t_s
{
  struct l4la_free_t_s *next;
  l4_size_t            size;
} l4la_free_t;

#define L4LA_INITIALIZER  { 0 }

EXTERN_C_BEGIN

/** Add free memory to memory pool.
 * \param first   list identifier
 * \param block   address of unused memory block
 * \param size    size of memory block */
L4_CV void      l4la_free(l4la_free_t **first, void *block, l4_size_t size);

/** Allocate memory from pool.
 * \param first   list identifier
 * \param size    length of memory block to allocate
 * \param align   alignment */
L4_CV void*     l4la_alloc(l4la_free_t **first, l4_size_t size, unsigned align);

/** Show all list members.
 * \param first   list identifier */
L4_CV void      l4la_dump(l4la_free_t **first);

/** Init memory pool.
 * \param first   list identifier */
L4_CV void      l4la_init(l4la_free_t **first);

/** Show available memory in pool.
 * \param first   list identifier */
L4_CV l4_size_t l4la_avail(l4la_free_t **first);

EXTERN_C_END

#endif
