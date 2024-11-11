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
 * License: see LICENSE.spdx (in this directory or the directories above)
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
