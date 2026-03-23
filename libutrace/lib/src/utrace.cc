/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Martin Decky <martin.decky@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <version>
#include <stdexcept>
#include <bit>
#include <string>
#include <optional>
#include <array>
#include <l4/sys/capability>
#include <l4/sys/debugger>
#include <l4/sys/ktrace.h>
#include <l4/sys/cxx/consts>
#include <l4/sys/task>
#include <l4/re/error_helper>
#include <l4/re/env>
#include <l4/re/rm>
#include <l4/utrace/ring_buffer>
#include <l4/utrace/utrace>

#if defined(__cpp_lib_generator)
#include <generator>
#else
#include <list>
#endif

#if defined(__cpp_lib_start_lifetime_as)
#include <memory>
#endif

namespace utrace {

L4::Cap<L4::Debugger> Tracebuffer::_jdb(L4_BASE_DEBUGGER_CAP);

Tracebuffer::Tracebuffer(Factory &factory)
  : _buffer(factory.status(), factory.slots())
{}

void Tracebuffer::validate()
{
  auto ret = fiasco_tbuf_validate();
  if (!ret.label())
    throw std::invalid_argument("Tracebuffer JDB capability not valid.");

  if (!_jdb.validate().label())
    throw std::invalid_argument("Debugger JDB capability not valid.");
}

unsigned Tracebuffer::version()
{ return tracebuffer_version; }

std::endian Tracebuffer::endianness()
{ return std::endian::native; }

std::optional<Tracebuffer::Index_desc> Tracebuffer::index(unsigned idx)
{
  std::array<char, string_buffer_size> name;
  std::array<char, string_buffer_size> shortname;

  auto ret = _jdb->query_log_name(idx, name.data(), name.size(),
                                  shortname.data(), shortname.size());
  if (ret != 0)
    return std::nullopt;

  return Index_desc{idx, name.data(), shortname.data()};
}

#if defined(__cpp_lib_generator)

std::generator<Tracebuffer::Index_desc> Tracebuffer::indexes()
{
  unsigned const limit = l4_ktrace_tbuf_max;

  for (unsigned idx = 0; idx < limit; ++idx)
    if (auto desc = index(idx))
      co_yield *desc;
}

#else

std::vector<Tracebuffer::Index_desc> Tracebuffer::indexes()
{
  std::vector<Tracebuffer::Index_desc> vector;
  unsigned const limit = l4_ktrace_tbuf_max;

  for (unsigned idx = 0; idx < limit; ++idx)
    if (auto desc = index(idx))
      vector.push_back(*desc);

  return vector;
}

#endif

Tracebuffer &Tracebuffer::instance()
{
  static Tracebuffer singleton(Factory::instance());
  return singleton;
}

size_t Tracebuffer::items() const
{ return _buffer.items(); }

size_t Tracebuffer::dequeue(Item *items, size_t capacity, size_t burst,
                             Drop_policy policy, Sequence *drops)
{
  auto yield = [](size_t idle_cycles) -> bool
    {
      // This is prepared to implement passive waiting in the future.
      if (idle_cycles > yield_cycles)
        return true;

      return false;
    };

  return _buffer.dequeue(items, capacity, burst, policy, yield, drops);
}

Tracebuffer::Factory::Factory()
{
  _fpage_status = reserve(L4_PAGESIZE);
  _status = map_status(_fpage_status);

  if (_status->version() != tracebuffer_version)
    throw std::runtime_error("Unsupported tracebuffer version.");

  if (_status->alignment() != stable_cache_alignment)
    throw std::runtime_error("Unsupported tracebuffer cache alignment.");

  auto items = _status->items();
  Buffer::check_items(items);

  _fpage_slots = reserve(Slot::size(items));
  _slots = map_slots(_fpage_slots, items);
}

Tracebuffer::Factory::~Factory()
{
  auto *env = L4Re::Env::env();

  L4Re::chksys(env->task()->unmap(_fpage_slots, L4_FP_ALL_SPACES));
  L4Re::chksys(env->task()->unmap(_fpage_status, L4_FP_ALL_SPACES));

  L4Re::chksys(env->rm()->free_area(reinterpret_cast<l4_addr_t>(_slots)));
  L4Re::chksys(env->rm()->free_area(reinterpret_cast<l4_addr_t>(_status)));
}

Tracebuffer::Factory &Tracebuffer::Factory::instance()
{
  static Tracebuffer::Factory singleton;
  return singleton;
}

Tracebuffer::Status &Tracebuffer::Factory::status()
{ return *_status; }

Tracebuffer::Slot *Tracebuffer::Factory::slots()
{ return _slots; }

l4_fpage_t Tracebuffer::Factory::reserve(size_t size)
{
  auto *env = L4Re::Env::env();
  unsigned char order = std::bit_width(size - 1);

  l4_addr_t addr = 0;
  L4Re::chksys(
    env->rm()->reserve_area(&addr, size,
                            L4Re::Rm::F::Search_addr | L4Re::Rm::F::Kernel,
                            order),
    "Reserve area for tracebuffer.");

  return l4_fpage(addr, order, L4_FPAGE_RO);
}

Tracebuffer::Status *Tracebuffer::Factory::map_status(l4_fpage_t fpage)
{
  L4Re::chksys(fiasco_tbuf_map_status(&fpage),
               "Mapping tracebuffer status area.");
  void *ptr = reinterpret_cast<void *>(l4_fpage_memaddr(fpage));

#if defined(__cpp_lib_start_lifetime_as)
  return std::start_lifetime_as<Status>(ptr);
#else
  return reinterpret_cast<Status *>(ptr);
#endif
}

Tracebuffer::Slot *
Tracebuffer::Factory::map_slots(l4_fpage_t fpage, [[maybe_unused]] size_t items)
{
  L4Re::chksys(fiasco_tbuf_map_slots(&fpage), "Mapping tracebuffer slots.");
  void *ptr = reinterpret_cast<void *>(l4_fpage_memaddr(fpage));

#if defined(__cpp_lib_start_lifetime_as)
  return std::start_lifetime_as_array<Slot>(ptr, items);
#else
  return reinterpret_cast<Slot *>(ptr);
#endif
}

} // namespace utrace
