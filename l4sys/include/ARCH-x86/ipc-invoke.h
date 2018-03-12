/**
 * \internal
 * \file
 * \brief   L4 IPC System Call Invoking in Assembler.
 * \ingroup l4_api
 *
 * This file can also be used in asm-files, so don't include C statements.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>
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

#pragma once

/*
 * Some words about the sysenter entry frame: Since the sysenter instruction
 * automatically reloads the instruction pointer (eip) and the stack pointer
 * (esp) after kernel entry, we have to save both registers preliminary to
 * that instruction. We use ecx to store the user-level esp and save eip onto
 * the stack. The ecx register contains the IPC timeout and has to be saved
 * onto the stack, too. The ebp register is saved for compatibility reasons
 * with the Hazelnut kernel. Both the esp and the ss register are also pushed
 * onto the stack to be able to return using the "lret" instruction from the
 * sysexit trampoline page if Small Address Spaces are enabled.
 */

#ifdef __PIC__
#  define L4S_PIC_SAVE "push %%ebx; "
#  define L4S_PIC_RESTORE "pop %%ebx; "
#  define L4S_PIC_CLOBBER
#  define L4S_PIC_SYSCALL , [func] "m" (__l4sys_invoke_indirect)
#if 1

#ifdef __PIC__
extern void (*__l4sys_invoke_indirect)(void);
#endif
#  define IPC_SYSENTER      "# indirect sys invoke \n\t" \
                            "call *%[func]    \n\t"
#else
#  define L4S_PIC_SYSCALL
#  define IPC_SYSENTER      "call __l4sys_invoke_direct@plt    \n\t"
#endif
#  define IPC_SYSENTER_ASM   call __l4sys_invoke_direct@plt
#else
/**
 * \internal
 * \brief Kernel entry code for inline assembly.
 */
#define IPC_SYSENTER      "call __l4sys_invoke_direct    \n\t"
/**
 * \internal
 * \brief Kernel entry code for assembler code.
 */
#define IPC_SYSENTER_ASM   call __l4sys_invoke_direct
/**
 * \internal
 * \brief Save PIC register, if needed.
 */
#  define L4S_PIC_SAVE
/**
 * \internal
 * \brief Restore PIC register, if needed.
 */
#  define L4S_PIC_RESTORE
/**
 * \internal
 * \brief PIC clobber list.
 */
#  define L4S_PIC_CLOBBER ,"ebx"
#  define L4S_PIC_SYSCALL

#endif
/**
 * \internal
 * \brief Kernel entry code for inline assembly.
 */
#define L4_ENTER_KERNEL L4S_PIC_SAVE "push %%ebp; " \
                        IPC_SYSENTER                \
                        " pop %%ebp; " L4S_PIC_RESTORE

