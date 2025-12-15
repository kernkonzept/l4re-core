/*
 * Copyright (C) 2021-2022 Kernkonzept GmbH.
 * Author(s): Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

/**
 * \file
 * Implementation of _dl_find_object() which is referenced by libgcc as of gcc
 * version 12.
 */

#include <sys/cdefs.h>

/*
 * Unfortunately uclibc headers don't work well with C++ code. <link.h> pulls
 * socket-specific functions (e.g. ntohl()) carrying the __THROW attribute but
 * the corresponding hidden alias function does not carry it. All this doesn't
 * matter here because we only need <link.h> for a structure definition so just
 * disable the function attributes.
 *
 * Also undef _LIBC so that <link.h> doesn't drag <tls.h> and friends.
 */
#undef __THROW
#define __THROW
#undef __NTH
#define __NTH(fct) fct
#undef _LIBC

#include <stddef.h>
#include <elf.h>

#include <link.h>

struct Dl_find_object
{
  unsigned long long dlfo_flags;
  void *dlfo_map_start;
  void *dlfo_map_end;
  struct link_map *dlfo_link_map;
  void *dlfo_eh_frame;
  unsigned long long __dlfo_reserved[7];
};

struct Shared_data
{
  // Address we are looking for.
  void *address;
  // Pointer to the result -- set by callback() function on success.
  Dl_find_object *dlfo;
  // Set to true on success.
  int found;
};

/**
 * This callback function is called once per shared object.
 *
 * \param info  Info for shared object.
 * \param size  Size of the structure referred by `info`.
 * \param data  Shared data with _dl_find_object(), See Shared_data.
 *
 * This function looks for `Shared_data.address` in all shared objects and, in
 * case the corresponding object was found, saves certain information to
 * `Shared_data.dlfo` as required by _dl_find_object:
 * - Start and end address of the shared object.
 * - Address of the PT_GNU_EH_FRAME section.
 *
 * \retval 0    Continue iteration.
 * \retval != 0 Stop iteration.
 */
static int
callback(struct dl_phdr_info *info, size_t size, void *data)
{
  if (size < sizeof(struct dl_phdr_info))
    return 1; // sanity check

  auto *d = static_cast<Shared_data *>(data);

  void *eh_frame = nullptr;
  int set_address = false;
  for (unsigned i = 0; i < info->dlpi_phnum; ++i)
    {
      uintmax_t beg = info->dlpi_addr + info->dlpi_phdr[i].p_vaddr;
      uintmax_t end = beg + info->dlpi_phdr[i].p_memsz;
      if ((uintmax_t)d->address >= beg && (uintmax_t)d->address < end)
        {
          set_address = true;
          d->found = true;
          d->dlfo->dlfo_map_start = (void*)beg;
          d->dlfo->dlfo_map_end   = (void*)end;
        }

      if (info->dlpi_phdr[i].p_type == PT_GNU_EH_FRAME)
        eh_frame = (void*)beg;
  }

  if (set_address)
    {
      d->dlfo->dlfo_eh_frame = eh_frame;
      return 1;
    }

  return 0;
}

/**
 * See the documentation in
 * https://www.gnu.org/software/libc/manual/html_node/Dynamic-Linker-Introspection.html
 */
extern "C" int _dl_find_object(void *address, Dl_find_object *result) noexcept;
int _dl_find_object(void *address, Dl_find_object *result) noexcept
{
  Shared_data data = { address, result, false };
  dl_iterate_phdr(callback, &data);

  return data.found ? 0 : -1;
}
