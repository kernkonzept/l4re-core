/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#define FFI_DECLARE_CLASS_FN(ret_type, fn_name, ...) \
  static ret_type fn_name(__VA_ARGS__)

#define FFI_DEFINE_CLASS_FN(ret_type, cls_name, fn_name, ...) \
  ret_type cls_name::fn_name(__VA_ARGS__)
