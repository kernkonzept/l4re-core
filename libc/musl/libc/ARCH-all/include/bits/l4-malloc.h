#pragma once

#include <l4/sys/compiler.h>
#include <stddef.h>

L4_BEGIN_DECLS
void *uclibc_morecore(long bytes);
L4_END_DECLS
