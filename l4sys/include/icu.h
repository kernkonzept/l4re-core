/**
 * \file
 * Interrupt controller.
 * \ingroup l4_api
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

#include <l4/sys/kernel_object.h>
#include <l4/sys/ipc.h>

/**
 * \defgroup l4_icu_api Interrupt controller
 * \ingroup  l4_kernel_object_api
 *
 * The C Icu interface.
 *
 * To setup an IRQ line the following steps are required:
 * 1. #l4_icu_set_mode() (optional if IRQ has a default mode)
 * 2. #l4_irq_attach() to attach the IRQ capability to a thread
 * 3. #l4_icu_bind()
 * 4. #l4_icu_unmask() to receive the first IRQ
 *
 * \includefile{l4/sys/icu.h}
 */


/**
 * Flags for IRQ numbers used for the ICU.
 * \ingroup l4_icu_api
 */
enum L4_icu_flags
{
  /**
   * Flag to denote that the IRQ is actually an MSI.
   * \hideinitializer
   *
   * This flag may be used for l4_icu_bind() and l4_icu_unbind() functions to
   * denote that the IRQ number is meant to be an MSI.
   */
  L4_ICU_FLAG_MSI = 0x80000000,
};


/**
 * Interrupt attributes.
 * \ingroup l4_irq_api
 */
enum L4_irq_mode
{
  /** Flow types */
  L4_IRQ_F_NONE         = 0,     /**< None */
  L4_IRQ_F_LEVEL        = 0x2,   /**< Level triggered */
  L4_IRQ_F_EDGE         = 0x0,   /**< Edge triggered */
  L4_IRQ_F_POS          = 0x0,   /**< Positive trigger */
  L4_IRQ_F_NEG          = 0x4,   /**< Negative trigger */
  L4_IRQ_F_BOTH         = 0x8,   /**< Both edges trigger */
  L4_IRQ_F_LEVEL_HIGH   = 0x3,   /**< Level high trigger */
  L4_IRQ_F_LEVEL_LOW    = 0x7,   /**< Level low trigger */
  L4_IRQ_F_POS_EDGE     = 0x1,   /**< Positive edge trigger */
  L4_IRQ_F_NEG_EDGE     = 0x5,   /**< Negative edge trigger */
  L4_IRQ_F_BOTH_EDGE    = 0x9,   /**< Both edges trigger */
  L4_IRQ_F_MASK         = 0xf,   /**< Mask */

  /** Wakeup source? */
  L4_IRQ_F_SET_WAKEUP   = 0x10,  /**< Use irq as wakeup source */
  L4_IRQ_F_CLEAR_WAKEUP = 0x20,  /**< Do not use irq as wakeup source */
};


/**
 * Opcodes to the ICU interface.
 * \ingroup l4_protocol_ops
 */
enum L4_icu_opcode
{
  /**
   * Bind opcode.
   * \hideinitializer
   * \see l4_icu_bind()
   */
  L4_ICU_OP_BIND = 0,

  /**
   * Unbind opcode.
   * \hideinitializer
   * \see l4_icu_unbind()
   */
  L4_ICU_OP_UNBIND = 1,

  /**
   * Info opcode.
   * \hideinitializer
   * \see l4_icu_info()
   */
  L4_ICU_OP_INFO = 2,

  /**
   * Msi-info opcode.
   * \hideinitializer
   * \see l4_icu_msi_info()
   */
  L4_ICU_OP_MSI_INFO = 3,

  /**
   * Unmask opcode.
   * \hideinitializer
   * \see l4_icu_unmask()
   */
  L4_ICU_OP_UNMASK   = 4,

  /**
   * Mask opcode.
   * \hideinitializer
   * \see l4_icu_mask()
   */
  L4_ICU_OP_MASK     = 5,

  /**
   * Set-mode opcode.
   * \hideinitializer
   * \see l4_icu_set_mode()
   */
  L4_ICU_OP_SET_MODE = 6,
};

enum L4_icu_ctl_op
{
  L4_ICU_CTL_UNMASK = 0,
  L4_ICU_CTL_MASK   = 1
};


/**
 * Info structure for an ICU.
 * \ingroup l4_icu_api
 *
 * This structure contains information about the features of an ICU.
 * \see l4_icu_info().
 */
typedef struct l4_icu_info_t
{
  /**
   * Feature flags.
   *
   * If #L4_ICU_FLAG_MSI is set the ICU supports MSIs.
   */
  unsigned features;

  /**
   * The number of IRQ lines supported by the ICU,
   */
  unsigned nr_irqs;

  /**
   * The number of MSI vectors supported by the ICU,
   */
  unsigned nr_msis;
} l4_icu_info_t;

