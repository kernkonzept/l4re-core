/**
 * \file
 * \brief  Simple Pseudo-Random Number Generator
 * \ingroup l4util_api
 *
 * \date   1998
 * \author Lars Reuther <reuther@os.inf.tu-dresden.de> */
/*
 * (c) 2008-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#ifndef __L4UTIL_RAND_H
#define __L4UTIL_RAND_H

#define L4_RAND_MAX 65535

#include <l4/sys/compiler.h>
#include <l4/sys/l4int.h>

L4_BEGIN_DECLS

/**
 * \defgroup l4util_random Random number support
 * \ingroup l4util_api
 */

/**
 * \brief Deliver next random number
 * \ingroup l4util_random
 *
 * \return A new random number
 */
L4_CV l4_uint32_t
l4util_rand(void);

/**
 * \brief Initialize random number generator
 * \ingroup l4util_random
 *
 * \param seed Value to initialize
 */
L4_CV void
l4util_srand (l4_uint32_t seed);

L4_END_DECLS

#endif /* __L4UTIL_RAND_H */
