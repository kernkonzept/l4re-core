/*
 * Copyright (C) 2010 Technische Universität Dresden.
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

.globl l4_atomic_add
l4_atomic_add:
	ta 0x0815

.globl l4_atomic_cmpxchg
l4_atomic_cmpxchg:
	ta 0x0815

.globl __atomic_compare_exchange_1
__atomic_compare_exchange_1:
	ta 0x815
