/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/bid_config.h>
#include <l4/sys/types.h>
#include <l4/sys/ipc.h>
#include <l4/sys/assert.h>
#include <l4/sys/factory.h>
#include <l4/sys/capability>
#include <l4/sys/cxx/ipc_epiface>
#include <l4/sys/factory>
#include <l4/sys/err.h>

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

extern "C" void cov_print(void) __attribute__((weak));

l4_kernel_info_t *l4_info;

Mem_man iomem;

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
void
map_kip(Answer *answer)
{
#ifdef CONFIG_MMU
  answer->snd_fpage(reinterpret_cast<l4_umword_t>(l4_info), L4_LOG2_PAGESIZE,
                    L4_FPAGE_RX, true);
#else
  answer->snd_addr(reinterpret_cast<l4_umword_t>(l4_info));
#endif
}

static
void
new_client(Answer *answer)
{
  // The kernel passed a Sigma0 IPC gate with an IPC label 4<<4 (0x40) to Moe.
  // Actually IPC labels < L4_BASE_CAPS_LAST are reserved for Moe.

  static l4_cap_idx_t _next_gate = L4_BASE_CAPS_LAST + L4_CAP_OFFSET;

  if ((_next_gate >> L4_CAP_SHIFT) & ~Region::Owner_mask)
    return answer->error(L4_ENOMEM);

  l4_factory_create_gate_u(L4_BASE_FACTORY_CAP, _next_gate,
                           L4_BASE_THREAD_CAP, (_next_gate >> L4_CAP_SHIFT) << 4,
                           answer->utcb);
  answer->snd_fpage(l4_obj_fpage(_next_gate, 0, L4_CAP_FPAGE_RWS));
  _next_gate += L4_CAP_OFFSET;
}

static
void
map_free_page(unsigned order, l4_umword_t client_id, Answer *answer)
{
  if (order < L4_PAGESHIFT)
    return answer->error(L4_EINVAL);

  unsigned long addr = Mem_man::ram()->alloc_first(order, client_id);
  if (addr != ~0UL)
    answer->snd_fpage(addr, order, L4_FPAGE_RWX, true);
  else
    answer->error(L4_ENOMEM);
}


static
void
map_mem(l4_fpage_t fp, Memory_type fn, l4_umword_t client_id, Answer *answer)
{
  unsigned long send_addr = l4_fpage_memaddr(fp);
  unsigned send_order = l4_fpage_order(fp);

  // Check if send_addr is correctly aligned to send_order since the kernel
  // will otherwise truncate the send address. Fail in case it is not aligned.
  if (l4_trunc_size(send_addr, send_order) != send_addr)
    return answer->error(L4_EINVAL);

  // Isolation is only enforced at page granularity. Deny smaller requests.
  if (send_order < L4_PAGESHIFT)
    return answer->error(L4_EINVAL);

  bool cached = true;
  L4_fpage_rights mem_flags;
  unsigned long addr = ~0UL;

  switch (fn)
    {
    case Ram:
      mem_flags = L4_FPAGE_RWX;
      if (Mem_man::ram()->alloc(Region::start_order(send_addr, send_order,
                                                    client_id)))
        addr = send_addr;
      break;
    case Io_mem:
      cached = false;
      /* fall through */
    case Io_mem_cached:
      {
        // there is no first-come, first-serve for IO memory
        Region r = Region::start_order(send_addr, send_order);
        Region const *p = iomem.find(r);
        if (p)
          {
            addr = r.start();
            mem_flags = p->rights();
          }
        break;
      }
    default:
      return answer->error(L4_EINVAL);
    }

  if (addr == ~0UL)
    return answer->error(L4_ENOMEM);

  answer->snd_fpage(addr, send_order, mem_flags, cached);
}

/* handler for page fault requests */
static
void
handle_page_fault(l4_umword_t client_id, unsigned words, l4_utcb_t *utcb,
                  Answer *answer)
{
  if (words < 1)
    return answer->error(L4_EMSGTOOSHORT);
  if (words > 2)
    return answer->error(L4_EMSGTOOLONG);

  unsigned long pfa = l4_utcb_mr_u(utcb)->mr[0] & ~7UL;
  bool inst_fetch = l4_utcb_mr_u(utcb)->mr[0] & 4;
  bool write = l4_utcb_mr_u(utcb)->mr[0] & 2;

  L4_fpage_rights dr = inst_fetch ? (write ? L4_FPAGE_RWX : L4_FPAGE_RX)
                                  : (write ? L4_FPAGE_RW : L4_FPAGE_RO);

  L4_fpage_rights rights;
  Region r = Region::start_order(l4_trunc_page(pfa), L4_PAGESHIFT, client_id, dr);
  if (Mem_man::ram()->alloc_get_rights(r, &rights))
    return answer->snd_fpage(r.start(), L4_LOG2_PAGESIZE, rights, true);

  if (debug_warnings)
    L4::cout << PROG_NAME ": Page fault, did not find page " << r << "\n";

  answer->error(L4_ENOMEM);
}

static
void handle_service_request(l4_umword_t rights, unsigned words, l4_utcb_t *utcb,
                            Answer *answer)
{
  if (words < 1)
    return answer->error(L4_EMSGTOOSHORT);
  if (words > 1)
    return answer->error(L4_EMSGTOOLONG);

  if (!(rights & L4_CAP_FPAGE_S))
    return answer->error(L4_EPERM);

  if (static_cast<long>(l4_utcb_mr_u(utcb)->mr[0]) != L4_PROTO_SIGMA0)
    return answer->error(L4_ENODEV);

  new_client(answer);
}

