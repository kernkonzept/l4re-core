/*
 * (c) 2011 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
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
/*
 * Note, do _NOT_ use this lock unless you got advised to do so.
 */
/*
 * This lock-implementation is quite simplistic, especially its list of
 * blockers on a lock will only grow, i.e. ex-blockers will not be removed.
 * We also have one kernel-provided irq per lock+thread. For general use,
 * this is too much.
 */

#include <l4/util/llulc.h>
#include <l4/sys/semaphore>
#include <l4/sys/factory>
#include <l4/util/atomic.h>

struct l4ullulock_struct_t {
  unsigned long          lock;
  void *                 blocklistroot;
  l4_umword_t            block_cnt;
  void *               (*mem_alloc)(size_t);
  void                 (*mem_free)(void *);
  l4_cap_idx_t         (*cap_alloc)(void);
  void                 (*cap_free)(l4_cap_idx_t c);
  L4::Cap<L4::Factory>   factory;
};

static l4_umword_t add_atomic(l4_umword_t *x, int delta)
{
  l4_umword_t oldval, newval;
  do
    {
      oldval = *x;
      newval = oldval + delta;
    }
  while (!l4util_cmpxchg(x, oldval, newval));

  return oldval;
}

class Lock
{
public:
  typedef L4::Cap<L4::Semaphore> Lockcap;
  typedef l4ullulock_t Allocator;

  explicit Lock(l4_utcb_t *u, Allocator *al)
    : _utcb(u), _triggers(0), _next(0), _allocator(al)
  {}

  void init_root() { _next = this; }

  void lock(Lockcap c) { _lock = c; }

  void block() throw()
  { _lock->down(); add_atomic(&_triggers, -1); }

  void wakeup() throw()
  {
    if (_triggers < 2)
      {
        add_atomic(&_triggers, 1);
        _lock->trigger();
      }
  }

  static Lock *get_dummy(l4ullulock_t *t)
  { return reinterpret_cast<Lock *>(t->blocklistroot); }

  void enqueue(l4ullulock_t *t)
  {
    Lock *r = get_dummy(t);
    l4_umword_t oldval, newval;

    do
      {
        _next = r->_next;
        oldval = (l4_umword_t)r->_next;
        newval = (l4_umword_t)this;
      }
    while (!l4util_cmpxchg((volatile l4_umword_t*)&r->_next, oldval, newval));
  }

  void *operator new(size_t, void *p)
  { return p; }

  void operator delete(void *p)
  { reinterpret_cast<Lock *>(p)->_allocator->mem_free(p); }

  static void shift(l4ullulock_t *t)
  { t->blocklistroot = (void *)get_dummy(t)->_next; }

  static void free_list(l4ullulock_t *t);

  static Lock *find(l4ullulock_t *t, l4_utcb_t *u);
  static void wakeup_others(l4ullulock_t *t, l4_utcb_t *u);

private:
  l4_utcb_t *_utcb;
  Lockcap _lock;
  l4_umword_t _triggers;
  Lock *_next;
  Allocator *_allocator;
};


Lock *Lock::find(l4ullulock_t *t, l4_utcb_t *u)
{
  Lock *r = get_dummy(t)->_next;
  Lock *i = r;

  do
    {
      if (u == i->_utcb)
        return i;
      i = i->_next;
    }
  while (i != r);

  return 0;
}

void Lock::wakeup_others(l4ullulock_t *t, l4_utcb_t *u)
{
  Lock *r = get_dummy(t)->_next;
  Lock *i = r;

  do
    {
      if (i->_utcb && u != i->_utcb)
        i->wakeup();
      i = i->_next;
    }
  while (i != r);
}

void Lock::free_list(l4ullulock_t *t)
{
  Lock *r = get_dummy(t)->_next;
  Lock *i = r;

  do
    {
      Lock *d = i;
      i = i->_next;
      delete d;
    }
  while (i != r);

  delete r;
}

static Lock *create_new_thread_lock(l4ullulock_t *t, l4_utcb_t *u)
{
  int err;
  L4::Cap<L4::Factory> f;
  void *p = t->mem_alloc(sizeof(Lock));
  if (!p)
    return 0;

  Lock *x = new (p) Lock(u, t);

  L4::Cap<L4::Semaphore> c = L4::Cap<L4::Semaphore>(t->cap_alloc());
  if (!c.is_valid())
    goto fail1;

  f = L4::Cap<L4::Factory>(t->factory);

  err = l4_error(f->create(c, L4_PROTO_SEMAPHORE));
  if (err < 0)
    goto fail2;

  x->lock(c);
  return x;

fail2:
  t->cap_free(c.cap());
fail1:
  delete x;
  return 0;
}

int l4ullulock_init(l4ullulock_t **t,
                    void *(*mem_alloc)(size_t x),
                    void  (*mem_free)(void *p),
                    l4_cap_idx_t (*cap_alloc)(void),
                    void (*cap_free)(l4_cap_idx_t c),
                    l4_cap_idx_t factory)
{
  l4ullulock_t *_t = (l4ullulock_t *)mem_alloc(sizeof(*_t));
  if (!_t)
    return -L4_ENOMEM;

  _t->lock      = 0;
  _t->block_cnt = 0;
  _t->mem_alloc = mem_alloc;
  _t->mem_free  = mem_free;
  _t->cap_alloc = cap_alloc;
  _t->cap_free  = cap_free;
  _t->factory   = L4::Cap<L4::Factory>(factory);

  void *p = mem_alloc(sizeof(Lock));
  if (!p)
    {
      mem_free(_t);
      return -L4_ENOMEM;
    }

  Lock *l = new (p) Lock(0, _t);
  l->init_root();
  _t->blocklistroot = l;
  *t = _t;
  return 0;
}

int l4ullulock_deinit(l4ullulock_t *t)
{
  Lock::free_list(t);
  t->mem_free(t);
  return 0;
}

int l4ullulock_lock(l4ullulock_t *t, l4_utcb_t *u)
{
  while (add_atomic(&t->lock, 1) > 0)
    {
      // we need to block, someone has the lock already
      Lock *tl = Lock::find(t, u);
      if (!tl)
        {
          tl = create_new_thread_lock(t, u);
          if (!tl)
            {
              add_atomic(&t->lock, -1);
              return -L4_ENOMEM;
            }

          tl->enqueue(t);

          add_atomic(&t->lock, -1);
          continue;
        }

      add_atomic(&t->lock, -1);
      asm volatile("" : : : "memory");
      add_atomic(&t->block_cnt, 1);
      asm volatile("" : : : "memory");

      tl->block();

      asm volatile("" : : : "memory");
      add_atomic(&t->block_cnt, -1);
    }

  return 0;
}

int l4ullulock_unlock(l4ullulock_t *t, l4_utcb_t *u)
{
  add_atomic(&t->lock, -1);
  asm volatile("" : : : "memory");
  if (t->block_cnt > 0)
    {
      Lock::shift(t);
      Lock::wakeup_others(t, u);
    }

  return 0;
}
