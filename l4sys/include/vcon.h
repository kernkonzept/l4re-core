/**
 * \file
 * Virtual console interface.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
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
#pragma once

#include <l4/sys/ipc.h>

/**
 * \defgroup l4_vcon_api Virtual Console
 * \ingroup  l4_kernel_object_api
 * Virtual console for simple character based input and output.
 *
 * The interrupt for read events is provided by the virtual key interrupt.
 *
 * \includefile{l4/sys/vcon.h}
 *
 * See L4::Vcon for the C++ interface.
 */

/**
 * Send data to virtual console.
 * \ingroup l4_vcon_api
 *
 * \param vcon  Vcon object.
 * \param buf   Pointer to data buffer.
 * \param size  Size of buffer in bytes.
 *
 * \return Syscall return tag
 *
 * \note Size must not exceed #L4_VCON_WRITE_SIZE, a proper value of the
 *       `size` parameter is NOT checked.
 */
L4_INLINE l4_msgtag_t
l4_vcon_send(l4_cap_idx_t vcon, char const *buf, int size) L4_NOTHROW;

/**
 * \ingroup l4_vcon_api
 * \copybrief L4::Vcon::send
 * \param vcon  Capability index of the Vcon object.
 * \copydetails L4::Vcon::send
 */
L4_INLINE l4_msgtag_t
l4_vcon_send_u(l4_cap_idx_t vcon, char const *buf, int size, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Write data to virtual console.
 * \ingroup l4_vcon_api
 *
 * \param vcon  Vcon object.
 * \param buf   Pointer to data buffer.
 * \param size  Size of buffer in bytes.
 *
 * \retval <0   Error.
 * \retval >=0  Number of bytes written to the virtual console
 */
L4_INLINE long
l4_vcon_write(l4_cap_idx_t vcon, char const *buf, int size) L4_NOTHROW;

/**
 * \ingroup l4_vcon_api
 * \copybrief L4::Vcon::write
 * \param vcon  Capability index of the vcon object.
 * \copydetails L4::Vcon::write
 */
L4_INLINE long
l4_vcon_write_u(l4_cap_idx_t vcon, char const *buf, int size, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Size constants.
 * \ingroup l4_vcon_api
 */
enum L4_vcon_size_consts
{
  /** Maximum size that can be written with one l4_vcon_write call. */
  L4_VCON_WRITE_SIZE = (L4_UTCB_GENERIC_DATA_SIZE - 2) * sizeof(l4_umword_t),
  /** Maximum size that can be read with one l4_vcon_read* call. */
  L4_VCON_READ_SIZE  = (L4_UTCB_GENERIC_DATA_SIZE - 1) * sizeof(l4_umword_t),
};

/**
 * Read data from virtual console.
 * \ingroup l4_vcon_api
 *
 * \param      vcon    Vcon object.
 * \param[out] buf     Pointer to data buffer.
 * \param      size    Size of buffer in bytes.
 *
 * \retval <0      Error code.
 * \retval >size   If more bytes are to read, `size` bytes are in the
 *                 buffer `buf`.
 * \retval <=size  Number of bytes read.
 *
 * \note Size must not exceed #L4_VCON_READ_SIZE.
 */
L4_INLINE int
l4_vcon_read(l4_cap_idx_t vcon, char *buf, int size) L4_NOTHROW;

/**
 * \ingroup l4_vcon_api
 * \copybrief L4::Vcon::read
 * \param vcon  Capability index of the vcon object.
 * \copydetails L4::Vcon::read
 */
L4_INLINE int
l4_vcon_read_u(l4_cap_idx_t vcon, char *buf, int size, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Read data from virtual console, extended version including flags.
 * \ingroup l4_vcon_api
 *
 * \param      vcon  Vcon object.
 * \param[out] buf   Pointer to data buffer.
 * \param      size  Size of buffer in bytes.
 *
 * If this function returns a positive value the caller can check the
 * #L4_VCON_READ_STAT_BREAK flag bit for a break condition. The bytes read
 * can be obtained by masking the return value with #L4_VCON_READ_SIZE_MASK.
 *
 * If a break condition is signaled, it is always the first event in the
 * transmitted content, i.e. all characters supplied by this read call
 * follow the break condition.
 *
 * `buf` might be a `NULL`, in this case the input data will be dropped.
 *
 * \note Size must not exceed #L4_VCON_READ_SIZE.
 *
 * \retval <0      Error code.
 * \retval >size   More bytes to read, `size` bytes are in the buffer `buf`.
 * \retval <=size  Number of bytes read.
 */
L4_INLINE int
l4_vcon_read_with_flags(l4_cap_idx_t vcon, char *buf, int size) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE int
l4_vcon_read_with_flags_u(l4_cap_idx_t vcon, char *buf, int size,
                          l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Vcon read flags
 */
enum L4_vcon_read_flags
{
  L4_VCON_READ_SIZE_MASK  = 0x3fffffff,  ///< Size mask
  L4_VCON_READ_STAT_BREAK = 1 << 30,     ///< Break condition flag
  L4_VCON_READ_STAT_DONE  = 1 << 31,     ///< Done condition flag
};

/**
 * Vcon attribute structure.
 * \ingroup l4_vcon_api
 */
typedef struct l4_vcon_attr_t
{
  l4_umword_t i_flags; ///< input flags
  l4_umword_t o_flags; ///< output flags
  l4_umword_t l_flags; ///< local flags
} l4_vcon_attr_t;

/**
 * Input flags.
 * \ingroup l4_vcon_api
 */
enum L4_vcon_i_flags
{
  L4_VCON_INLCR  = 000100, ///< Translate NL to CR
  L4_VCON_IGNCR  = 000200, ///< Ignore CR
  L4_VCON_ICRNL  = 000400, ///< Translate CR to NL if L4_VCON_IGNCR is not set
};

/**
 * Output flags.
 * \ingroup l4_vcon_api
 */
enum L4_vcon_o_flags
{
  L4_VCON_ONLCR  = 000004, ///< Translate NL to CR-NL
  L4_VCON_OCRNL  = 000010, ///< Translate CR to NL
  L4_VCON_ONLRET = 000040, ///< Do not output CR
};

/**
 * Local flags.
 * \ingroup l4_vcon_api
 */
enum L4_vcon_l_flags
{
  L4_VCON_ICANON = 000002,  ///< Canonical mode
  L4_VCON_ECHO   = 000010,  ///< Echo input
};

/**
 * Set attributes of a Vcon.
 * \ingroup l4_vcon_api
 *
 * \param vcon  Vcon object.
 * \param attr  Attribute structure.
 * \return Syscall return tag
 */
L4_INLINE l4_msgtag_t
l4_vcon_set_attr(l4_cap_idx_t vcon, l4_vcon_attr_t const *attr) L4_NOTHROW;

/**
 * \ingroup l4_vcon_api
 * \copybrief L4::Vcon::set_attr
 * \param vcon  Capability index of the vcon object.
 * \copydetails L4::Vcon::set_attr
 */
L4_INLINE l4_msgtag_t
l4_vcon_set_attr_u(l4_cap_idx_t vcon, l4_vcon_attr_t const *attr,
                   l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Get attributes of a Vcon.
 * \ingroup l4_vcon_api
 *
 * \param      vcon  Vcon object.
 * \param[out] attr  Attribute structure.
 * \return Syscall return tag
 */
L4_INLINE l4_msgtag_t
l4_vcon_get_attr(l4_cap_idx_t vcon, l4_vcon_attr_t *attr) L4_NOTHROW;

/**
 * \ingroup l4_vcon_api
 * \copybrief L4::Vcon::get_attr
 * \param vcon  Capability index of the vcon object.
 * \copydetails L4::Vcon::get_attr
 */
L4_INLINE l4_msgtag_t
l4_vcon_get_attr_u(l4_cap_idx_t vcon, l4_vcon_attr_t *attr,
                   l4_utcb_t *utcb) L4_NOTHROW;


/**
 * Operations on vcon objects.
 * \ingroup l4_protocol_ops
 */
enum L4_vcon_ops
{
  L4_VCON_WRITE_OP       = 0UL,    /**< Write */
  L4_VCON_READ_OP        = 1UL,    /**< Read */
  L4_VCON_SET_ATTR_OP    = 2UL,    /**< Get console attributes */
  L4_VCON_GET_ATTR_OP    = 3UL,    /**< Set console attributes */
};

/******* Implementations ********************/

L4_INLINE l4_msgtag_t
l4_vcon_send_u(l4_cap_idx_t vcon, char const *buf, int size, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *mr = l4_utcb_mr_u(utcb);
  mr->mr[0] = L4_VCON_WRITE_OP;
  mr->mr[1] = size;
  __builtin_memcpy(&mr->mr[2], buf, size);
  return l4_ipc_send(vcon, utcb,
                     l4_msgtag(L4_PROTO_LOG,
                               2 + (size + sizeof(l4_umword_t) - 1) / sizeof(l4_umword_t),
                               0, L4_MSGTAG_SCHEDULE),
                               L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_vcon_send(l4_cap_idx_t vcon, char const *buf, int size) L4_NOTHROW
{
  return l4_vcon_send_u(vcon, buf, size, l4_utcb());
}

L4_INLINE long
l4_vcon_write_u(l4_cap_idx_t vcon, char const *buf, int size, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msgtag_t t;

  if (size > L4_VCON_WRITE_SIZE)
    size = L4_VCON_WRITE_SIZE;

  t = l4_vcon_send_u(vcon, buf, size, utcb);
  if (l4_msgtag_has_error(t))
    return l4_error(t);

  return size;
}

L4_INLINE long
l4_vcon_write(l4_cap_idx_t vcon, char const *buf, int size) L4_NOTHROW
{
  return l4_vcon_write_u(vcon, buf, size, l4_utcb());
}

L4_INLINE int
l4_vcon_read_with_flags_u(l4_cap_idx_t vcon, char *buf, int size,
                          l4_utcb_t *utcb) L4_NOTHROW
{
  int ret, r;
  l4_msg_regs_t *mr;

  if (size < 0)
    return -L4_EINVAL;

  mr = l4_utcb_mr_u(utcb);
  mr->mr[0] = (size << 16) | L4_VCON_READ_OP;

  ret = l4_error_u(l4_ipc_call(vcon, utcb,
                               l4_msgtag(L4_PROTO_LOG, 1, 0, 0),
                               L4_IPC_NEVER),
                   utcb);
  if (ret < 0)
    return ret;

  r = mr->mr[0] & L4_VCON_READ_SIZE_MASK;

  if (!(mr->mr[0] & L4_VCON_READ_STAT_DONE)) // !eof
    ret = size + 1;
  else if (r < size)
    ret = r;
  else
    ret = size;

  if (L4_LIKELY(buf != NULL))
    __builtin_memcpy(buf, &mr->mr[1], r < size ? r : size);

  return ret | (mr->mr[0] & ~(L4_VCON_READ_STAT_DONE | L4_VCON_READ_SIZE_MASK));
}

L4_INLINE int
l4_vcon_read_with_flags(l4_cap_idx_t vcon, char *buf, int size) L4_NOTHROW
{
  return l4_vcon_read_with_flags_u(vcon, buf, size, l4_utcb());
}

L4_INLINE int
l4_vcon_read_u(l4_cap_idx_t vcon, char *buf, int size, l4_utcb_t *utcb) L4_NOTHROW
{
  int r = l4_vcon_read_with_flags_u(vcon, buf, size, utcb);
  if (r < 0)
    return r;

  return r & L4_VCON_READ_SIZE_MASK;
}

L4_INLINE int
l4_vcon_read(l4_cap_idx_t vcon, char *buf, int size) L4_NOTHROW
{
  return l4_vcon_read_u(vcon, buf, size, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_vcon_set_attr_u(l4_cap_idx_t vcon, l4_vcon_attr_t const *attr,
                   l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *mr = l4_utcb_mr_u(utcb);

  mr->mr[0] = L4_VCON_SET_ATTR_OP;
  __builtin_memcpy(&mr->mr[1], attr, sizeof(*attr));

  return l4_ipc_call(vcon, utcb,
                     l4_msgtag(L4_PROTO_LOG, 4, 0, 0),
                     L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_vcon_set_attr(l4_cap_idx_t vcon, l4_vcon_attr_t const *attr) L4_NOTHROW
{
  return l4_vcon_set_attr_u(vcon, attr, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_vcon_get_attr_u(l4_cap_idx_t vcon, l4_vcon_attr_t *attr,
                   l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msgtag_t res;
  l4_msg_regs_t *mr = l4_utcb_mr_u(utcb);

  mr->mr[0] = L4_VCON_GET_ATTR_OP;

  res = l4_ipc_call(vcon, utcb,
                    l4_msgtag(L4_PROTO_LOG, 1, 0, 0),
                    L4_IPC_NEVER);
  if (l4_error_u(res, utcb) >= 0)
    __builtin_memcpy(attr, &mr->mr[1], sizeof(*attr));

  return res;
}

L4_INLINE l4_msgtag_t
l4_vcon_get_attr(l4_cap_idx_t vcon, l4_vcon_attr_t *attr) L4_NOTHROW
{
  return l4_vcon_get_attr_u(vcon, attr, l4_utcb());
}
