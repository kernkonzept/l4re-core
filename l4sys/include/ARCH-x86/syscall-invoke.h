/**
 * \internal
 * \file
 * \brief   L4 System Call Invoking in Assembler
 * \ingroup l4_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>
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
#ifndef __L4_SYSCALL_INVOKE_H__
#define __L4_SYSCALL_INVOKE_H__

/**
 * \internal
 * \addtogroup l4_internal_api Internal API
 */
/*@{*/
/**
 * \internal
 * \brief Address of system call page.
 */
#define L4_ABS_syscall_page              0xeacff000

/**
 * \internal
 * \brief Offset of invoke system call.
 */
#define L4_ABS_invoke                    (0x000)

/**
 * \internal
 * \brief Offset of debugger system call.
 */
#define L4_ABS_debugger                  (0x200)

/**
 * \internal
 * \brief System call.
 */
# define L4_SYSCALL_debugger              "int $0x32 \n\t"

/**
 * \internal
 * \brief Generic system call.
 */
# define L4_SYSCALL(name)                 L4_SYSCALL_ ## name

/*@}*/
#endif
