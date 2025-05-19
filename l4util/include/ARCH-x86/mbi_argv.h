/**
 * \file
 * \brief    command line handling
 *
 * \date     2003
 * \author   Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#ifndef L4UTIL_MBI_ARGV
#define L4UTIL_MBI_ARGV

#include <l4/sys/l4int.h>
#include <l4/util/mb_info.h>
#include <l4/sys/compiler.h>

L4_BEGIN_DECLS

void l4util_mbi_to_argv(l4_mword_t flag, l4util_mb_info_t *mbi);

extern int  l4util_argc;
extern char *l4util_argv[];

L4_END_DECLS

#endif

