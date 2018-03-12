/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
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

#include <l4/sys/err.h>
#include <l4/sys/ipc.h>

static char const *const _l4sys_errortab[L4_ERRNOMAX] = {
    [L4_EOK]           = "OK",
    [L4_EPERM]         = "Operation not permitted",
    [L4_ENOENT]        = "No such object found",
    [L4_EIO]           = "I/O error",
    [L4_EAGAIN]        = "Try again",
    [L4_ENOMEM]        = "Insufficient memory",
    [L4_EACCESS]       = "Access not permitted",
    [L4_EFAULT]        = "Invalid memory address",
    [L4_EBUSY]         = "Function busy",
    [L4_EEXIST]        = "Object exists",
    [L4_ENODEV]        = "No such device",
    [L4_EINVAL]        = "Invalid argument",
    [L4_ERANGE]        = "Argument out of range",
    [L4_ENAMETOOLONG]  = "Name too long",
    [L4_ENOSYS]        = "Invalid request",
    [L4_EBADPROTO]     = "Invalid protocol",
    [L4_EADDRNOTAVAIL] = "Address not available"
};

static char const *const _l4sys_ipc_errortab[L4_EIPC_HI - L4_EIPC_LO] = {
    [0]                = "OK",
    [L4_IPC_SETIMEOUT]     = "Send timeout",
    [L4_IPC_RETIMEOUT]     = "Receive timeout",
    [L4_IPC_ENOT_EXISTENT] = "Void capability invoked",
    [L4_IPC_SECANCELED]    = "Send operation canceled",
    [L4_IPC_RECANCELED]    = "Receive operation canceled",
    [L4_IPC_SEMSGCUT]      = "Overflow during send operation",
    [L4_IPC_REMSGCUT]      = "Overflow during receive operation",
    [L4_IPC_SESNDPFTO]     = "Send page-fault timeout (send phase)",
    [L4_IPC_RESNDPFTO]     = "Send page-fault timeout (receive phase)",
    [L4_IPC_SERCVPFTO]     = "Receive page-fault timeout (send phase)",
    [L4_IPC_RERCVPFTO]     = "Receive page-fault timeout (receive phase)",
    [L4_IPC_SEABORTED]     = "Send operation aborted",
    [L4_IPC_REABORTED]     = "Receive operation aborted",
    [L4_IPC_SEMAPFAILED]   = "Map operation failed (send phase)",
    [L4_IPC_REMAPFAILED]   = "Map operation failed (receive phase)"
};

static char const *const _l4sys_ipc_errortab2[] = {
    [L4_ENOREPLY     - 1000] = "Do not reply",
    [L4_EMSGTOOSHORT - 1000] = "Message too short",
    [L4_EMSGTOOLONG  - 1000] = "Message too long",
    [L4_EMSGMISSARG  - 1000] = "Message is missing (an) argument(s)",
};

L4_CV char const *l4sys_errtostr(long err)
{
  static_assert(L4_ENOREPLY    == 1000, "L4_ENOREPLY value change");
  static_assert(L4_EMSGMISSARG == 1003, "L4_EMSGMISSARG value change");

  err = -err;
  if (err >= 0 && err < L4_ERRNOMAX)
    return _l4sys_errortab[err];
  else if (err >= L4_EIPC_LO && err < L4_EIPC_HI)
    return _l4sys_ipc_errortab[err - L4_EIPC_LO];
  else if (err >= L4_ENOREPLY && err <= L4_EMSGMISSARG)
    return _l4sys_ipc_errortab2[err - 1000];
  else
    return "bad, unknown runtime error";
}
