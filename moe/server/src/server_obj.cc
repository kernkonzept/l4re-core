#include "server_obj.h"
#include "globals.h"

static Moe::Null_handler null_handler;

Moe::Server_object::~Server_object()
{
  _weak_ptrs.reset();

  if (_weak_cap)
    object_pool.cap_alloc()->free(_weak_cap);

  if (obj_cap().is_valid())
    {
      L4::Thread::Modify_senders todo;
      todo.add(~3UL,
               reinterpret_cast<l4_umword_t>(static_cast<L4::Epiface *>(this)),
               ~0UL,
               reinterpret_cast<l4_umword_t>(static_cast<L4::Epiface *>(&null_handler)));
      L4::Cap<L4::Thread>(L4_BASE_THREAD_CAP)->modify_senders(todo);

      object_pool.cap_alloc()->free(obj_cap(), L4_FP_ALL_SPACES | L4_FP_DELETE_OBJ);
    }
}

void
Moe::Server_object::add_weak_ref(cxx::Weak_ref_base *obj) const
{
  if (_weak_ptrs.empty() && obj_cap())
    {
      _weak_cap = object_pool.cap_alloc()->alloc<L4::Kobject>();
      // get a reference counted copy of the capability,
      // so that it does not disappear when all caps are released in user land
      _weak_cap.copy(obj_cap());
    }

  _weak_ptrs.add(obj);
}

void
Moe::Server_object::remove_weak_ref(cxx::Weak_ref_base *obj) const
{
  _weak_ptrs.remove(obj);

  if (_weak_ptrs.empty() && _weak_cap)
    {
      object_pool.cap_alloc()->free(_weak_cap);
      _weak_cap.invalidate();
    }
}


