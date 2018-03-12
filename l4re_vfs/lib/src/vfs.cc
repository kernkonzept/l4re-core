/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
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
#include <l4/re/util/cap_alloc>
#include <cstring>
#include <cstdlib>
#include <l4/re/env>

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
#include <l4/l4re_vfs/impl/vfs_api_impl.h>
#include <l4/l4re_vfs/impl/vfs_impl.h>
// must be the last
#include <l4/l4re_vfs/impl/default_ops_impl.h>
