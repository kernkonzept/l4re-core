// vi:set ft=cpp: -*- Mode: C++ -*-
/**
 * \file
 * Virtualization interface
 */
/*
 * (c) 2018 Adam Lackorzynski <adam@l4re.org>
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
#pragma once

#include <l4/sys/task>

namespace L4 {

/**
 * Virtual machine host address space.
 * \ingroup l4_kernel_object_api
 *
 * L4::Vm is a specialisation of L4::Task, used for virtual machines.
 * For Arm, it offers a call to make the virtual GICC area available
 * to the VM.
 */
class Vm : public Kobject_t<Vm, Task, L4_PROTO_VM>
{
public:
  /*
   * Map the GIC's virtual GICC page to the task.
   *
   * \param vgicc_fpage   Flexpage that describes an area in the address space
   *                      of the destination task to map the vGICC page to.
   * \utcb{utcb}
   *
   * \return Syscall return tag.
   */
  l4_msgtag_t vgicc_map(l4_fpage_t const vgicc_fpage,
                        l4_utcb_t *utcb = l4_utcb()) noexcept
  { return l4_task_vgicc_map_u(cap(), vgicc_fpage, utcb); }

protected:
  Vm();

private:
  Vm(Vm const &);
  void operator = (Vm const &);
};

}
