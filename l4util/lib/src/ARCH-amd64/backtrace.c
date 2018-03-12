/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <l4/util/backtrace.h>

#include <unwind.h>
#include <stdlib.h>

#if defined NOT_FOR_L4 && defined __PIC__

#include <dlfcn.h>

static _Unwind_Reason_Code (*uw_bt) (_Unwind_Trace_Fn, void *);
static _Unwind_Ptr (*uw_getpc) (struct _Unwind_Context *);

static void
init (void)
{
  void *handle = dlopen ("libgcc_s.so.1", 0);

  if (handle == NULL)
    return;

  uw_bt     = dlsym (handle, "_Unwind_Backtrace");
  uw_getpc  = dlsym (handle, "_Unwind_GetIP");
  if (uw_getpc == NULL)
    uw_bt = NULL;
}
#else

#define uw_getgr _Unwind_GetGR
#define uw_getpc _Unwind_GetIP
#define uw_bt _Unwind_Backtrace
#define uw_getcfa _Unwind_GetCFA

#endif

struct Bt_arg
{
  void **pc_array;
  int  cnt, max;
};


static _Unwind_Reason_Code
__bt_helper(struct _Unwind_Context *ctx, void *a)
{
  struct Bt_arg *arg = a;

  /* Skip first function, it is l4util_backtrace ... */
  if (arg->cnt != -1)
    arg->pc_array[arg->cnt] = (void *)uw_getpc (ctx);
  if (++arg->cnt == arg->max)
    return _URC_END_OF_STACK;

  return _URC_NO_REASON;
}


int
l4util_backtrace(void **pc_array, int max)
{
  struct Bt_arg arg = { .pc_array = pc_array, .max = max, .cnt = -1 };

#if defined NOT_FOR_L4 && defined __PIC__
  static int initialized = 0;
  if (!initialized)
    {
      initialized = 1;
      init();
    }

  if (uw_bt == NULL)
    return 0;
#endif

  if (max >= 1)
    uw_bt (__bt_helper, &arg);

  if (arg.cnt > 1 && arg.pc_array[arg.cnt - 1] == (void*)0)
    --arg.cnt;
  return arg.cnt != -1 ? arg.cnt : 0;
}

