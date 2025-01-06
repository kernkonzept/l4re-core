/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/re/util/cap_alloc>
#include <l4/re/cap_alloc>

#include <string.h>
#include <stdlib.h>
#include <l4/re/env>
#include <l4/crtn/initpriorities.h>

namespace Vfs_config {

using L4Re::Util::cap_alloc;
using ::memcpy;
using ::malloc;
using ::free;

inline
L4::Cap<L4Re::Mem_alloc> allocator()
{ return L4Re::Env::env()->mem_alloc(); }

inline int
load_module(char const *)
{ return -1; }

}

#include <l4/l4re_vfs/impl/ns_fs_impl.h>
#include <l4/l4re_vfs/impl/ro_file_impl.h>
#include <l4/l4re_vfs/impl/fd_store_impl.h>
#include <l4/l4re_vfs/impl/vcon_stream_impl.h>
#include <l4/l4re_vfs/impl/vfs_impl.h>
// must be the last
#include <l4/l4re_vfs/impl/default_ops_impl.h>
