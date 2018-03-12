/*****************************************************************************/
/**
 * \file
 * \brief  Some useful generic macros, L4f  version
 *
 * \date   11/12/2002
 * \author Lars Reuther <reuther@os.inf.tu-dresden.de> */
/*
 * (c) 2000-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

/*****************************************************************************/

#include_next <l4/util/l4_macros.h>

#pragma once

/*****************************************************************************
 *** generic macros
 *****************************************************************************/

/* generate L4 thread id printf string */
#ifndef l4util_idstr
#  define l4util_idfmt         "%lx"
#  define l4util_idfmt_adjust  "%04lx"
#  define l4util_idstr(tid)    (tid >> L4_CAP_SHIFT)
#endif