static
void handle_sigma0_request(l4_umword_t client_id, unsigned words, l4_utcb_t *utcb,
                           Answer *answer)
{
  if (words < 1)
    return answer->error(L4_EMSGTOOSHORT);

  l4_msg_regs_t const *const m = l4_utcb_mr_u(utcb);
  if (!SIGMA0_IS_MAGIC_REQ(m->mr[0]))
    return answer->error(L4_ENOSYS);

  unsigned long id = m->mr[0] & SIGMA0_REQ_ID_MASK;

  switch (id)
    {
    case SIGMA0_REQ_ID_DEBUG_DUMP:
      if (words > 1)
        answer->error(L4_EMSGTOOLONG);
      else
        {
#ifndef NDEBUG
          Mem_man::Tree::Node_allocator alloc;
          L4::cout << PROG_NAME": Memory usage: a total of "
            << Page_alloc_base::total()
            << " bytes are in the memory pool\n"
            << "  allocated "
            << alloc.total_objects() - alloc.free_objects()
            << " of " << alloc.total_objects() << " objects\n"
            << "  these are "
            << (alloc.total_objects() - alloc.free_objects())
            * alloc.object_size
            << " of " << alloc.total_objects() * alloc.object_size
            << " bytes\n";
          dump_all();
#endif
          answer->error(0);
        }
      break;

    case SIGMA0_REQ_ID_FPAGE_RAM:
      if (words < 2)
        answer->error(L4_EMSGTOOSHORT);
      else if (words > 2)
        answer->error(L4_EMSGTOOLONG);
      else
        map_mem(l4_fpage_t{m->mr[1]}, Ram, client_id, answer);
      break;

    case SIGMA0_REQ_ID_FPAGE_IOMEM:
      if (words < 2)
        answer->error(L4_EMSGTOOSHORT);
      else if (words > 2)
        answer->error(L4_EMSGTOOLONG);
      else
        map_mem(l4_fpage_t{m->mr[1]}, Io_mem, client_id, answer);
      break;

    case SIGMA0_REQ_ID_FPAGE_IOMEM_CACHED:
      if (words < 2)
        answer->error(L4_EMSGTOOSHORT);
      else if (words > 2)
        answer->error(L4_EMSGTOOLONG);
      else
        map_mem(l4_fpage_t{m->mr[1]}, Io_mem_cached, client_id, answer);
      break;

    case SIGMA0_REQ_ID_KIP:
      if (words > 1)
        answer->error(L4_EMSGTOOLONG);
      else
        map_kip(answer);
      break;

    case SIGMA0_REQ_ID_FPAGE_ANY:
      if (words < 2)
        answer->error(L4_EMSGTOOSHORT);
      else if (words > 2)
        answer->error(L4_EMSGTOOLONG);
      else
        map_free_page(l4_fpage_order(l4_fpage_t{m->mr[1]}), client_id, answer);
      break;

    case SIGMA0_REQ_ID_COV:
      if (words > 1)
        answer->error(L4_EMSGTOOLONG);
      else if (cov_print)
        cov_print();
      else
        answer->error(L4_ENOSYS);
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
  l4_msgtag_t tag;

  l4_utcb_t *utcb = l4_utcb();
  Answer answer(utcb);

  /* now start serving the subtasks */
  for (;;)
    {
      l4_umword_t label;
      tag = l4_ipc_wait(utcb, &label, L4_IPC_NEVER);
      if (0)
        L4::cout << PROG_NAME << ": rcv: " << tag << "\n";
      while (!l4_msgtag_has_error(tag))
        {
          l4_umword_t pfa;
          if (debug_warnings)
            pfa = l4_utcb_mr_u(utcb)->mr[0];
          l4_umword_t client_rights = label & (L4_CAP_FPAGE_W | L4_CAP_FPAGE_S);
          l4_umword_t client_id = label >> 4;

          /* we received a paging request here */
          /* handle the sigma0 protocol */

          if (debug_ipc)
            {
              l4_umword_t d1 = l4_utcb_mr_u(utcb)->mr[0];
              l4_umword_t d2 = l4_utcb_mr_u(utcb)->mr[1];
              L4::cout << PROG_NAME": received " << tag << " d1=" << L4::hex
                       << d1 << " d2=" << d2 << L4::dec << " from client="
                       << client_id << '\n';
            }

          switch (tag.label())
            {
            case L4_PROTO_SIGMA0:
              handle_sigma0_request(client_id, tag.words(), utcb, &answer);
              break;
            case L4::Meta::Protocol:
              {
                L4::Ipc::Detail::Meta_svr<Sigma0> dummy;
                answer.tag
                  = L4::Ipc::Msg::dispatch_call<L4::Meta::Rpcs>(&dummy, utcb,
                                                                tag, client_id);
              }
              break;
            case L4::Factory::Protocol:
              handle_service_request(client_rights, tag.words(), utcb, &answer);
              break;
            case L4_PROTO_PAGE_FAULT:
              handle_page_fault(client_id, tag.words(), utcb, &answer);
              break;
            case L4_PROTO_IO_PAGE_FAULT:
              handle_io_page_fault(client_id, tag.words(), utcb, &answer);
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
                           << " from client=" << L4::dec << client_id << '\n';
                  if (tag.is_page_fault())
                    Mem_man::ram()->dump();
                }

              l4_assert(!tag.is_exception());
            }

          if (debug_ipc)
            L4::cout << PROG_NAME": sending d1="
                     << L4::hex << l4_utcb_mr_u(utcb)->mr[0]
                     << " d2=" << l4_utcb_mr_u(utcb)->mr[1]
                     << " msg=" << answer.tag << L4::dec
                     << " to thread=" << client_id << '\n';

          /* send reply and wait for next message */
          tag = l4_ipc_reply_and_wait(utcb, answer.tag, &label,
                                      L4_IPC_SEND_TIMEOUT_0);
        }
    }
}
