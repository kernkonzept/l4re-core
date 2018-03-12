/**
 * \file
 * \brief  Syscall entry definitions.
 */
/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
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
#ifndef __L4SYS__ARCH_ARM__L4API_L4F__SYSCALL_DEFS_H__
#define __L4SYS__ARCH_ARM__L4API_L4F__SYSCALL_DEFS_H__

#ifndef L4_SYSCALL_MAGIC_OFFSET
#  define L4_SYSCALL_MAGIC_OFFSET	8
#endif
#define L4_SYSCALL_INVOKE		(-0x00000004-L4_SYSCALL_MAGIC_OFFSET)
#define L4_SYSCALL_MEM_OP               (-0x00000008-L4_SYSCALL_MAGIC_OFFSET)
#define L4_SYSCALL_DEBUGGER		(-0x0000000C-L4_SYSCALL_MAGIC_OFFSET)

#endif /* __L4SYS__ARCH_ARM__L4API_L4F__SYSCALL_DEFS_H__ */
