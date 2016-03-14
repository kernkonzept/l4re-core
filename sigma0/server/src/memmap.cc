/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/sys/types.h>
#include <l4/sys/ipc.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/factory.h>
#include <l4/sys/capability>
#include <l4/sys/cxx/ipc_epiface>
#include <l4/sys/factory>

#include <l4/sigma0/sigma0.h>

#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>
#include <l4/cxx/l4types.h>

#include "globals.h"
#include "page_alloc.h"
#include "mem_man.h"
#include "memmap.h"
#include "memmap_internal.h"
#include "ioports.h"


l4_addr_t	 mem_high;
l4_kernel_info_t *l4_info;
l4_addr_t        tbuf_status;

Mem_man iomem;


enum Requests
{
  None,
  Map_free_page,
  Map_kip,
  Map_tbuf,
  Map_ram,
  Map_iomem,
  Map_iomem_cached,
  Debug_dump,
};

enum Memory_type { Ram, Io_mem, Io_mem_cached };

void dump_all()
{
  L4::cout << PROG_NAME": Dump of all resource maps\n"
           << "RAM:------------------------\n";
  Mem_man::ram()->dump();
  L4::cout << "IOMEM:----------------------\n";
  iomem.dump();
  dump_io_ports();
}

static
void map_kip(Answer *a)
{
  a->snd_fpage((l4_umword_t) l4_info, L4_LOG2_PAGESIZE, L4_FPAGE_RX, true);
}

static
void new_client(l4_umword_t, Answer *a)
{
  static l4_cap_idx_t _next_gate = L4_BASE_CAPS_LAST + L4_CAP_OFFSET;
  l4_factory_create_gate_u(L4_BASE_FACTORY_CAP, _next_gate,
                           L4_BASE_THREAD_CAP, (_next_gate >> L4_CAP_SHIFT) << 4, a->utcb);
  a->snd_fpage(l4_obj_fpage(_next_gate, 0, L4_FPAGE_RWX));
  _next_gate += L4_CAP_SIZE;
  return;
}

static
void map_tbuf(Answer *a)
{
  if (tbuf_status != 0x00000000 && tbuf_status != ~0UL)
    {
      a->snd_fpage(tbuf_status, L4_LOG2_PAGESIZE, L4_FPAGE_RW, true);
    }
}

static
void map_free_page(unsigned size, l4_umword_t t, Answer *a)
{
  unsigned long addr;
  addr = Mem_man::ram()->alloc_first(1UL << size, t);
  if (addr != ~0UL)
    {
      a->snd_fpage(addr, size, L4_FPAGE_RWX, true);

      if (t < root_taskno) /* sender == kernel? */
	a->do_grant(); /* kernel wants page granted */
    }
  else
    a->error(L4_ENOMEM);
}


static
void map_mem(l4_fpage_t fp, Memory_type fn, l4_umword_t t, Answer *an)
{
  Mem_man *m;
  unsigned mem_flags;
  bool cached = true;
  unsigned long addr;
  switch (fn)
    {
    case Ram:
      m = Mem_man::ram();
      mem_flags = L4_FPAGE_RWX;
      addr = m->alloc(Region::bs(fp.raw & ~((1UL << 12) - 1),
                                 1UL << l4_fpage_size(fp), t));
      break;
    case Io_mem:
      cached = false;
      /* fall through */
    case Io_mem_cached:
      mem_flags = L4_FPAGE_RW;
      // there is no first-come, first-serve for IO memory
      addr = fp.raw & ~((1UL << 12) - 1);
      break;
    default:
      an->error(L4_EINVAL);
      return;
    }

  if (addr == ~0UL)
    {
      an->error(L4_ENOMEM);
      return;
    }

  /* the Fiasco kernel makes the page non-cachable if the frame
   * address is greater than mem_high */
  an->snd_fpage(addr, l4_fpage_size(fp), mem_flags, cached);

  return;
}

/* handler for page fault fault requests */
static
void
handle_page_fault(l4_umword_t t, l4_utcb_t *utcb, Answer *answer)
{
  unsigned long pfa = l4_utcb_mr_u(utcb)->mr[0] & ~3UL;


  unsigned long addr
    = Mem_man::ram()->alloc(Region::bs(l4_trunc_page(pfa), L4_PAGESIZE, t));

  if (addr != ~0UL)
    {
      answer->snd_fpage(addr, L4_LOG2_PAGESIZE, L4_FPAGE_RWX, true);
      return;
    }
  else if (debug_warnings)
    L4::cout << PROG_NAME": Page fault, did not find page at "
             << L4::hex << pfa << " for " << L4::dec << t << "\n";

  answer->error(L4_ENOMEM);
}

static
void handle_service_request(l4_umword_t t, l4_utcb_t *utcb, Answer *answer)
{
  if ((long)l4_utcb_mr_u(utcb)->mr[0] != L4_PROTO_SIGMA0)
    {
      answer->error(L4_ENODEV);
      return;
    }
  new_client(t, answer);
}

