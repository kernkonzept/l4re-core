#pragma once
/**
 * \file
 * \brief   Goos protocol definition
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

namespace L4Re { namespace Video {
  namespace Goos_
  {
    /**
     * \brief Frame buffer communication-protocol opcodes
     * \ingroup api_l4re_protocols
     * \internal
     */
    enum Opcodes
    {
      Info, Get_buffer, Create_buffer, Create_view,
      Delete_buffer, Delete_view,
      View_info, View_set_info, View_stack, View_refresh,
      Screen_refresh
    };
  };
}}
