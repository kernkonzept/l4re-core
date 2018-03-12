typedef unsigned long l4_umword_t;

typedef struct l4re_elf_aux_mword_t
{
  l4_umword_t type;
  l4_umword_t length;
  l4_umword_t value;
} l4re_elf_aux_mword_t;

extern char const __L4_KIP_ADDR__[];

static __attribute__((used, section(".rol4re_elf_aux"), aligned(sizeof(l4_umword_t)))) l4re_elf_aux_mword_t const kip_addr
  = { 4, sizeof(l4re_elf_aux_mword_t), (l4_umword_t)__L4_KIP_ADDR__ };

