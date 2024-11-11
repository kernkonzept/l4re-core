/*
 * (c) 2004-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

namespace L4 {

  char const *ipc_error_str[] =
    { "ok",
      "timeout",
      "phase canceled",
      "mapping failed",
      "send page fault timeout",
      "receive page fault timeout",
      "aborted",
      "message cut"
    };
}