/** Info to use for a specific MSI */
typedef struct l4_icu_msi_info_t
{
  /** Value to use as address when sending this MSI */
  l4_uint64_t msi_addr;
  /** Value to use as data written to msi_addr, when sending this MSI. */
  l4_uint32_t msi_data;
} l4_icu_msi_info_t;

/**
 * Bind an interrupt line of an interrupt controller to an interrupt object.
 * \ingroup l4_icu_api
 *
 * \param icu     ICU object to bind `irq` to.
 * \param irqnum  IRQ line at the ICU.
 * \param irq     IRQ object to bind to this ICU.
 *
 * \return Syscall return tag. The caller should check the return value using
 *         l4_error() to check for errors and to identify the correct method
 *         for unmasking the interrupt.
 *         Return values `< 0` indicate an error. A return value of `0` means a
 *         direct unmask via the IRQ object using l4_irq_unmask(). A return
 *         value of `1` means that the interrupt has to be unmasked via the ICU
 *         using l4_icu_unmask().
 */
L4_INLINE l4_msgtag_t
l4_icu_bind(l4_cap_idx_t icu, unsigned irqnum, l4_cap_idx_t irq) L4_NOTHROW;

/**
 * \ingroup l4_icu_api
 * \copybrief L4::Icu::bind
 * \param icu  The ICU objecte to bind `irq` to.
 * \copydetails L4::Icu::bind
 */
