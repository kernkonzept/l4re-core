#include <sys/asm.h>

.section ".init"

PTR_LA $t9, __pthread_initialize_minimal
jalr $t9

