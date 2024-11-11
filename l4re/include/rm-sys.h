/**
 * \file
 * \brief   Region mapper protocol definitions
 */
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
  namespace Rm_
  {
    /**
     * \brief Region-map communication-protocol opcodes.
     * \ingroup api_l4re_protocols
     * \internal
     */
    enum Opcodes
    {
      Attach, Detach, Find, Attach_area, Detach_area, Get_regions, Get_areas
    };
  };
};
