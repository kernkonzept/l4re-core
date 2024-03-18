        .data

        .type   l4_global_kip, %object
        .global l4_global_kip
#if __SIZEOF_LONG__ == 4
        .p2align 2
l4_global_kip:
        .long   L4_KIP_ADDR
#else
        .p2align 3
l4_global_kip:
        .quad   L4_KIP_ADDR
#endif
        .size   l4_global_kip, .-l4_global_kip
