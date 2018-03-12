/*
 * (c) 2014 Steffen Liebergeld <steffen.liebergeld@kernkonzept.com>
 *
 * This file is licensed under the terms of the GNU Lesser General Public
 * License 2.1.
 * See the file COPYING-LGPL-2.1 for details.
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
