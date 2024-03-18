/* Definition for thread-local data handling.  NPTL/RISCV32 version.
   Copyright (C) 2005, 2007, 2011 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _TLS_H
#define _TLS_H  1

#ifndef __ASSEMBLER__
# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>

/* Type for the dtv.  */
typedef union dtv
{
  size_t counter;
  struct
  {
    void *val;
    bool is_static;
  } pointer;
} dtv_t;

#else /* __ASSEMBLER__ */
# include <tcb-offsets.h>
#endif /* __ASSEMBLER__ */

/* We require TLS support in the tools.  */
#define HAVE_TLS_SUPPORT                1
#define HAVE_TLS_MODEL_ATTRIBUTE        1
#define HAVE___THREAD                   1

/* Signal that TLS support is available.  */
#define USE_TLS	1

#ifndef __ASSEMBLER__

register void *__thread_self __asm__("tp");
# define READ_THREAD_POINTER() ({ __thread_self; })

/* Get system call information.  */
# include <sysdep.h>

/* The TP points to the start of the TLS block.  */
# define TLS_DTV_AT_TP  1

/* Get the thread descriptor definition.  */
# include <../../descr.h>

typedef struct
{
  dtv_t *dtv;
  void *private;
} tcbhead_t;

/* This is the size of the initial TCB.  Because our TCB is before the thread
   pointer, we don't need this.  */
# define TLS_INIT_TCB_SIZE    0
# define TLS_INIT_TCB_ALIGN   __alignof__ (struct pthread)

/* This is the size of the TCB.  Because our TCB is before the thread
   pointer, we don't need this.  */
# define TLS_TCB_SIZE         0
# define TLS_TCB_ALIGN        __alignof__ (struct pthread)

/* This is the size we need before TCB - actually, it includes the TCB.  */
# define TLS_PRE_TCB_SIZE \
    (sizeof (struct pthread)                                                  \
   + ((sizeof (tcbhead_t) + __alignof (struct pthread) - 1)                   \
      & ~(__alignof (struct pthread) - 1)))

/* The thread pointer tp points to the end of the TCB.
   The pthread_descr structure is immediately in front of the TCB.  */
# define TLS_TCB_OFFSET 0

/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contain the length.  */
# define INSTALL_DTV(tcbp, dtvp) \
  (((tcbhead_t *) (tcbp))[-1].dtv = (dtvp) + 1)

/* Install new dtv for current thread  */
# define INSTALL_NEW_DTV(dtv) \
  (THREAD_DTV() = (dtv))

/* Return dtv of given thread descriptor.  */
# define GET_DTV(tcbp) \
  (((tcbhead_t *) (tcbp))[-1].dtv)

/* Code to initially initialize the thread pointer.
 *
 * Set TP to the address _after_ tcbhead_t. This will allow us
 * to change the size of tcbhead_t without having to re-link everything.
 *
 * secondcall has something to do with USE__THREAD,
 * seems to always be 0 so we don't care about it.
 *
 * This has to return NULL on success (or a string with the failure text).
 * It's hard to fail this, so return NULL always.
 */
# define TLS_INIT_TP(tcbp, secondcall) \
  ({ __thread_self = (char*)tcbp + TLS_TCB_OFFSET; NULL; })

/* Return the address of the dtv for the current thread.  */
#  define THREAD_DTV() \
  (((tcbhead_t *) (READ_THREAD_POINTER () - TLS_TCB_OFFSET))[-1].dtv)

/* Return the thread descriptor for the current thread.  */
# define THREAD_SELF \
  ((struct pthread *) (READ_THREAD_POINTER ()                         \
                       - TLS_TCB_OFFSET - TLS_PRE_TCB_SIZE))

/* Informs libthread_db that the thread pointer is register 4, which is used
 * to know how to do THREAD_SELF.  */
# define DB_THREAD_SELF \
  REGISTER (64, 64, 4 * 8, - TLS_TCB_OFFSET - TLS_PRE_TCB_SIZE)

/* Access to data in the thread descriptor is easy.  */
#define THREAD_GETMEM(descr, member) \
  descr->member
#define THREAD_GETMEM_NC(descr, member, idx) \
  descr->member[idx]
#define THREAD_SETMEM(descr, member, value) \
  descr->member = (value)
#define THREAD_SETMEM_NC(descr, member, idx, value) \
  descr->member[idx] = (value)

/* l_tls_offset == 0 is perfectly valid, so we have to use some different
   value to mean unset l_tls_offset.  */
# define NO_TLS_OFFSET          -1

/* Get and set the global scope generation counter in struct pthread.  */
#define THREAD_GSCOPE_FLAG_UNUSED 0
#define THREAD_GSCOPE_FLAG_USED   1
#define THREAD_GSCOPE_FLAG_WAIT   2
#define THREAD_GSCOPE_RESET_FLAG() \
  do                       \
    { int __res                    \
  = atomic_exchange_rel (&THREAD_SELF->header.gscope_flag,       \
             THREAD_GSCOPE_FLAG_UNUSED);         \
      if (__res == THREAD_GSCOPE_FLAG_WAIT)            \
  lll_futex_wake (&THREAD_SELF->header.gscope_flag, 1, LLL_PRIVATE);   \
    }                      \
  while (0)
#define THREAD_GSCOPE_SET_FLAG() \
  do                       \
    {                      \
      THREAD_SELF->header.gscope_flag = THREAD_GSCOPE_FLAG_USED;       \
      atomic_write_barrier ();                 \
    }                      \
  while (0)
#define THREAD_GSCOPE_WAIT() \
  GL(dl_wait_lookup_done) ()

#endif /* __ASSEMBLER__ */

#endif  /* tls.h */