L4_INLINE l4_msgtag_t
l4_icu_bind_u(l4_cap_idx_t icu, unsigned irqnum, l4_cap_idx_t irq,
              l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Remove binding of an interrupt line from the interrupt controller object.
 * \ingroup l4_icu_api
 *
 * \param icu     The ICU object from where the binding shall be removed.
 * \param irqnum  IRQ line at the ICU.
 * \param irq     IRQ object to remove from the ICU.
 *
 * \return Syscall return tag
 */
L4_INLINE l4_msgtag_t
l4_icu_unbind(l4_cap_idx_t icu, unsigned irqnum, l4_cap_idx_t irq) L4_NOTHROW;

/**
 * \ingroup l4_icu_api
 * \copybrief L4::Icu::unbind
 * \param icu  The ICU object from where the binding shall be removed.
 * \copydetails L4::Icu::unbind
 */
L4_INLINE l4_msgtag_t
l4_icu_unbind_u(l4_cap_idx_t icu, unsigned irqnum, l4_cap_idx_t irq,
                l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Set interrupt mode.
 * \ingroup l4_icu_api
 *
 * \param  icu     The ICU object.
 * \param  irqnum  IRQ line at the ICU.
 * \param  mode    Mode, see #L4_irq_mode.
 *
 * \return Syscall return tag
 */
L4_INLINE l4_msgtag_t
l4_icu_set_mode(l4_cap_idx_t icu, unsigned irqnum, l4_umword_t mode) L4_NOTHROW;

/**
 * \ingroup l4_icu_api
 * \copybrief L4::Icu::set_mode
 * \param icu  The ICU object.
 * \copydetails L4::Icu::set_mode
 */
L4_INLINE l4_msgtag_t
l4_icu_set_mode_u(l4_cap_idx_t icu, unsigned irqnum, l4_umword_t mode,
                  l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Get information about the capabilites of the ICU.
 * \ingroup l4_icu_api
 *
 * \param      icu   The ICU object from which information shall be retrieved.
 * \param[out] info  Pointer to an info structure to be filled with information.
 *                   The memory for this structure has to be allocated by the
 *                   caller.
 *
 * \return Syscall return tag
 */
L4_INLINE l4_msgtag_t
l4_icu_info(l4_cap_idx_t icu, l4_icu_info_t *info) L4_NOTHROW;

/**
 * \ingroup l4_icu_api
 * \copybrief L4::Icu::info
 * \param icu  The ICU object from which MSI information shall be retrieved.
 * \copydetails L4::Icu::info
 */
L4_INLINE l4_msgtag_t
l4_icu_info_u(l4_cap_idx_t icu, l4_icu_info_t *info,
              l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \ingroup l4_icu_api
 * \copybrief L4::Icu::msi_info
 * \param icu  The ICU object from which MSI information shall be retrieved.
 * \copydetails L4::Icu::msi_info
 */
L4_INLINE l4_msgtag_t
l4_icu_msi_info(l4_cap_idx_t icu, unsigned irqnum, l4_uint64_t source,
                l4_icu_msi_info_t *msi_info) L4_NOTHROW;

/**
 * \ingroup l4_icu_api
 * \copybrief l4_icu_msi_info()
 * \utcb{utcb}
 * \copydetails l4_icu_msi_info()
 */
L4_INLINE l4_msgtag_t
l4_icu_msi_info_u(l4_cap_idx_t icu, unsigned irqnum, l4_uint64_t source,
                  l4_icu_msi_info_t *msi_info, l4_utcb_t *utcb) L4_NOTHROW;


/**
 * Unmask an IRQ line.
 * \ingroup l4_icu_api
 *
 * \param icu     The ICU object where the IRQ line shall be unmasked.
 * \param irqnum  IRQ line at the ICU.
 * \param label   If non-NULL the function also waits for the next message.
 * \param to      Timeout for message to ICU, if unsure use L4_IPC_NEVER.
 *
 * \return Syscall return tag, the error values therein are undefined because
 *         l4_icu_unmask() is a sender-only IPC.
 */
L4_INLINE l4_msgtag_t
l4_icu_unmask(l4_cap_idx_t icu, unsigned irqnum, l4_umword_t *label,
              l4_timeout_t to) L4_NOTHROW;

/**
 * \ingroup l4_icu_api
 * \copybrief L4::Icu::unmask
 * \param icu  The ICU object where the IRQ line shall be unmasked.
 * \copydetails L4::Icu::unmask
 */
L4_INLINE l4_msgtag_t
l4_icu_unmask_u(l4_cap_idx_t icu, unsigned irqnum, l4_umword_t *label,
                l4_timeout_t to, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Mask an IRQ line.
 * \ingroup l4_icu_api
 *
 * \param icu     The ICU object where the IRQ line 'irqnum' shall be masked.
 * \param irqnum  IRQ line at the ICU.
 * \param label   If non-NULL the function also waits for the next message.
 * \param to      Timeout for message to ICU, if unsure use L4_IPC_NEVER.
 *
 * \return Syscall return tag
 */
L4_INLINE l4_msgtag_t
l4_icu_mask(l4_cap_idx_t icu, unsigned irqnum, l4_umword_t *label,
            l4_timeout_t to) L4_NOTHROW;

/**
 * \ingroup l4_icu_api
 * \copybrief L4::Icu::mask
 * \param icu  The ICU object where the IRQ line 'irqnum' shall be masked.
 * \copydetails L4::Icu::mask
 */
L4_INLINE l4_msgtag_t
l4_icu_mask_u(l4_cap_idx_t icu, unsigned irqnum, l4_umword_t *label,
              l4_timeout_t to, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_icu_control_u(l4_cap_idx_t icu, unsigned irqnum, unsigned op,
                 l4_umword_t *label,l4_timeout_t to,
                 l4_utcb_t *utcb) L4_NOTHROW;


/**************************************************************************
 * Implementations
 */

L4_INLINE l4_msgtag_t
l4_icu_bind_u(l4_cap_idx_t icu, unsigned irqnum, l4_cap_idx_t irq,
              l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  m->mr[0] = L4_ICU_OP_BIND;
  m->mr[1] = irqnum;
  m->mr[2] = l4_map_obj_control(0, 0);
  m->mr[3] = l4_obj_fpage(irq, 0, L4_FPAGE_RWX).raw;
  return l4_ipc_call(icu, utcb, l4_msgtag(L4_PROTO_IRQ, 2, 1, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_icu_unbind_u(l4_cap_idx_t icu, unsigned irqnum, l4_cap_idx_t irq,
                l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  m->mr[0] = L4_ICU_OP_UNBIND;
  m->mr[1] = irqnum;
  m->mr[2] = l4_map_obj_control(0, 0);
  m->mr[3] = l4_obj_fpage(irq, 0, L4_FPAGE_RWX).raw;
  return l4_ipc_call(icu, utcb, l4_msgtag(L4_PROTO_IRQ, 2, 1, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_icu_info_u(l4_cap_idx_t icu, l4_icu_info_t *info,
              l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msgtag_t res;
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  m->mr[0] = L4_ICU_OP_INFO;
  res = l4_ipc_call(icu, utcb, l4_msgtag(L4_PROTO_IRQ, 1, 0, 0), L4_IPC_NEVER);
  info->features = m->mr[0];
  info->nr_irqs  = m->mr[1];
  info->nr_msis  = m->mr[2];
  return res;
}

L4_INLINE l4_msgtag_t
l4_icu_msi_info_u(l4_cap_idx_t icu, unsigned irqnum, l4_uint64_t source,
                  l4_icu_msi_info_t *msi_info, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msgtag_t res;
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  m->mr[0] = L4_ICU_OP_MSI_INFO;
  m->mr[1] = irqnum;
  m->mr64[l4_utcb_mr64_idx(2)] = source;
  res = l4_ipc_call(icu, utcb, l4_msgtag(L4_PROTO_IRQ,
                                         2 + 1 * sizeof(l4_uint64_t)
                                               / sizeof(l4_umword_t),
                                         0, 0), L4_IPC_NEVER);
  if (L4_UNLIKELY(l4_msgtag_has_error(res)))
    return res;

  if (L4_UNLIKELY(l4_msgtag_words(res) * sizeof(l4_umword_t) < sizeof(*msi_info)))
    return res;

  __builtin_memcpy(msi_info, &m->mr[0], sizeof(*msi_info));
  return res;
}

L4_INLINE l4_msgtag_t
l4_icu_set_mode_u(l4_cap_idx_t icu, unsigned irqnum, l4_umword_t mode,
                  l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *mr = l4_utcb_mr_u(utcb);
  mr->mr[0] = L4_ICU_OP_SET_MODE;
  mr->mr[1] = irqnum;
  mr->mr[2] = mode;
  return l4_ipc_call(icu, utcb, l4_msgtag(L4_PROTO_IRQ, 3, 0, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_icu_control_u(l4_cap_idx_t icu, unsigned irqnum, unsigned op,
                 l4_umword_t *label, l4_timeout_t to,
                 l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  m->mr[0] = L4_ICU_OP_UNMASK + op;
  m->mr[1] = irqnum;
  if (label)
    return l4_ipc_send_and_wait(icu, utcb, l4_msgtag(L4_PROTO_IRQ, 2, 0, 0),
                                label, to);
  else
    return l4_ipc_send(icu, utcb, l4_msgtag(L4_PROTO_IRQ, 2, 0, 0), to);
}

L4_INLINE l4_msgtag_t
l4_icu_mask_u(l4_cap_idx_t icu, unsigned irqnum, l4_umword_t *label,
              l4_timeout_t to, l4_utcb_t *utcb) L4_NOTHROW
{ return l4_icu_control_u(icu, irqnum, L4_ICU_CTL_MASK, label, to, utcb); }

L4_INLINE l4_msgtag_t
l4_icu_unmask_u(l4_cap_idx_t icu, unsigned irqnum, l4_umword_t *label,
                l4_timeout_t to, l4_utcb_t *utcb) L4_NOTHROW
{ return l4_icu_control_u(icu, irqnum, L4_ICU_CTL_UNMASK, label, to, utcb); }




L4_INLINE l4_msgtag_t
l4_icu_bind(l4_cap_idx_t icu, unsigned irqnum, l4_cap_idx_t irq) L4_NOTHROW
{ return l4_icu_bind_u(icu, irqnum, irq, l4_utcb()); }

L4_INLINE l4_msgtag_t
l4_icu_unbind(l4_cap_idx_t icu, unsigned irqnum, l4_cap_idx_t irq) L4_NOTHROW
{ return l4_icu_unbind_u(icu, irqnum, irq, l4_utcb()); }

L4_INLINE l4_msgtag_t
l4_icu_info(l4_cap_idx_t icu, l4_icu_info_t *info) L4_NOTHROW
{ return l4_icu_info_u(icu, info, l4_utcb()); }

L4_INLINE l4_msgtag_t
l4_icu_msi_info(l4_cap_idx_t icu, unsigned irqnum, l4_uint64_t source,
                l4_icu_msi_info_t *msi_info) L4_NOTHROW
{ return l4_icu_msi_info_u(icu, irqnum, source, msi_info, l4_utcb()); }

L4_INLINE l4_msgtag_t
l4_icu_unmask(l4_cap_idx_t icu, unsigned irqnum, l4_umword_t *label,
              l4_timeout_t to) L4_NOTHROW
{ return l4_icu_control_u(icu, irqnum, L4_ICU_CTL_UNMASK, label, to, l4_utcb()); }

L4_INLINE l4_msgtag_t
l4_icu_mask(l4_cap_idx_t icu, unsigned irqnum, l4_umword_t *label,
            l4_timeout_t to) L4_NOTHROW
{ return l4_icu_control_u(icu, irqnum, L4_ICU_CTL_MASK, label, to, l4_utcb()); }

L4_INLINE l4_msgtag_t
l4_icu_set_mode(l4_cap_idx_t icu, unsigned irqnum, l4_umword_t mode) L4_NOTHROW
{
  return l4_icu_set_mode_u(icu, irqnum, mode, l4_utcb());
}
