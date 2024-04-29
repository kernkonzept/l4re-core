/*
 * (c) 2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "sched_proxy.h"
#include "globals.h"
#include "debug.h"

#include <algorithm>
#include <l4/cxx/minmax>
#include <l4/re/env>
#include <l4/sys/scheduler>

//#include <cstdio>

static l4_umword_t kernel_cpu_max;
static l4_umword_t kernel_sched_classes;

Dyn_cpu_set::Dyn_cpu_set(Moe::Q_alloc *q)
: Bitmap_base(q->alloc(bit_buffer_bytes(kernel_cpu_max), alignof(word_type)))
{}

Dyn_cpu_set::~Dyn_cpu_set()
{
  void *buf = bit_buffer();
  Moe::Malloc_container::from_ptr(buf)->free(buf);
}

Dyn_cpu_set& Dyn_cpu_set::operator=(Dyn_cpu_set const &other)
{
  memcpy(bit_buffer(), other.bit_buffer(), bit_buffer_bytes(kernel_cpu_max));
  return *this;
}

void Dyn_cpu_set::update(unsigned offset, l4_umword_t mask)
{
  for (unsigned i = 0; i < sizeof(mask) * 8; i++)
    if (offset + i < kernel_cpu_max)
      bit(offset + i, mask & (1UL << i));
}

void Dyn_cpu_set::clear_all()
{
  memset(bit_buffer(), 0, bit_buffer_bytes(kernel_cpu_max));
}

void Dyn_cpu_set::set_all()
{
  memset(bit_buffer(), 0xff, bit_buffer_bytes(kernel_cpu_max));
}

Dyn_cpu_set& Dyn_cpu_set::operator&=(Dyn_cpu_set const &other)
{
  word_type *ours = reinterpret_cast<word_type*>(bit_buffer());
  word_type *theirs = reinterpret_cast<word_type*>(other.bit_buffer());

  for (long i = 0; i < words(kernel_cpu_max); i++)
    ours[i] &= theirs[i];

  return *this;
}

l4_sched_cpu_set_t Dyn_cpu_set::operator&(l4_sched_cpu_set_t const &s) const
{
  unsigned const base = s.offset();
  unsigned constexpr map_bits = sizeof(s.map) * 8;

  // Cap the granularity at a value where the `limit` calculation below does
  // not overflow the range of an unsigned int. Strictly speaking we would have
  // to calculate
  //
  //   sizeof(unsigned) * 8 - log2(map_bits + 24)
  //
  // with the 24 being the bits coming from `s.offset(). To keep things simple,
  // we assume map_bits == 64. In any case we will never have that many CPUs.
  unsigned const gran = cxx::min<unsigned>(s.granularity(),
                                           sizeof(unsigned) * 8 - 6 - 1);
  unsigned limit =
    cxx::min<unsigned>(kernel_cpu_max,
                       base + (1U << gran) * map_bits);

  l4_sched_cpu_set_t ret = l4_sched_cpu_set(base, 0, 0);
  unsigned first = base;
  for (unsigned off = base; off < limit; ++off)
    {
      if (bit(off) && s.map & (1UL << ((off - base) >> gran)))
        {
          if (! ret.map)
            {
              // First valid CPU found - adapt offset and end of interval
              first = off;
              ret.set(0, first);
              limit = cxx::min<unsigned>(limit, first + map_bits);
            }
          ret.map |= 1UL << (off - first);
        }
    }
  return ret;
}


Sched_proxy::List Sched_proxy::_list;

Sched_proxy::Sched_proxy(Moe::Q_alloc *q) :
  Icu(1, &_scheduler_irq),
  _cpus(q), _real_cpus(q), _cpu_mask(q),
  _prio_offset(0), _prio_limit(0)
{
  _cpu_mask.set_all();
  rescan_cpus_and_classes();
  _list.push_front(this);
}

void
Sched_proxy::rescan_cpus_and_classes()
{
  _real_cpus.clear_all();

  l4_sched_cpu_set_t c;
  c.gran_offset = 0;

  auto sched = L4Re::Env::env()->scheduler();
  while (c.offset() < kernel_cpu_max)
    {
      c.map = 0;
      int e = l4_error(sched->info(nullptr, &c, nullptr));
      if (e < 0)
        break;

      _real_cpus.update(c.offset(), c.map);
      c.set(0, c.offset() + sizeof(l4_umword_t) * 8);
    }

  _cpus = _real_cpus;
  _cpus &= _cpu_mask;
}

int
Sched_proxy::info(l4_umword_t *cpu_max, l4_sched_cpu_set_t *cpus,
                  l4_umword_t *sched_classes)
{
  *cpu_max = kernel_cpu_max;
  unsigned char g = cpus->granularity() & (sizeof(l4_umword_t) * 8 - 1);
  l4_umword_t offs = cpus->offset() & (~0UL << g);
  if (offs >= kernel_cpu_max)
    return -L4_ERANGE;

  cpus->map = 0;
  unsigned b = 0;
  for (unsigned i = offs; i < kernel_cpu_max && b < sizeof(l4_umword_t) * 8;)
    {
      if (_cpus[i])
	cpus->map |= 1UL << b;

      ++i;

      if (!(i & ~(~0UL << g)))
	++b;
    }

  *sched_classes = kernel_sched_classes;

  return L4_EOK;
}


int
Sched_proxy::run_thread(L4::Cap<L4::Thread> thread, l4_sched_param_t const &sp)
{
  l4_sched_param_t s = sp;
  s.prio = std::min<l4_umword_t>(sp.prio + _prio_offset, _prio_limit);
  s.affinity = _cpus & sp.affinity;
  if (0)
    {
      printf("loader[%p] run_thread: sp.m=%lx sp.o=%u sp.g=%u\n",
             this, sp.affinity.map, sp.affinity.offset(),
             sp.affinity.granularity());
      printf("loader[%p]                                      "
             " s.m=%lx  s.o=%u  s.g=%u\n",
             this, s.affinity.map, s.affinity.offset(),
             s.affinity.granularity());
    }
  return l4_error(L4Re::Env::env()->scheduler()->run_thread(thread, s));
}

int
Sched_proxy::idle_time(l4_sched_cpu_set_t const &, l4_kernel_clock_t &)
{ return -L4_ENOSYS; }


L4::Cap<L4::Thread>
Sched_proxy::received_thread(L4::Ipc::Snd_fpage const &fp)
{
  if (!fp.cap_received())
    return L4::Cap<L4::Thread>::Invalid;

  return L4::Cap<L4::Thread>(Rcv_cap << L4_CAP_SHIFT);
}

void
Sched_proxy::restrict_cpus(Dyn_cpu_set const &cpus)
{
  _cpu_mask = cpus;
  _cpus = _real_cpus;
  _cpus &= _cpu_mask;
}


class Cpu_hotplug_server :
  public L4::Irqep_t<Cpu_hotplug_server, Moe::Server_object>
{
public:
  void handle_irq()
  {
    for (auto i : Sched_proxy::_list)
      {
        i->rescan_cpus_and_classes();
        i->hotplug_event();
      }
  }

  Cpu_hotplug_server()
  {
    l4_sched_cpu_set_t c = l4_sched_cpu_set(0, 0, 0);
    int e = l4_error(L4Re::Env::env()->scheduler()->info(&kernel_cpu_max, &c,
                                                         &kernel_sched_classes));
    if (e < 0)
      {
        Err(Err::Fatal).printf("Could not query scheduler: %d\n", e);
        return;
      }

    L4::Cap<L4::Irq> irq = object_pool.cap_alloc()->alloc<L4::Irq>();
    if (!irq)
      {
        Err(Err::Fatal).printf("Could not allocate capability for CPU hotplug\n");
        return;
      }

    if (l4_error(L4::Cap<L4::Factory>(L4_BASE_FACTORY_CAP)->create(irq)) < 0)
      {
        Err(Err::Fatal).printf("Could not allocate IRQ for CPU hotplug\n");
        return;
      }

    if (l4_error(irq->bind_thread(L4::Cap<L4::Thread>(L4_BASE_THREAD_CAP), l4_umword_t(this))) < 0)
      {
        Err(Err::Fatal).printf("Could not attach to CPU hotplug IRQ\n");
        return;
      }

    if (l4_error(L4Re::Env::env()->scheduler()->bind(0, irq)) < 0)
      {
        Err(Err::Fatal).printf("Could not bind CPU hotplug IRQ to scheduler\n");
        return;
      }
  }
};

static Cpu_hotplug_server _cpu_hotplug_server;
