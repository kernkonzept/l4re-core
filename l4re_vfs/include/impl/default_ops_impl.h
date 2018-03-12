// vi:ft=cpp
/*
 * Copyright (C) 2016 Kernkonzept GmbH.
 * Author(s): Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */
#include <l4/cxx/static_container>

namespace {
struct Vfs_init
{
  // The Static_containers are used to prevent automatic destruction during
  // program shutdown. At least the `vfs` object must never be destructued
  // because any later attempt to do any kind of file-descriptor access in
  // the program would crash, and we could not be sure that the destructor
  // would really be executed after each possible operation using files or file
  // descriptors.
  cxx::Static_container<Vfs> vfs;

  // The Static_containers below are just for providing ordering. The factories
  // must be initialized after the `vfs` object.
  cxx::Static_container<L4Re::Vfs::File_factory_t<L4Re::Dataspace, L4Re::Core::Ro_file> > ro_file;
  cxx::Static_container<L4Re::Vfs::File_factory_t<L4Re::Namespace, L4Re::Core::Ns_dir> > ns_dir;
  cxx::Static_container<L4Re::Vfs::File_factory_t<L4::Vcon, L4Re::Core::Vcon_stream> > vcon_stream;

  Vfs_init()
  {
    vfs.construct();
    __rtld_l4re_env_posix_vfs_ops = vfs;
    ns_dir.construct();
    auto ns_ptr = cxx::ref_ptr(ns_dir.get());
    vfs->register_file_factory(ns_ptr);
    ns_ptr.release(); // prevent deletion of static object

    ro_file.construct();
    auto ro_ptr = cxx::ref_ptr(ro_file.get());
    vfs->register_file_factory(ro_ptr);
    ro_ptr.release(); // prevent deletion of static object

    vcon_stream.construct();
    auto vcon_ptr = cxx::ref_ptr(vcon_stream.get());
    vfs->register_file_factory(vcon_ptr);
    vcon_ptr.release(); // prevent deletion of static object
  }
};

static Vfs_init __vfs_init __attribute__((init_priority(INIT_PRIO_VFS_INIT)));

};
