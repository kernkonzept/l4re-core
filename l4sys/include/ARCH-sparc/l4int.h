/**
 * \file
 * \brief   Fixed sized integer types, sparc(32) version
 * \ingroup l4_basic_types
 */
/*
 * (c) 2008-2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include_next <l4/sys/l4int.h>

/**
 * \addtogroup l4_basic_types
 */
/**@{*/

#define L4_MWORD_BITS           32            /**< Size of machine words in bits */

typedef unsigned int            l4_size_t;    /**< \brief Unsigned size type */
typedef signed int              l4_ssize_t;   /**< \brief Signed size type */
/**@}*/

