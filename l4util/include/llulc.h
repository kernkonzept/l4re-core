/*
 * (c) 2011 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
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
/*
 * Note, do _NOT_ use this lock unless you got advised to do so.
 */
#pragma once

#include <l4/sys/utcb.h>
#include <stddef.h>

__BEGIN_DECLS

struct l4lllock_struct_t;
typedef struct l4ullulock_struct_t l4ullulock_t;

#ifdef __cplusplus
#define DEFAULT_UTCB = l4_utcb()
#else
#define DEFAULT_UTCB
#endif

int l4ullulock_init(l4ullulock_t **t,
                    void *(*mem_alloc)(size_t x),
                    void  (*mem_free)(void *p),
                    l4_cap_idx_t (*cap_alloc)(void),
                    void (*cap_free)(l4_cap_idx_t c),
                    l4_cap_idx_t factory);
int l4ullulock_deinit(l4ullulock_t *t);
int l4ullulock_lock(l4ullulock_t *t, l4_utcb_t *u DEFAULT_UTCB);
int l4ullulock_unlock(l4ullulock_t *t, l4_utcb_t *u DEFAULT_UTCB);

__END_DECLS
