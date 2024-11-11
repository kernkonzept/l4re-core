/*
 * (c) 2014 Steffen Liebergeld <steffen.liebergeld@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

namespace L4Re {
  namespace Inhibitor_ {
    /**
     * \brief Inhibitor communication-protocol opcodes
     * \ingroup api_l4re_protocols
     * \internal
     */
    enum Opcodes { Acquire, Release, Next_lock_info };
  }
}
