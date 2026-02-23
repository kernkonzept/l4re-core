/* Define weak functions like in lib1funcs.S / _dvmd_tls. */
        .weak   __aeabi_idiv0
        .weak   __aeabi_ldiv0
        .text
        .arm
        .syntax unified
        .globl  __aeabi_idiv0
        .globl  __aeabi_ldiv0
        .type __aeabi_idiv0, %function
        .type __aeabi_ldiv0, %function
__aeabi_idiv0:
__aeabi_ldiv0:
        bx      lr
        .size   __aeabi_idiv0, . - __aeabi_idiv0
        .size   __aeabi_ldiv0, . - __aeabi_ldiv0
