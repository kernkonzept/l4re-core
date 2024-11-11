/*
 * (c) 2004-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#ifndef L4_CXX_ATEXIT_H__
#define L4_CXX_ATEXIT_H__

extern "C" void __cxa_finalize(void *dso_handle);
extern "C" int __cxa_atexit(void (*f)(void*), void *arg, void *dso_handle);
extern "C" int atexit(void (*f)(void));

#endif
