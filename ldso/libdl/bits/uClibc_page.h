#pragma once

#include <l4/sys/consts.h>

#define PAGE_SHIFT L4_PAGESHIFT
#define PAGE_SIZE  L4_PAGESIZE
#define PAGE_MASK  (~(PAGE_SIZE-1))

