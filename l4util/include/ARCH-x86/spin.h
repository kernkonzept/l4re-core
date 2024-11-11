/**
 * \file
 * \brief Spinning for x86
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#ifndef __l4util_spin_h
#define __l4util_spin_h

#include <l4/sys/compiler.h>

__BEGIN_DECLS

L4_CV void l4_spin(int x,int y);
L4_CV void l4_spin_vga(int x,int y);
L4_CV void l4_spin_n_text(int x, int y, int len, const char*s);
L4_CV void l4_spin_n_text_vga(int x, int y, int len, const char*s);

/****************************************************************************
*                                                                           *
* spin_text()     - spinning wheel at the hercules screen. The given text   *
*                   must be a text constant, no variables or arrays. Its    *
*                   size is determined with the sizeof operator, it's much  *
*                   faster than the strlen function.                        *
* spin_text_vga() - same for vga.                                           *
*                                                                           *
****************************************************************************/
#define l4_spin_text(x, y, text) \
	l4_spin_n_text((x), (y), sizeof(text)-1, "" text)
#define l4_spin_text_vga(x, y, text) \
	l4_spin_n_text_vga((x), (y), sizeof(text)-1, "" text)

__END_DECLS

#endif
