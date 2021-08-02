/**
 * \file
 * \brief   Fixed sized integer types, RISC-V version
 * \ingroup l4_basic_types
 */
/* SPDX-License-Identifier: ((GPL-2.0-only WITH mif-exception) OR LicenseRef-kk-custom) */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 */
#pragma once

#include_next <l4/sys/l4int.h>

/**
 * \addtogroup l4_basic_types
 */
/**@{*/

#define L4_MWORD_BITS           __riscv_xlen  /**< Size of machine words in bits */

#if __riscv_xlen == 64
typedef unsigned long           l4_size_t;    /**< \brief Unsigned size type */
typedef signed long             l4_ssize_t;   /**< \brief Signed size type */
#else
typedef unsigned int            l4_size_t;    /**< \brief Unsigned size type */
typedef signed int              l4_ssize_t;   /**< \brief Signed size type */
#endif
/**@}*/
