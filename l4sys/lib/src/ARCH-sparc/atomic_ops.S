/* SPDX-License-Identifier: GPL-2.0-only or License-Ref-kk-custom */
/*
 * Copyright (C) 2010 Technische Universität Dresden.
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
