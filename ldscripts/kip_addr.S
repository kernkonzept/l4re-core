
        .section .rol4re_elf_aux,"a",%progbits

#if __SIZEOF_LONG__ == 4

        .p2align 2
        .type   kip_addr, %object
        .size   kip_addr, 12
kip_addr:
        .long   4
        .long   12
        .long   __L4_KIP_ADDR__

#else

        .p2align 3
        .type   kip_addr, %object
        .size   kip_addr, 24
kip_addr:
        .quad   4
        .quad   24
        .quad   __L4_KIP_ADDR__

#endif
