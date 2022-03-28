#pragma once

#if CONFIG_BID_STATIC_STACK
#define L4_DEFINE_L4_STACK_PHDR l4re_stack 0x60000012;
#else
#define L4_DEFINE_L4_STACK_PHDR
#endif

#define L4_DEFINE_L4PHDRS \
   l4re_aux   0x60000014; \
   L4_DEFINE_L4_STACK_PHDR

#define L4_DEFINE_X86_KERNEL_ENTRY_SYMS \
   PROVIDE(__l4sys_invoke_direct = 0xeacff000 + 0x000);

#ifdef LD_LLD
  /* Be compatible with ld.lld´s --image-base option. */
# define L4_SET_BASE(default_addr) \
  . += SIZEOF_HEADERS
# define L4_DEFINE_EXECUTABLE_START(default_addr) \
  PROVIDE (__executable_start = .); \
  L4_SET_BASE(default_addr)
#else
  /* Default from GNU binutils. */
# define L4_SET_BASE(default_addr) \
  . = SEGMENT_START("text-segment", default_addr) + SIZEOF_HEADERS
# define L4_DEFINE_EXECUTABLE_START(default_addr) \
  PROVIDE (__executable_start = SEGMENT_START("text-segment", default_addr)); \
  L4_SET_BASE(default_addr)
#endif
