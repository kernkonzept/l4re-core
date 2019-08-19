/**
 * \file
 * \brief  Low-level kernel debugger functions.
 */
#pragma once

#include <l4/sys/ipc.h>

L4_INLINE l4_msgtag_t
l4_invoke_debugger(l4_cap_idx_t obj, l4_msgtag_t tag, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msgtag_t t2;
  unsigned const words = l4_msgtag_words(tag);
  l4_msg_regs_t *mr = l4_utcb_mr_u(utcb);

  if (l4_is_invalid_cap(obj))
    return l4_msgtag(-L4_EINVAL, 0, 0, 0);

  if (words + 2 > L4_UTCB_GENERIC_DATA_SIZE)
    return l4_msgtag(-L4_EMSGTOOLONG, 0, 0, 0);

  mr->mr[0] += 0x100;
  mr->mr[words]     = L4_ITEM_MAP;
  mr->mr[words + 1] = l4_obj_fpage(obj, 0, L4_CAP_FPAGE_RWS).raw;
  t2 = l4_msgtag(L4_PROTO_DEBUGGER, words, 1, l4_msgtag_flags(tag));

  return l4_ipc_call(L4_BASE_DEBUGGER_CAP, utcb, t2, L4_IPC_NEVER);
}

