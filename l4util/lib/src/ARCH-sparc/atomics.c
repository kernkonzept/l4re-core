
#include <l4/sys/l4int.h>

l4_uint32_t
__atomic_fetch_add_4(l4_uint32_t *v, l4_uint32_t a, int mm)
{ __builtin_trap(); return *v; }

l4_uint32_t
__atomic_fetch_sub_4(l4_uint32_t *v, l4_uint32_t a, int mm)
{ __builtin_trap(); return *v; }

l4_uint32_t
__atomic_compare_exchange_4(l4_uint32_t *d, l4_uint32_t cmp,
                            l4_uint32_t nv, int weak, int mmok, int mmerr)
{ __builtin_trap(); return 0; }
