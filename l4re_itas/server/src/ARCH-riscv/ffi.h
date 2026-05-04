/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <sys/asm.h>

#define FFI_DECLARE_CLASS_FN(ret_type, fn_name, ...) \
  static ret_type fn_name(__VA_ARGS__) \
    asm(L4_stringify(fn_name ## _stub)); \
  static ret_type fn_name ## _impl(__VA_ARGS__) \
    asm(L4_stringify(fn_name ## _impl))

// We're called from another binary and need to setup our GP-pointer before
// calling any functions that may use global symbols.
#define FFI_DEFINE_CLASS_FN(ret_type, cls_name, fn_name, ...) \
  asm(                                                      \
    ".global " L4_stringify(fn_name ## _stub)          "\n" \
    ".type " L4_stringify(fn_name ## _stub) " STT_FUNC  \n" \
    L4_stringify(fn_name ## _stub) ":                   \n" \
    "  addi sp, sp, -16                                 \n" \
    REG_S " ra, 0(sp)                                   \n" \
    REG_S " gp, 8(sp)                                   \n" \
    ".option push                                       \n" \
    ".option norelax                                    \n" \
    "  la   gp, __global_pointer$                       \n" \
    ".option pop                                        \n" \
    "  jal " L4_stringify(fn_name ## _impl) "           \n" \
    REG_L " ra, 0(sp)                                   \n" \
    REG_L " gp, 8(sp)                                   \n" \
    "  addi sp, sp, 16                                  \n" \
    "  ret                                              \n" \
  );                                                        \
  ret_type cls_name::fn_name ## _impl(__VA_ARGS__)
