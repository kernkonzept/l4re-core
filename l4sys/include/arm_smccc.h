/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/types.h>
#include <l4/sys/utcb.h>

L4_INLINE l4_msgtag_t
l4_arm_smccc_call(l4_cap_idx_t pfc, l4_umword_t func, l4_umword_t in0,
                  l4_umword_t in1, l4_umword_t in2, l4_umword_t in3,
                  l4_umword_t in4, l4_umword_t in5, l4_umword_t *out0,
                  l4_umword_t *out1, l4_umword_t *out2, l4_umword_t *out3,
                  l4_umword_t client_id) L4_NOTHROW;

L4_INLINE l4_msgtag_t
l4_arm_smccc_call_u(l4_cap_idx_t pfc, l4_umword_t func, l4_umword_t in0,
                    l4_umword_t in1, l4_umword_t in2, l4_umword_t in3,
                    l4_umword_t in4, l4_umword_t in5, l4_umword_t *out0,
                    l4_umword_t *out1, l4_umword_t *out2, l4_umword_t *out3,
                    l4_umword_t client_id, l4_utcb_t *utcb) L4_NOTHROW;

/* IMPLEMENTATION -----------------------------------------------------------*/

#include <l4/sys/ipc.h>

L4_INLINE l4_msgtag_t
l4_arm_smccc_call(l4_cap_idx_t pfc, l4_umword_t func,
                  l4_umword_t in0, l4_umword_t in1,
                  l4_umword_t in2, l4_umword_t in3,
                  l4_umword_t in4, l4_umword_t in5,
                  l4_umword_t *out0, l4_umword_t *out1,
                  l4_umword_t *out2, l4_umword_t *out3,
                  l4_umword_t client) L4_NOTHROW
{
  return l4_arm_smccc_call_u(pfc, func, in0, in1, in2, in3, in4, in5,
                             out0, out1, out2, out3, client, l4_utcb());
}


L4_INLINE l4_msgtag_t
l4_arm_smccc_call_u(l4_cap_idx_t pfc, l4_umword_t func, l4_umword_t in0,
                    l4_umword_t in1, l4_umword_t in2, l4_umword_t in3,
                    l4_umword_t in4, l4_umword_t in5, l4_umword_t *out0,
                    l4_umword_t *out1, l4_umword_t *out2, l4_umword_t *out3,
                    l4_umword_t client_id, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msgtag_t ret;
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[0] = func;
  v->mr[1] = in0;
  v->mr[2] = in1;
  v->mr[3] = in2;
  v->mr[4] = in3;
  v->mr[5] = in4;
  v->mr[6] = in5;
  v->mr[7] = client_id;

  ret = l4_ipc_call(pfc, utcb, l4_msgtag(L4_PROTO_SMCCC, 8, 0, 0),
                    L4_IPC_NEVER);

  if (l4_error(ret) >= 0)
    {
      *out0 = v->mr[0];
      *out1 = v->mr[1];
      *out2 = v->mr[2];
      *out3 = v->mr[3];
    }

  return ret;
}
