/*****************************************************************************/
/**
 * \file
 * Descriptors for virtual hardware (under UX).
 * \ingroup l4_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
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
/*****************************************************************************/
#ifndef _L4_SYS_VHW_H
#define _L4_SYS_VHW_H

#include <l4/sys/types.h>
#include <l4/sys/kip.h>

/**
 * \defgroup l4_kip_vhw_api Fiasco-UX Virtual devices
 * \ingroup l4_kip_api
 * Virtual hardware devices, provided by Fiasco-UX.
 *
 * \includefile{l4/sys/vhw.h}
 */

/**
 * Type of device.
 * \ingroup l4_kip_vhw_api
 */
enum l4_vhw_entry_type {
  L4_TYPE_VHW_NONE,                        /**< None entry. */
  L4_TYPE_VHW_FRAMEBUFFER,                 /**< Framebuffer device. */
  L4_TYPE_VHW_INPUT,                       /**< Input device. */
  L4_TYPE_VHW_NET,                         /**< Network device. */
};

/**
 * Description of a device.
 * \ingroup l4_kip_vhw_api
 */
struct l4_vhw_entry {
  enum l4_vhw_entry_type type;             /**< Type of virtual hardware. */
  l4_uint32_t            provider_pid;     /**< Host PID of the VHW provider. */

  l4_addr_t              mem_start;        /**< Start of memory region. */
  l4_addr_t              mem_size;         /**< Size of memory region. */

  l4_uint32_t            irq_no;           /**< IRQ number. */
  l4_uint32_t            fd;               /**< File descriptor. */
};

/**
 * Virtual hardware devices description.
 * \ingroup l4_kip_vhw_api
 */
struct l4_vhw_descriptor {
  l4_uint32_t magic;                       /**< Magic. */
  l4_uint8_t  version;                     /**< Version of the descriptor. */
  l4_uint8_t  count;                       /**< Number of entries. */
  l4_uint8_t  pad1;                        /**< padding \internal. */
  l4_uint8_t  pad2;                        /**< padding \internal. */

  struct l4_vhw_entry descs[];             /**< Array of device descriptions. */
};

enum {
  L4_VHW_MAGIC = 0x56687765,
};

static inline struct l4_vhw_descriptor *
l4_vhw_get(l4_kernel_info_t *kip) L4_NOTHROW
{
  struct l4_vhw_descriptor *v
    = (struct l4_vhw_descriptor *)(((unsigned long)kip) + kip->vhw_offset);

  if (v->magic == L4_VHW_MAGIC)
    return v;

  return NULL;
}

static inline struct l4_vhw_entry *
l4_vhw_get_entry(struct l4_vhw_descriptor *v, int entry) L4_NOTHROW
{
  return v->descs + entry;
}

static inline struct l4_vhw_entry *
l4_vhw_get_entry_type(struct l4_vhw_descriptor *v, enum l4_vhw_entry_type t) L4_NOTHROW
{
  int i;
  struct l4_vhw_entry *e = v->descs;

  for (i = 0; i < v->count; i++, e++)
    if (e->type == t)
      return e;

  return NULL;
}

#endif /* ! _L4_SYS_VHW_H */
