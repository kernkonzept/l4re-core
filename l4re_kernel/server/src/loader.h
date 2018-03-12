/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once
#include "region.h"
#include "globals.h"

#include <l4/re/env>
#include <l4/libloader/loader>
#include <l4/libloader/local_app_model>
#include <cstring>
#include <l4/re/l4aux.h>


class L4Re_stack
{
private:
  l4_addr_t _addr;
  unsigned long _size;
  char *sp;

public:
  void set_local_addr(l4_addr_t addr)
  { sp = (char *)addr + _size; }

  void set_target_stack(l4_addr_t addr, l4_size_t size)
  { _addr = addr; _size = size; }

  l4_size_t stack_size() const { return _size; }
  l4_addr_t target_addr() const { return _addr; }

  char *push_object(void const *src, unsigned long size)
  {
    sp -= size;
    memcpy(sp, src, size);
    return sp;
  }

  template<typename T>
  T *push(T const &v)
  { return reinterpret_cast<T*>(push_object(&v, sizeof(T))); }

  void const **push_local_ptr(void const *p)
  { return push(p); }

  void align(unsigned long size)
  {
    l4_addr_t p = l4_addr_t(sp);
    unsigned bits;
    for (bits = 0; (1UL << bits) <= size; ++bits)
      ;

    p &= ~0UL << bits;
    sp = (char *)p;
  }

  char const *ptr() const { return sp; }
  void ptr(char *p) { sp = p; }
};

struct L4Re_app_model : public Ldr::Base_app_model<L4Re_stack>
{
  typedef L4::Cap<L4Re::Dataspace> Const_dataspace;
  typedef L4::Cap<L4Re::Dataspace> Dataspace;

  L4::Cap<L4Re::Rm> _rm;

  L4Re_app_model(L4::Cap<L4Re::Rm> rm, void *) : _rm(rm) {}

  Dataspace alloc_ds(unsigned long size) const;

  static Const_dataspace open_file(char const *name);

  void prog_attach_ds(l4_addr_t addr, unsigned long size,
                      Const_dataspace ds, unsigned long offset,
                      unsigned flags, char const *what);

  static void copy_ds(Dataspace dst, unsigned long dst_offs,
                      Const_dataspace src, unsigned long src_offs,
                      unsigned long size);

  static bool all_segs_cow();

  l4_addr_t local_attach_ds(Const_dataspace ds, unsigned long size,
                            unsigned long offset) const;

  void local_detach_ds(l4_addr_t addr, unsigned long size) const;

  int prog_reserve_area(l4_addr_t *start, unsigned long size, unsigned flags, unsigned char align);

  Dataspace alloc_app_stack();

  void init_prog()
  {}

  static Const_dataspace reserved_area()
  { return L4::Cap<L4Re::Dataspace>::Invalid; }

  static Dataspace local_kip_ds()
  {
    return L4::Cap<L4Re::Dataspace>(Global::l4re_aux->kip_ds);
  }

  static L4::Cap<void> local_kip_cap()
  { return local_kip_ds(); }

  static L4::Cap<void> prog_kip_ds()
  {
    return L4::Cap<L4Re::Dataspace>(Global::l4re_aux->kip_ds);
  }

  void const *generate_l4aux(char const *) const
  { return Global::l4re_aux; }

  void extra_elf_auxv();
  void push_envp();
  void push_argv();

  L4Re::Env *add_env();
  void start_prog(L4Re::Env const *env);
};

typedef Ldr::Local_app_model<L4Re_app_model> L4Re_x_app_model;

class Loader
{
public:
  bool start(L4::Cap<L4Re::Dataspace> bin, Region_map *rm, l4re_aux_t *aux);
  bool __start(L4::Cap<L4Re::Dataspace> bin, Region_map *rm);
  virtual bool launch(L4::Cap<L4Re::Dataspace> bin, L4::Cap<L4Re::Rm>) = 0;
  virtual ~Loader() {}
};
