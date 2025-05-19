/**
 * \internal
 * \file
 * \brief   Atomic memory modifications, ARM.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/compiler.h>

L4_BEGIN_DECLS

long int
l4_atomic_add(volatile long int* mem, long int offset) L4_NOTHROW L4_LONG_CALL;

long int
l4_atomic_xchg(volatile long int* mem, long int newval) L4_NOTHROW L4_LONG_CALL;

long int
l4_atomic_cmpxchg(volatile long int* mem, long int oldval, long int newval) L4_NOTHROW L4_LONG_CALL;

L4_END_DECLS
