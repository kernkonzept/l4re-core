/* SPDX-License-Identifier: License-Ref-kk-custom */
/*
 * Copyright (C) 2022 Kernkonzept GmbH.
 * Author(s): Frank Mehnert <frank.mehnert@kernkonzept.com>
 */

        // Clobbers x1, x2
        .section .text
        .global __l4_setup_arm64_exception_vector
__l4_setup_arm64_exception_vector:
#ifndef EXCEPTION_EL1_DETECT_X1
        // Other tasks than Moe
        adrp    x2, :got:__l4_sys_syscall
        ldr     x2, [x2, #:got_lo12:__l4_sys_syscall]
        ldr     x1, [x2]
#else
        // Moe
#endif
        mov     x2, #0x0001
        movk    x2, #0xd400, lsl #16
        cmp     w1, w2 // 'w2' contains the encoding of 'svc #0'.
        b.eq    1f
        adr     x1, .Lexception_vector
        msr     VBAR_EL1, x1
        isb
1:      ret


#define FRAME_SIZE (10*16 + 16*32)

        .macro vec_handler num
        .align  7
        msr     spsel, #0
        sub     sp, sp, #FRAME_SIZE
        stp     x0, x1,   [sp, #16 + 0]
        mov     x0, #\num
        b       .Lcall_handler
        .endm

.Lcall_handler:
        /* x31 is stack pointer, x19..x29 are callee-saved */
        stp     q30, q31, [sp, #16 * 10 + 32 * 15]
        stp     q28, q29, [sp, #16 * 10 + 32 * 14]
        stp     q26, q27, [sp, #16 * 10 + 32 * 13]
        stp     q24, q25, [sp, #16 * 10 + 32 * 12]
        stp     q22, q23, [sp, #16 * 10 + 32 * 11]
        stp     q20, q21, [sp, #16 * 10 + 32 * 10]
        stp     q18, q19, [sp, #16 * 10 + 32 * 9]
        stp     q16, q17, [sp, #16 * 10 + 32 * 8]
        stp     q14, q15, [sp, #16 * 10 + 32 * 7]
        stp     q12, q13, [sp, #16 * 10 + 32 * 6]
        stp     q10, q11, [sp, #16 * 10 + 32 * 5]
        stp     q8, q9,   [sp, #16 * 10 + 32 * 4]
        stp     q6, q7,   [sp, #16 * 10 + 32 * 3]
        stp     q4, q5,   [sp, #16 * 10 + 32 * 2]
        stp     q2, q3,   [sp, #16 * 10 + 32 * 1]
        stp     q0, q1,   [sp, #16 * 10 + 32 * 0]
        stp     x18, x30, [sp, #16 * 9]
        stp     x16, x17, [sp, #16 * 8]
        stp     x14, x15, [sp, #16 * 7]
        stp     x12, x13, [sp, #16 * 6]
        stp     x10, x11, [sp, #16 * 5]
        stp     x8, x9,   [sp, #16 * 4]
        stp     x6, x7,   [sp, #16 * 3]
        stp     x4, x5,   [sp, #16 * 2]
        stp     x2, x3,   [sp, #16 * 1]
        // The content of ESR_el1 would be interesting as well but reading this
        // register would trap because of HCR_EL2.TRVM!
        mov     x2, #0
        mrs     x3, ELR_el1
        mrs     x4, SPSR_el1
        // x0 and x1 are saved in vec_handler
        mov     x1, sp
        bl      __l4_arm64_exception_handler

        ldp     x0, x1,   [sp, #16 * 0]
        ldp     x2, x3,   [sp, #16 * 1]
        ldp     x4, x5,   [sp, #16 * 2]
        ldp     x6, x7,   [sp, #16 * 3]
        ldp     x8, x9,   [sp, #16 * 4]
        ldp     x10, x11, [sp, #16 * 5]
        ldp     x12, x13, [sp, #16 * 6]
        ldp     x14, x15, [sp, #16 * 7]
        ldp     x16, x15, [sp, #16 * 8]
        ldp     x18, x30, [sp, #16 * 9]
        ldp     q30, q31, [sp, #16 * 10 + 32 * 15]
        ldp     q28, q29, [sp, #16 * 10 + 32 * 14]
        ldp     q26, q27, [sp, #16 * 10 + 32 * 13]
        ldp     q24, q25, [sp, #16 * 10 + 32 * 12]
        ldp     q22, q23, [sp, #16 * 10 + 32 * 11]
        ldp     q20, q21, [sp, #16 * 10 + 32 * 10]
        ldp     q18, q19, [sp, #16 * 10 + 32 * 9]
        ldp     q16, q17, [sp, #16 * 10 + 32 * 8]
        ldp     q14, q15, [sp, #16 * 10 + 32 * 7]
        ldp     q12, q13, [sp, #16 * 10 + 32 * 6]
        ldp     q10, q11, [sp, #16 * 10 + 32 * 5]
        ldp     q8, q9,   [sp, #16 * 10 + 32 * 4]
        ldp     q6, q7,   [sp, #16 * 10 + 32 * 3]
        ldp     q4, q5,   [sp, #16 * 10 + 32 * 2]
        ldp     q2, q3,   [sp, #16 * 10 + 32 * 1]
        ldp     q0, q1,   [sp, #16 * 10 + 32 * 0]
        add     sp, sp, #FRAME_SIZE
        eret

        .align  11
.Lexception_vector:
        vec_handler 0
        vec_handler 1
        vec_handler 2
        vec_handler 3
        vec_handler 4
        vec_handler 5
        vec_handler 6
        vec_handler 7
        vec_handler 8
        vec_handler 9
        vec_handler 10
        vec_handler 11
        vec_handler 12
        vec_handler 13
        vec_handler 14
        vec_handler 15
