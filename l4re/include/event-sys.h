/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once
namespace L4Re
{
  namespace Event_
  {
    /**
     * \brief Event communication-protocol opcodes
     * \ingroup api_l4re_protocols
     * \internal
     */
    enum Opcodes
    {
      Get, Get_num_streams, Get_stream_info, Get_stream_info_for_id,
      Get_axis_info, Get_stream_state_for_id
    };
  };
};
