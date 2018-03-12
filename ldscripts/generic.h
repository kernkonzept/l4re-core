#pragma once

#define L4_DEFINE_L4PHDRS \
   l4re_aux   0x60000014;

#define L4_DEFINE_L4PHDRS___DISABLED \
   stack 0x60000012 AT (__L4_STACK_ADDR__); \
   kip   0x60000013 AT (__L4_KIP_ADDR__); \
   l4re_aux   0x60000014;

#define L4_DEFINE_X86_KERNEL_ENTRY_SYMS \
   PROVIDE(__l4sys_invoke_direct = __L4_KIP_ADDR__ + 0x800); \
   PROVIDE(__l4sys_debugger_direct = __L4_KIP_ADDR__ + 0x900);
