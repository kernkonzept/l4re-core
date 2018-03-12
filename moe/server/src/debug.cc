/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "debug.h"

Dbg::Dbg_bits Dbg::dbg_bits[] =
{{"info",          Dbg::Info},
 {"nfo",           Dbg::Info},
 {"warn",          Dbg::Warn},
 {"boot",          Dbg::Boot},
 {"server",        Dbg::Server},
 {"svr",           Dbg::Server},
 {"exceptions",    Dbg::Exceptions},
 {"exc",           Dbg::Exceptions},
 {"loader",        Dbg::Loader},
 {"ldr",           Dbg::Loader},
 {"script",        Dbg::Parser},
 {"scr",           Dbg::Parser},
 {"fs",            Dbg::Boot_fs},
 {"namespace",     Dbg::Name_space},
 {"ns",            Dbg::Name_space},
 {"bfs",           Dbg::Boot_fs},
 {"all",           ~0UL},
 {0, 0}};

