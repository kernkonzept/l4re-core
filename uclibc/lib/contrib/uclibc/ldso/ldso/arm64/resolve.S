
#include <features.h>

#define ip0 x16
#define ip1 x17
#define lr  x30

/*
 * Proxy function for lazy symbol resolution of PLTGOT relocations
 */
.text
.align 4
.globl _dl_linux_resolve
.type _dl_linux_resolve,%function
.cfi_startproc
.align 4;

/* Arguments for AArch64:
 *   ip0        &PLTGOT[2]
 *   ip1        temp (dl resolver entry point) -> unused by the code below
 *   [sp, #8]   lr
 *   [sp, #0]   &PLTGOT[n]
 */
_dl_linux_resolve:
         stp     x8, x9, [sp, #-(80+8*16)]!
         .cfi_adjust_cfa_offset (80+8*16)
         .cfi_rel_offset x8, 0
         .cfi_rel_offset x9, 8

         stp     x6, x7, [sp,  #16]
         .cfi_rel_offset x6, 16
         .cfi_rel_offset x7, 24

         stp     x4, x5, [sp,  #32]
         .cfi_rel_offset x4, 32
         .cfi_rel_offset x5, 40

         stp     x2, x3, [sp,  #48]
         .cfi_rel_offset x2, 48
         .cfi_rel_offset x3, 56

         stp     x0, x1, [sp,  #64]
         .cfi_rel_offset x0, 64
         .cfi_rel_offset x1, 72

         stp     q0, q1, [sp, #(80+0*16)]
         .cfi_rel_offset q0, 80+0*16
         .cfi_rel_offset q1, 80+1*16

         stp     q2, q3, [sp, #(80+2*16)]
         .cfi_rel_offset q0, 80+2*16
         .cfi_rel_offset q1, 80+3*16

         stp     q4, q5, [sp, #(80+4*16)]
         .cfi_rel_offset q0, 80+4*16
         .cfi_rel_offset q1, 80+5*16

         stp     q6, q7, [sp, #(80+6*16)]
         .cfi_rel_offset q0, 80+6*16
         .cfi_rel_offset q1, 80+7*16

         /* Get pointer to linker struct.  */
         ldr     x0, [ip0, #-8]

         /* Prepare to call _dl_fixup().  */
         ldr     x1, [sp, 80+8*16]       /* Recover &PLTGOT[n] */

         sub     x1, x1, ip0
         add     x1, x1, x1, lsl #1
         lsl     x1, x1, #3
         sub     x1, x1, #192
         lsr     x1, x1, #3

         /* Call fixup routine.  */
         bl      _dl_linux_resolver

         /* Save the return.  */
         mov     ip0, x0

         /* Get arguments and return address back.  */
         ldp     q0, q1, [sp, #(80+0*16)]
         ldp     q2, q3, [sp, #(80+2*16)]
         ldp     q4, q5, [sp, #(80+4*16)]
         ldp     q6, q7, [sp, #(80+6*16)]
         ldp     x0, x1, [sp, #64]
         ldp     x2, x3, [sp, #48]
         ldp     x4, x5, [sp, #32]
         ldp     x6, x7, [sp, #16]
         ldp     x8, x9, [sp], #(80+8*16)
         .cfi_adjust_cfa_offset -(80+8*16)

         ldp     ip1, lr, [sp], #16
         .cfi_adjust_cfa_offset -16

         /* Jump to the newly found address.  */
         br      ip0

         .cfi_endproc

.size _dl_linux_resolve, .-_dl_linux_resolve


#define NSAVED_QREG_PAIRS 16
.macro save_q_regs
        stp	q0, q1,	[sp, #-32*NSAVED_QREG_PAIRS]!
        .cfi_adjust_cfa_offset 32 * NSAVED_QREG_PAIRS
        stp	 q2,  q3, [sp, #32*1]
        stp	 q4,  q5, [sp, #32*2]
        stp	 q6,  q7, [sp, #32*3]
        stp	 q8,  q9, [sp, #32*4]
        stp	q10, q11, [sp, #32*5]
        stp	q12, q13, [sp, #32*6]
        stp	q14, q15, [sp, #32*7]
        stp	q16, q17, [sp, #32*8]
        stp	q18, q19, [sp, #32*9]
        stp	q20, q21, [sp, #32*10]
        stp	q22, q23, [sp, #32*11]
        stp	q24, q25, [sp, #32*12]
        stp	q26, q27, [sp, #32*13]
        stp	q28, q29, [sp, #32*14]
        stp	q30, q31, [sp, #32*15]
.endm

.macro restore_q_regs
        ldp	 q2,  q3, [sp, #32*1]
        ldp	 q4,  q5, [sp, #32*2]
        ldp	 q6,  q7, [sp, #32*3]
        ldp	 q8,  q9, [sp, #32*4]
        ldp	q10, q11, [sp, #32*5]
        ldp	q12, q13, [sp, #32*6]
        ldp	q14, q15, [sp, #32*7]
        ldp	q16, q17, [sp, #32*8]
        ldp	q18, q19, [sp, #32*9]
        ldp	q20, q21, [sp, #32*10]
        ldp	q22, q23, [sp, #32*11]
        ldp	q24, q25, [sp, #32*12]
        ldp	q26, q27, [sp, #32*13]
        ldp	q28, q29, [sp, #32*14]
        ldp	q30, q31, [sp, #32*15]
        ldp	 q0,  q1, [sp], #32 * NSAVED_QREG_PAIRS
        .cfi_adjust_cfa_offset -32 * NSAVED_QREG_PAIRS
.endm


/*
 * Proxy for lazy TLSDESC resolution.
 */
        .hidden _dl_tlsdesc_resolve_rela
        .global	_dl_tlsdesc_resolve_rela
        .type	_dl_tlsdesc_resolve_rela,%function
        .cfi_startproc
        .align 2
_dl_tlsdesc_resolve_rela:
#define	NSAVED_XREG_PAIRS 9
        stp	x29, x30, [sp, #-(32+16 * NSAVED_XREG_PAIRS)]!
        .cfi_adjust_cfa_offset 32+16 * NSAVED_XREG_PAIRS
        mov	x29, sp
        stp	 x1,  x4, [sp, #32+16*0]
        stp	 x5,  x6, [sp, #32+16*1]
        stp	 x7,  x8, [sp, #32+16*2]
        stp	 x9, x10, [sp, #32+16*3]
        stp	x11, x12, [sp, #32+16*4]
        stp	x13, x14, [sp, #32+16*5]
        stp	x15, x16, [sp, #32+16*6]
        stp	x17, x18, [sp, #32+16*7]
        str	x0,       [sp, #32+16*8]

        save_q_regs

        ldr	x1, [x3, #8]
        bl	_dl_tlsdesc_resolver_rela

        restore_q_regs

        ldr	x0, [sp, #32+16*8]
        ldr	x1, [x0]
        blr	x1

        ldp	 x1,  x4, [sp, #32+16*0]
        ldp	 x5,  x6, [sp, #32+16*1]
        ldp	 x7,  x8, [sp, #32+16*2]
        ldp	 x9, x10, [sp, #32+16*3]
        ldp	x11, x12, [sp, #32+16*4]
        ldp	x13, x14, [sp, #32+16*5]
        ldp	x15, x16, [sp, #32+16*6]
        ldp	x17, x18, [sp, #32+16*7]
        ldp	x29, x30, [sp], #(32+16 * NSAVED_XREG_PAIRS)
        .cfi_adjust_cfa_offset -32-16 * NSAVED_XREG_PAIRS
        ldp	x2, x3, [sp], #16
        .cfi_adjust_cfa_offset -16
        ret
#undef NSAVED_XREG_PAIRS
        .cfi_endproc
        .size	_dl_tlsdesc_resolve_rela, .-_dl_tlsdesc_resolve_rela


/*
 * Always blocking proxy for lazy TLSDESC resolution.
 *
 * This function is used when there is already a relocation for the
 * TLSDESC running in another thread.
 */
        .hidden _dl_tlsdesc_resolve_block
        .global	_dl_tlsdesc_resolve_block
        .type	_dl_tlsdesc_resolve_block,%function
        .cfi_startproc
        .align 2
_dl_tlsdesc_resolve_block:
#define	NSAVED_XREG_PAIRS 10
1:      stp	x29, x30, [sp, #-(32 + 16 * NSAVED_XREG_PAIRS)]!
        .cfi_adjust_cfa_offset 32 + 16 * NSAVED_XREG_PAIRS
        mov	x29, sp
        stp	 x1,  x2, [sp, #32+16*0]
        stp	 x3,  x4, [sp, #32+16*1]
        stp	 x5,  x6, [sp, #32+16*2]
        stp	 x7,  x8, [sp, #32+16*3]
        stp	 x9, x10, [sp, #32+16*4]
        stp	x11, x12, [sp, #32+16*5]
        stp	x13, x14, [sp, #32+16*6]
        stp	x15, x16, [sp, #32+16*7]
        stp	x17, x18, [sp, #32+16*8]
        str	x0,       [sp, #32+16*9]

        save_q_regs

        adr	x1, 1b
        bl	_dl_tlsdesc_resolver_block

        restore_q_regs

        ldr	x0, [sp, #32 + 16 * 9]
        ldr	x1, [x0]
        blr	x1

        ldp	 x1,  x2, [sp, #32+16*0]
        ldp	 x3,  x4, [sp, #32+16*1]
        ldp	 x5,  x6, [sp, #32+16*2]
        ldp	 x7,  x8, [sp, #32+16*3]
        ldp	 x9, x10, [sp, #32+16*4]
        ldp	x11, x12, [sp, #32+16*5]
        ldp	x13, x14, [sp, #32+16*6]
        ldp	x15, x16, [sp, #32+16*7]
        ldp	x17, x18, [sp, #32+16*8]
        ldp	x29, x30, [sp], #(32 + 16 * NSAVED_XREG_PAIRS)
        .cfi_adjust_cfa_offset -32 - 16 * NSAVED_XREG_PAIRS
        ret
        .cfi_endproc
        .size	_dl_tlsdesc_resolve_block, .-_dl_tlsdesc_resolve_block
#undef NSAVED_XREG_PAIRS


/*
 * This function is used for all (resolved) undefined weak
 * TLSDESC symbols.
 */
        .hidden _dl_tlsdesc_undefweak
        .global	_dl_tlsdesc_undefweak
        .type	_dl_tlsdesc_undefweak,%function
        .cfi_startproc
        .align  2
_dl_tlsdesc_undefweak:
        str	x1, [sp, #-16]!
        .cfi_adjust_cfa_offset 16
        ldar	xzr, [x0]
        ldr	x0, [x0, #8]
        mrs	x1, tpidr_el0
        sub	x0, x0, x1
        ldr	x1, [sp], #16
        .cfi_adjust_cfa_offset -16
        ret
        .cfi_endproc
        .size	_dl_tlsdesc_undefweak, .-_dl_tlsdesc_undefweak

#ifdef SHARED

/*
 * Function to handle TLSDESC symbols that are located in the dynamic
 * TLS vector.
 *
 * The function checks the version of the TLS vector and if there is
 * already a slot allocated for the descriptor and returns the offset
 * of the symbol to the thread pointer.
 *
 * If there is a mismatch in the DTV version 
 */
        .hidden _dl_tlsdesc_dynamic
        .global	_dl_tlsdesc_dynamic
        .type	_dl_tlsdesc_dynamic,%function
        .cfi_startproc
        .align 2
_dl_tlsdesc_dynamic:
# define NSAVED_XREG_PAIRS 2
        stp	x29, x30, [sp,#-(32+16*NSAVED_XREG_PAIRS)]!
        .cfi_adjust_cfa_offset (32+16*NSAVED_XREG_PAIRS)
        mov	x29, sp

        stp	x1,  x2, [sp, #32+16*0]
        stp	x3,  x4, [sp, #32+16*1]

        mrs	x4, tpidr_el0
        ldar	x2, [x0]
        ldr	x1, [x0,#8]
        ldr	x0, [x4]
        ldr	x3, [x1,#16]
        cmp	x3, x2
        b.hi	2f
        ldr	x2, [x1]
        add	x0, x0, x2, lsl #4
        ldr	x0, [x0]
        cmn	x0, #0x1
        b.eq	2f
        ldr	x1, [x1,#8]
        add	x0, x0, x1
        sub	x0, x0, x4
1:
        ldp	 x1,  x2, [sp, #32+16*0]
        ldp	 x3,  x4, [sp, #32+16*1]

        ldp	x29, x30, [sp], #(32+16*NSAVED_XREG_PAIRS)
        .cfi_adjust_cfa_offset (-32-16*NSAVED_XREG_PAIRS)
# undef NSAVED_XREG_PAIRS
        ret
2:
        /* Slow path, uses __tls_get_addr() */
# define NSAVED_XREG_PAIRS 7
        stp	 x5,  x6, [sp, #-16*NSAVED_XREG_PAIRS]!
        .cfi_adjust_cfa_offset (16*NSAVED_XREG_PAIRS)
        stp	 x7,  x8, [sp, #16*1]
        stp	 x9, x10, [sp, #16*2]
        stp	x11, x12, [sp, #16*3]
        stp	x13, x14, [sp, #16*4]
        stp	x15, x16, [sp, #16*5]
        stp	x17, x18, [sp, #16*6]

        save_q_regs

        mov	x0, x1
        bl	__tls_get_addr

        mrs	x1, tpidr_el0
        sub	x0, x0, x1

        restore_q_regs

        ldp	 x7,  x8, [sp, #16*1]
        ldp	 x9, x10, [sp, #16*2]
        ldp	x11, x12, [sp, #16*3]
        ldp	x13, x14, [sp, #16*4]
        ldp	x15, x16, [sp, #16*5]
        ldp	x17, x18, [sp, #16*6]
        ldp	 x5,  x6, [sp], #16*NSAVED_XREG_PAIRS
        .cfi_adjust_cfa_offset (-16*NSAVED_XREG_PAIRS)
        b	1b
        .cfi_endproc
        .size	_dl_tlsdesc_dynamic, .-_dl_tlsdesc_dynamic
# undef NSAVED_XREG_PAIRS
#endif

        .hidden _dl_tlsdesc_return_lazy
        .global	_dl_tlsdesc_return_lazy
        .type	_dl_tlsdesc_return_lazy,%function
        .cfi_startproc
        .align 2
_dl_tlsdesc_return_lazy:
	ldar	xzr, [x0]
	ldr	x0, [x0, #8]
	ret
	.cfi_endproc
	.size	_dl_tlsdesc_return_lazy, .-_dl_tlsdesc_return_lazy