static
void handle_sigma0_request(l4_umword_t t, l4_utcb_t *utcb, Answer *answer)
{
  if (!SIGMA0_IS_MAGIC_REQ(l4_utcb_mr_u(utcb)->mr[0]))
    {
      answer->error(L4_ENOSYS);
      return;
    }

  switch (l4_utcb_mr_u(utcb)->mr[0] & 0x0f0)
    {
    case SIGMA0_REQ_ID_DEBUG_DUMP:
	{
	  Mem_man::Tree::Node_allocator alloc;
	  L4::cout << PROG_NAME": Memory usage: a total of "
	    << Page_alloc_base::total()
	    << " byte are in the memory pool\n"
	    << "  allocated "
	    << alloc.total_objects() - alloc.free_objects()
	    << " of " << alloc.total_objects() << " objects\n"
	    << "  this are "
	    << (alloc.total_objects() - alloc.free_objects())
	    * alloc.object_size
	    << " of " << alloc.total_objects() * alloc.object_size
	    << " byte\n";
          dump_all();
	  answer->error(0);
	}
      break;
    case SIGMA0_REQ_ID_TBUF:
      map_tbuf(answer);
      break;
    case SIGMA0_REQ_ID_FPAGE_RAM:
      map_mem((l4_fpage_t&)l4_utcb_mr_u(utcb)->mr[1], Ram, t, answer);
      break;
    case SIGMA0_REQ_ID_FPAGE_IOMEM:
      map_mem((l4_fpage_t&)l4_utcb_mr_u(utcb)->mr[1], Io_mem, t, answer);
      break;
    case SIGMA0_REQ_ID_FPAGE_IOMEM_CACHED:
      map_mem((l4_fpage_t&)l4_utcb_mr_u(utcb)->mr[1], Io_mem_cached, t, answer);
      break;
    case SIGMA0_REQ_ID_KIP:
      map_kip(answer);
      break;
    case SIGMA0_REQ_ID_FPAGE_ANY:
      map_free_page(l4_fpage_size(*(l4_fpage_t*)(&l4_utcb_mr_u(utcb)->mr[1])), t, answer);
      break;
    case SIGMA0_REQ_ID_NEW_CLIENT:
      new_client(t, answer);
      break;
    default:
      answer->error(L4_ENOSYS);
      break;
    }
}

namespace {

class Sigma0 :
  public L4::Kobject_t<Sigma0, L4::Factory, L4_PROTO_SIGMA0>
{};

}

/* PAGER dispatch loop */
void
pager(void)
{
  l4_umword_t t;
  l4_msgtag_t tag;

  l4_utcb_t *utcb = l4_utcb();
  Answer answer(utcb);

  /* now start serving the subtasks */
  for (;;)
    {
      tag = l4_ipc_wait(utcb, &t, L4_IPC_NEVER);
#if 0
	  L4::cout << "w: res=" << L4::MsgDope(result)
	           << " tag=" << L4::dec << l4_msgtag_label(tag) << '\n';
#endif
      //L4::cout << PROG_NAME << ": rcv: " << tag << "\n";
      while (!l4_msgtag_has_error(tag))
	{
          l4_umword_t pfa;
          if (debug_warnings)
            pfa = l4_utcb_mr_u(utcb)->mr[0];
	  t >>= 4;
	  /* we received a paging request here */
	  /* handle the sigma0 protocol */

	  if (debug_ipc)
            {
              l4_umword_t d1 = l4_utcb_mr_u(utcb)->mr[0];
              l4_umword_t d2 = l4_utcb_mr_u(utcb)->mr[1];
              L4::cout << PROG_NAME": received " << tag << " d1=" << L4::hex
                       << d1 << " d2=" << d2 << L4::dec << " from thread="
                       << t << '\n';
            }

	  switch(tag.label())
	    {
	    case L4_PROTO_SIGMA0:
	      handle_sigma0_request(t, utcb, &answer);
	      break;
	    case L4::Meta::Protocol:
              answer.tag = L4::Ipc::Msg::dispatch_call<L4::Meta::Rpcs>((L4::Ipc::Detail::Meta_svr<Sigma0> *)0, utcb, tag, t);
	      break;
	    case L4::Factory::Protocol:
	      handle_service_request(t, utcb, &answer);
	      break;
	    case L4_PROTO_PAGE_FAULT:
	      handle_page_fault(t, utcb, &answer);
	      break;
	    case L4_PROTO_IO_PAGE_FAULT:
	      handle_io_page_fault(t, utcb, &answer);
	      break;
	    default:
	      answer.error(L4_EBADPROTO);
	      break;
	    }

	  if (answer.failed())
	    {
	      if (debug_warnings)
                {
                  L4::cout << PROG_NAME": can't handle label=" << L4::dec
                           << l4_msgtag_label(tag)
                           << " d1=" << L4::hex << pfa
                           << " d2=" << l4_utcb_mr_u(utcb)->mr[1]
                           << " from thread=" << L4::dec << t << '\n';
	          if (tag.is_page_fault())
		    Mem_man::ram()->dump();
                }

	      if (tag.is_exception())
                enter_kdebug("s1");
	    }

	  if (debug_ipc)
	    L4::cout << PROG_NAME": sending d1=" << L4::hex << l4_utcb_mr_u(utcb)->mr[0]
	      << " d2=" << l4_utcb_mr_u(utcb)->mr[1]
	      << " msg=" << answer.tag << L4::dec
	      << " to thread=" << t << '\n';

	  /* send reply and wait for next message */
	  tag = l4_ipc_reply_and_wait(utcb, answer.tag, &t, L4_IPC_SEND_TIMEOUT_0);
#if 0
	  L4::cout << "rplw: res=" << L4::MsgDope(result)
	           << " tag=" << L4::dec << l4_msgtag_label(tag) << '\n';
#endif

	}
    }
}

