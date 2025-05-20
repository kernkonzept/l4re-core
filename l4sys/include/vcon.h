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
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/ipc.h>

/**
 * \defgroup l4_vcon_api Virtual Console
 * \ingroup  l4_kernel_object_api
 * C Virtual console interface for simple character based input and output, see
 * L4::Vcon for the C++ interface.
 *
 * The interrupt for read events is provided by the virtual key interrupt
 * which, in contrast to hardware IRQs, implements a limited functionality:
 *  - Only IRQ line 0 is supported, no MSI vectors.
 *  - The IRQ is edge-triggered and the IRQ mode cannot be changed.
 *  - As the IRQ is edge-triggered, it does not have to be explicitly unmasked.
 *
 * A server implementing the virtual console protocol has a queue for input
 * events. When the first input event is added to the empty queue, the virtual
 * key interrupt is triggered. Further events are added to the queue without
 * generating further interrupts. The queue is emptied when a client reads all
 * queued input events.
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
 *       `size` parameter is NOT checked. Also, this function is a send only
 *       operation, this means there is no return value except for a failed
 *       send operation. Use l4_ipc_error() to check for send errors, and
 *       **do not** use l4_error().
 */
L4_INLINE l4_msgtag_t
l4_vcon_send(l4_cap_idx_t vcon, char const *buf, unsigned size) L4_NOTHROW;

/**
 * \ingroup l4_vcon_api
 * \copybrief L4::Vcon::send
 * \param vcon  Capability index of the Vcon object.
 * \copydetails L4::Vcon::send
 */
L4_INLINE l4_msgtag_t
l4_vcon_send_u(l4_cap_idx_t vcon, char const *buf, unsigned size, l4_utcb_t *utcb) L4_NOTHROW;

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
l4_vcon_write(l4_cap_idx_t vcon, char const *buf, unsigned size) L4_NOTHROW;

/**
 * \ingroup l4_vcon_api
 * \copybrief L4::Vcon::write
 * \param vcon  Capability index of the vcon object.
 * \copydetails L4::Vcon::write
 */
L4_INLINE long
l4_vcon_write_u(l4_cap_idx_t vcon, char const *buf, unsigned size, l4_utcb_t *utcb) L4_NOTHROW;

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
 * \retval -L4_EPERM  Insufficient permissions; see precondition.
 * \retval >size      More bytes to read, `size` bytes are in the buffer `buf`.
 * \retval <=size     Number of bytes read.
 *
 * \pre The capability `vcon` must have the permission #L4_CAP_FPAGE_W.
 *
 * \note Size must not exceed #L4_VCON_READ_SIZE.
 */
L4_INLINE int
l4_vcon_read(l4_cap_idx_t vcon, char *buf, unsigned size) L4_NOTHROW;

/**
 * \ingroup l4_vcon_api
 * \copybrief L4::Vcon::read
 * \param vcon  Capability index of the vcon object.
 * \copydetails L4::Vcon::read
 */
L4_INLINE int
l4_vcon_read_u(l4_cap_idx_t vcon, char *buf, unsigned size, l4_utcb_t *utcb) L4_NOTHROW;

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
 * \retval -L4_EPERM  Insufficient permissions; see precondition.
 * \retval >size      More bytes to read, `size` bytes are in the buffer `buf`.
 * \retval <=size     Number of bytes read.
 *
 * \pre The capability `vcon` must have the permission #L4_CAP_FPAGE_W.
 */
L4_INLINE int
l4_vcon_read_with_flags(l4_cap_idx_t vcon, char *buf, unsigned size) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE int
l4_vcon_read_with_flags_u(l4_cap_idx_t vcon, char *buf, unsigned size,
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
 *
 * The flags members can be a combination of their respective enums.
 *
 * \see L4_vcon_i_flags
 * \see L4_vcon_o_flags
 * \see L4_vcon_l_flags
 * \ingroup l4_vcon_api
 */
typedef struct l4_vcon_attr_t
{
  l4_umword_t i_flags; ///< input flags
  l4_umword_t o_flags; ///< output flags
  l4_umword_t l_flags; ///< local flags

#ifdef __cplusplus
  /**
   * Set terminal attributes to disable all special processing.
   *
   * Removes all flags that would mangle the read or written characters. Also
   * disables echoing and any special processing of characters.
   */
  inline void set_raw();
#endif
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
 * \copydoc l4_vcon_attr_t::set_raw
 * \ingroup l4_vcon_api
 * \param[in,out] attr  Attribute structure to update.
 */
L4_INLINE void
l4_vcon_set_attr_raw(l4_vcon_attr_t *attr) L4_NOTHROW;


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
l4_vcon_send_u(l4_cap_idx_t vcon, char const *buf, unsigned size, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *mr = l4_utcb_mr_u(utcb);
  mr->mr[0] = L4_VCON_WRITE_OP;
  mr->mr[1] = size;
  __builtin_memcpy(&mr->mr[2], buf, size);
  return l4_ipc_send(vcon, utcb,
                     l4_msgtag(L4_PROTO_LOG, 2 + l4_bytes_to_mwords(size),
                               0, L4_MSGTAG_SCHEDULE),
                     L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_vcon_send(l4_cap_idx_t vcon, char const *buf, unsigned size) L4_NOTHROW
{
  return l4_vcon_send_u(vcon, buf, size, l4_utcb());
}

L4_INLINE long
l4_vcon_write_u(l4_cap_idx_t vcon, char const *buf, unsigned size, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msgtag_t t;

  if (size > L4_VCON_WRITE_SIZE)
    size = L4_VCON_WRITE_SIZE;

  t = l4_vcon_send_u(vcon, buf, size, utcb);
  if (l4_msgtag_has_error(t))
    return l4_error(t);

  return (long) size;
}

L4_INLINE long
l4_vcon_write(l4_cap_idx_t vcon, char const *buf, unsigned size) L4_NOTHROW
{
  return l4_vcon_write_u(vcon, buf, size, l4_utcb());
}

L4_INLINE int
l4_vcon_read_with_flags_u(l4_cap_idx_t vcon, char *buf, unsigned size,
                          l4_utcb_t *utcb) L4_NOTHROW
{
  int ret;
  unsigned r;
  l4_msg_regs_t *mr;

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
l4_vcon_read_with_flags(l4_cap_idx_t vcon, char *buf, unsigned size) L4_NOTHROW
{
  return l4_vcon_read_with_flags_u(vcon, buf, size, l4_utcb());
}

L4_INLINE int
l4_vcon_read_u(l4_cap_idx_t vcon, char *buf, unsigned size, l4_utcb_t *utcb) L4_NOTHROW
{
  int r = l4_vcon_read_with_flags_u(vcon, buf, size, utcb);
  if (r < 0)
    return r;

  return r & L4_VCON_READ_SIZE_MASK;
}

L4_INLINE int
l4_vcon_read(l4_cap_idx_t vcon, char *buf, unsigned size) L4_NOTHROW
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

L4_INLINE void
l4_vcon_set_attr_raw(l4_vcon_attr_t *attr) L4_NOTHROW
{
  attr->i_flags = 0;
  attr->o_flags = 0;
  attr->l_flags = 0;
}

#ifdef __cplusplus
inline void
l4_vcon_attr_t::set_raw()
{ l4_vcon_set_attr_raw(this); }
#endif
