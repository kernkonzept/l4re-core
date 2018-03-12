/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "exception.h"

#include <l4/sys/utcb.h>
#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>

Exception_handler *Exception_handler::handlers[0x100];

bool Exception_handler::do_handle_exception(l4_umword_t /*t*/,
    l4_msgtag_t &tag)
{
#if 0
  l4_utcb_t *u = l4_utcb();
  unsigned trap = u->exc.trapno;
  static Exception_handler std_handler;
  Exception_handler *hdl = &std_handler;
  if (trap < 0x100 && handlers[trap])
    hdl = handlers[trap];

  if (hdl->handle(t, u))
    {
      tag = l4_msgtag(0, L4_UTCB_EXCEPTION_REGS_SIZE, 0, 0);
      return true;
    }
  else
    {
      tag = l4_msgtag(0, 0, 0, 0);
      return false;
    }
#endif
  tag = l4_msgtag(0, 0, 0, 0);
  return false;
}

bool Exception_handler::handle(l4_umword_t t, void *s)
{
#if defined ARCH_x86 || defined ARCH_amd64
  l4_exc_regs_t e = *l4_utcb_exc_u((l4_utcb_t*)s);
  L4::cerr << t << ": Unhandled exception #" << e.trapno << '\n';
  L4::cerr << t << " PFA " << L4::hex << e.pfa << " EIP " << e.ip 
    << " ESP " << e.sp << " Flags " << e.flags << '\n';
#else
  (void)t;
#endif
#if 0
    << t << " EAX " << L4::hex << e->eax << " EBX " << e->ebx
    << " ECX " << e->ecx << " EDX " << e->edx << '\n' 
    << t << " ESI " << e->esi << " EDI " << e->edi << '\n';
#endif

  for (unsigned i = 0; i< 24; ++i)
    L4::cout << '[' << i << "] " << L4::hex << l4_utcb_mr_u((l4_utcb_t*)s)->mr[i] << '\n';
  return false;
}

