.globl l4_atomic_add
l4_atomic_add:
	ta 0x0815

.globl l4_atomic_cmpxchg
l4_atomic_cmpxchg:
	ta 0x0815
