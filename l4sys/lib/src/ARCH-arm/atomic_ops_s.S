/*
 * (c) 2008-2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#if __ARM_ARCH < 6

/* Jump to kernel lib page which provides appropriate helpers. Because the code
 * is used in shared libs we must be careful to not emit any relocation! */

#define JUMP_CODE(name)			\
	.global name;			\
	.type name, #function;		\
	.size name, 12;			\
	name:				\
		ldr	pc, .LC##name;	\
		/* Will return to our caller */

JUMP_CODE(l4_atomic_add)
JUMP_CODE(l4_atomic_cmpxchg)
JUMP_CODE(l4_atomic_xchg)
	

.LCl4_atomic_add:         .word 0xffffe000
.LCl4_atomic_cmpxchg:     .word 0xffffe100
.LCl4_atomic_xchg:        .word 0xffffe200

#else

/* Use the proper instructions on contemporary Arm architectures. */

.global l4_atomic_add
.type l4_atomic_add, #function
l4_atomic_add:
1:	ldrex r2, [r0]
	add   r2, r2, r1
	strex r3, r2, [r0]
	teq r3, #0
	bne 1b
	mov r0, r2
	mov pc, lr
.size l4_atomic_add, .-l4_atomic_add

.global l4_atomic_cmpxchg
.type l4_atomic_cmpxchg, #function
l4_atomic_cmpxchg:
1:	ldrex r3, [r0]
	cmp r3, r1
	movne r0, #0
	movne pc, lr
	strex r3, r2, [r0]
	teq r3, #0
	bne 1b
	mov r0, #1
	mov pc, lr
.size l4_atomic_cmpxchg, .-l4_atomic_cmpxchg

.global l4_atomic_xchg
.type l4_atomic_xchg, #function
l4_atomic_xchg:
1:	ldrex r2, [r0]
	strex r3, r1, [r0]
	cmp r3, #0
	bne 1b
	mov r0, r2
	mov pc, lr
.size l4_atomic_xchg, .-l4_atomic_xchg

#endif
