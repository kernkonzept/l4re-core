/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdint.h>

#if defined(__x86_64__)
# define R_ARCH_RELATIVE 8    /* R_X86_64_RELATIVE */
# define ARCH_HAS_RELR 1
# define ELF_USES_RELA 1
#elif defined(__aarch64__)
# define R_ARCH_RELATIVE 1027 /* R_AARCH64_RELATIVE */
# define ARCH_HAS_RELR 1
# define ELF_USES_RELA 1
#elif defined(__arm__)
# define R_ARCH_RELATIVE 23   /* R_ARM_RELATIVE */
# define ARCH_HAS_RELR 0
# define ELF_USES_RELA 0
#elif defined(__riscv)
# define R_ARCH_RELATIVE 3    /* R_RISCV_RELATIVE */
# define ARCH_HAS_RELR 0
# define ELF_USES_RELA 1
#else
# error "reloc_static_pie: unsupported architecture, add R_ARCH_RELATIVE"
#endif

#if UINTPTR_MAX == 0xffffffffffffffffULL

typedef uint64_t Elf_Addr;
typedef uint64_t Elf_Xword;
typedef int64_t  Elf_Sxword;

# define ELF_R_TYPE(info) ((uint32_t)(info))

#else

typedef uint32_t Elf_Addr;
typedef uint32_t Elf_Xword;
typedef int32_t  Elf_Sxword;

# define ELF_R_TYPE(info) ((uint8_t)(info))

#endif

typedef struct
{
  Elf_Addr   r_offset;
  Elf_Xword  r_info;
  Elf_Sxword r_addend;
} Elf_Rela;

typedef struct
{
  Elf_Addr  r_offset;
  Elf_Xword r_info;
} Elf_Rel;

typedef struct
{
  Elf_Addr d_tag;
  union
  {
    Elf_Addr d_val;
    Elf_Addr d_ptr;
  } d_un;
} Elf_Dyn;

enum
{
  DT_NULL    = 0,
  DT_REL     = 17,
  DT_RELSZ   = 18,
  DT_RELA    = 7,
  DT_RELASZ  = 8,
  DT_RELRSZ  = 35,
  DT_RELR    = 36,
};

extern Elf_Dyn _DYNAMIC[] __attribute__((visibility("hidden")));

/*
 * Freestanding self-relocation for statically linked, position-independent
 * executables.
 *
 * Some programs (sigma0, bootstrap) do not run through the usual libc startup
 * code. Instead they bring their own architecture specific crt0.S. When such a
 * program is built as a static PIE (CONFIG_BID_PIE), that crt0.S calls
 * reloc_static_pie() to apply the load bias to the binary's own relocations.
 */
void reloc_static_pie(Elf_Addr load_addr);

void reloc_static_pie(Elf_Addr load_addr)
{
  const Elf_Dyn *dyn = _DYNAMIC;
  Elf_Addr rel_addr = 0;
  Elf_Addr rel_size = 0;
#if ARCH_HAS_RELR
  Elf_Addr relr_addr = 0;
  Elf_Addr relr_size = 0;
#endif

  for (; dyn->d_tag != DT_NULL; ++dyn)
    switch (dyn->d_tag)
      {
#if ELF_USES_RELA
      case DT_RELA:
        rel_addr = dyn->d_un.d_ptr;
        break;
      case DT_RELASZ:
        rel_size = dyn->d_un.d_val;
        break;
#else
      case DT_REL:
        rel_addr = dyn->d_un.d_ptr;
        break;
      case DT_RELSZ:
        rel_size = dyn->d_un.d_val;
        break;
#endif
#if ARCH_HAS_RELR
      case DT_RELR:
        relr_addr = dyn->d_un.d_ptr;
        break;
      case DT_RELRSZ:
        relr_size = dyn->d_un.d_val;
        break;
#endif
      default:
        break;
      }

  if (rel_size)
    {
#if ELF_USES_RELA
      const Elf_Rela *rel = (const Elf_Rela *)(load_addr + rel_addr);
      const Elf_Rela *end = (const Elf_Rela *)((const char *)rel + rel_size);

      for (; rel < end; ++rel)
        if (ELF_R_TYPE(rel->r_info) == R_ARCH_RELATIVE)
          *(Elf_Addr *)(load_addr + rel->r_offset) = load_addr + rel->r_addend;
#else
      const Elf_Rel *rel = (const Elf_Rel *)(load_addr + rel_addr);
      const Elf_Rel *end = (const Elf_Rel *)((const char *)rel + rel_size);

      for (; rel < end; ++rel)
        if (ELF_R_TYPE(rel->r_info) == R_ARCH_RELATIVE)
          *(Elf_Addr *)(load_addr + rel->r_offset) += load_addr;
#endif
    }

#if ARCH_HAS_RELR
  if (relr_size)
    {
      const Elf_Addr *relr = (const Elf_Addr *)(load_addr + relr_addr);
      const Elf_Addr *end = (const Elf_Addr *)((const char *)relr + relr_size);
      Elf_Addr *addr = 0;

      for (; relr < end; ++relr)
        {
          if ((*relr & 1) == 0)
            {
              addr = (Elf_Addr *)(load_addr + *relr);
              *addr++ += load_addr;
            }
          else
            {
              Elf_Addr bitmap = *relr;
              for (unsigned i = 0; (bitmap >>= 1) != 0; ++i)
                if (bitmap & 1)
                  addr[i] += load_addr;
              addr += 8 * sizeof(Elf_Addr) - 1;
            }
        }
    }
#endif
}
