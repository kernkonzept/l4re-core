/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
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
static _Unwind_Ptr (*uw_getcfa) (struct _Unwind_Context *);
static _Unwind_Ptr (*uw_getgr) (struct _Unwind_Context *, int);

static void
init (void)
{
  void *handle = dlopen ("libgcc_s.so.1", 0);

  if (handle == NULL)
    return;

  uw_bt     = dlsym (handle, "_Unwind_Backtrace");
  uw_getpc  = dlsym (handle, "_Unwind_GetIP");
  uw_getcfa = dlsym (handle, "_Unwind_GetCFA");
  uw_getgr  = dlsym (handle, "_Unwind_GetGR");
  if (uw_getpc == NULL || uw_getgr == NULL || uw_getcfa == NULL)
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
  void *last_frame;
  void *last_sp;
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

  /* IA32: %ebp is DWARF2 register 5 */
  arg->last_sp    = (void *)uw_getcfa (ctx);
  arg->last_frame = (void *)uw_getgr (ctx, 5);
  return _URC_NO_REASON;
}
struct Frame
{
  struct Frame *fp;
  void *ret;
};


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
  else if (arg.cnt < max)
    {
      struct Frame *fp = (struct Frame *)arg.last_frame;

      while (arg.cnt < max)
        {
          /* Check for out of range.  */
          if ((void *)fp < arg.last_sp
              /* || (void *) ebp > __libc_stack_end*/
              || ((long)fp & 3))
            break;

          pc_array[arg.cnt++] = fp->ret;
          fp = fp->fp;
        }
    }
  return arg.cnt != -1 ? arg.cnt : 0;
}

