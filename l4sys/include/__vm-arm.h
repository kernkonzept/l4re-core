// vi:set ft=cpp: -*- Mode: C++ -*-
/**
 * \file
 * Virtualization interface
 */
/*
 * (c) 2018 Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/task>

namespace L4 {

class Vm : public Kobject_t<Vm, Task, L4_PROTO_VM>
{
public:
  /**
   * Map the GIC virtual CPU interface page to the task in case Fiasco
   * detected a GIC version 2.
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
