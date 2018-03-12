/*
 * (c) 2008-2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#if 0
#define ABS_FUNC(s,value) \
.global s;          \
.type s, #function; \
.size s, 12;        \
s = value

ABS_FUNC(l4_atomic_add,     0xffffe000)
ABS_FUNC(l4_atomic_cmpxchg, 0xffffe100)
ABS_FUNC(l4_atomic_xchg,    0xffffe200)

#else

/* Using the above gives problems when using it in shared libs
 * due to relocation which actually shouldn't happen */

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
#endif
