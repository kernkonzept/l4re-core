/**
 * \file    crtx/include/crt0.h
 * \brief   Variables exported by crt0. This is low-level stuff --
 *          applications should not use this information directly.
 *
 * \date    08/2003
 * \author  Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#ifndef CRTX_CRT0_H
#define CRTX_CRT0_H

#include <l4/sys/compiler.h>

EXTERN_C_BEGIN

void crt0_construction(void) L4_NOTHROW;
void crt0_sys_destruction(void) L4_NOTHROW;
void crt0_dde_construction(void) L4_NOTHROW;

void __main(int argc, char const *argv[]) L4_NOTHROW;

EXTERN_C_END

#endif
