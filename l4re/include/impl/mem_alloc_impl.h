/**
 * \file
 * \brief  Memory allocator client stub implementation
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/re/mem_alloc>
#include <l4/re/dataspace>
#include <l4/re/error_helper>

#include <l4/sys/factory>


namespace L4Re
{

l4_ret_t
Mem_alloc::alloc(long size, L4::Cap<Dataspace> mem, unsigned long flags,
                 unsigned long align, l4_addr_t paddr,
                 char const *type) const noexcept
{
  L4::Cap<L4::Factory> f(cap());
  auto call = f->create(mem, L4Re::Dataspace::Protocol);
  call << l4_mword_t(size)
       << l4_umword_t(flags)
       << l4_umword_t(align);
  if (flags & Fixed_paddr)
    call << l4_umword_t(paddr);
  if (type)
    {
      // We have to prepend 'type='.
      static constexpr unsigned Prefix_len = 5;
      char type_buf[32]; // assuming that type length is limited
      auto len = __builtin_strlen(type);
      if (len > sizeof(type_buf) - Prefix_len - 1)
        return -L4_EINVAL; // name too long
      __builtin_memcpy(type_buf, "type=", Prefix_len);
      __builtin_memcpy(&type_buf[Prefix_len], type, len);
      type_buf[Prefix_len + len] = '\0';

      call << type_buf;
    }

  return l4_error(call);
}

};
