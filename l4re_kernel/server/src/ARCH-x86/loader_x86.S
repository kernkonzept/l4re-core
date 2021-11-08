/* SPDX-License-Identifier: GPL-2.0-only or License-Ref-kk-custom */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Frank Mehnert <frank.mehnert@kernkonzept.com>
 */

        .global loader_thread
loader_thread:
        // The C function expects that it was called and adjusts the stack
        // accordingly. We need this extra call to keep the 16-byte alignment.
        call    loader_thread_c
1:      jmp     1b
