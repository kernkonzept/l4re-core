/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#pragma once

#include <sys/cdefs.h>
#include <l4/sys/types.h>

__BEGIN_DECLS

void libsig_be_add_thread(l4_cap_idx_t t);
void libsig_be_set_dbg_name(const char *name);

__END_DECLS
